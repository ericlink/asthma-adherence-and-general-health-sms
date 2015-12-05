<?php

function hfstx_get_next_message($account) {
    healthimo_log("hfstx_message_get_next_message=" . $account->uid);

    // function taxonomy_get_vocabularies($type = NULL) {
    $program_tid = 1; // lookup $program_tid

    if (hfstx_is_time_to_ask($account)) {
        $quicktipDataRequest = hfstx_get_next_request_message($account, $program_tid);
        if ($quicktipDataRequest) {
            return $quicktipDataRequest;
        }
    }

    return hfstx_get_next_education_message($account, $program_tid);
}

function hfstx_is_time_to_ask($account) {
    $timeToAskForSomething = FALSE;
    $most_recent_request_message_log = hfstx_message_log_load_most_recent_request($account->uid);
    $configInterval = variable_get('hfstx_min_message_interval_days_for_request', 7);
    $configTimesToSendBeforeRequest = variable_get('hfstx_min_times_to_send_before_first_request', 4);
    if ($most_recent_request_message_log) {
        $timeToAskForSomething = $configInterval == 0 ? TRUE :
                healthimo_date_is_diff_n_or_more_days(
                        $most_recent_request_message_log->created, time(), $configInterval );
    } else {
        $sent_count = hfstx_message_log_count_sent($account->uid);
        $timeToAskForSomething = $sent_count >= $configTimesToSendBeforeRequest;
    }

    return $timeToAskForSomething;
}

function hfstx_get_next_request_message($account, $program_tid) {
    $quicktip = hfstx_find_question($account);
    return $quicktip->message_nid == 0 ? $quicktip : FALSE;
}

function hfstx_find_profile_fields_of_interest(&$account) {
    healthimo_log("hfstx_find_profile_fields_of_interest uid=" . $account->uid);

    $fields = array();

    // get all the questions that might apply to me
    $categories = array('general');
    if (isset($account->profile_has_asthma_in_household)
            && !empty($account->profile_has_asthma_in_household)
            && $account->profile_has_asthma_in_household == 1
    ) {
        $categories[] = 'asthma';
    }

    if (isset($account->profile_has_diabetes_in_household)
            && !empty($account->profile_has_diabetes_in_household)
            && $account->profile_has_diabetes_in_household == 1
    ) {
        $categories[] = 'diabetes';
    }


//    foreach ($categories as $c) {
//        healthimo_log("hfstx_find_profile_fields_of_interest categories=". $c);
//    }
    // see which of these fields are not set for me yet
    $result = db_query(
            "
            select  profile_field_name 
            from    profile_field_question 
            where   category in ("
            .
            db_placeholders($categories, 'varchar')
            .
            ");", $categories
    );

    if ($result) {
        while ($row = db_fetch_object($result)) {
            // variable variable using field name $a->$b
            $f = $row->profile_field_name;
            if (empty($account->$f)) {
                // this one is needed
                $fields[] = $f;
            }
        }
    }

    return $fields;
}

function hfsx_find_questions_by_least_asked($account, $fields) {
    $result = db_query_range(
            "
			select 
			name as profile_field_name, 
			max(created) as created, 
			(select ordinal  from profile_field_question pfq where pfq.profile_field_name = x.name) as ordinal, 
			(select question from profile_field_question pfq where pfq.profile_field_name = x.name) as question 
			from (
			select pf.fid, pf.name, m.created from profile_fields pf left outer join hfstx_message_log m on m.response_type_expected = pf.name 
			and m.uid = %d order by pf.fid asc) as x 
			where name in ("
            .
            db_placeholders($fields, 'varchar')
            .
            ") 
			group by name order by created, ordinal         
            ", array_merge(array($account->uid), $fields), 0, 1
    );
    return $result ? db_fetch_object($result) : FALSE;
}

function hfstx_find_question(&$account) {
    healthimo_log("hfstx_find_question uid=" . $account->uid);

    $quicktip = hfstx_message_log_get_default(
            $account->uid, 1, 'address not set yet', 'message not set yet', -1, 1, 1
    );

    $fields = hfstx_find_profile_fields_of_interest($account);

    if (count($fields) > 0) {
        $result = hfsx_find_questions_by_least_asked($account, $fields);
        if ($result) {
            $quicktip->message_nid = 0;
            $quicktip->message = $result->question;
            $quicktip->response_type_expected = $result->profile_field_name;
            // take the first one only
            return $quicktip;
        }
    }

    // return default quicktip
    return $quicktip;
}

function hfstx_get_next_education_message($account, $program_tid) {
    // get term based on profile area of interest
    // diabetes or general health for now
    if (isset($account->profile_areas_of_interest_diabetes)
            && !empty($account->profile_areas_of_interest_diabetes)
            && $account->profile_areas_of_interest_diabetes == 1
    ) {
        $term_name = 'diabetes';
    } else {
        $term_name = 'health';
    }
    $tid = healthimo_taxonomy_get_term_by_name($term_name)->tid;

    // look at area of interest and
    // the last sent to figure out what to send next,
    $lastOrdinal = 0;
    $most_recent_educational_message_log =
            hfstx_message_log_load_most_recent_sent_education($account->uid);
    if ($most_recent_educational_message_log) {
        $mostRecentQuicktipNode =
                healthimo_quicktip_load($most_recent_educational_message_log->message_nid);
        $lastOrdinal =
                !empty($mostRecentQuicktipNode->field_ordinal_value) ? $mostRecentQuicktipNode->field_ordinal_value : 0;
        healthimo_log('mostRecentQuicktipNode ' . $mostRecentQuicktipNode->field_ordinal_value);
    }

    // only get message if it's > start interval
    //;
    // if $lastOrdinal == 0 &&  $account->created, time() < MESSAGING_START_DELAY return FALSE
    //consider changing $tid (diff category) w/ previous last ordinal
    $quicktipNode = healthimo_quicktip_load_next_in_series($tid, $lastOrdinal);
    healthimo_log(
            'hfstx_get_next_education_message'
            . ' most recent ['
            . isset($most_recent_educational_message_log->mid) && !empty($most_recent_educational_message_log->mid) ? $most_recent_educational_message_log->mid : null
                    . ', '
                    . isset($most_recent_educational_message_log->message_nid) && !empty($most_recent_educational_message_log->message_nid) ? $most_recent_educational_message_log->message_nid : null
                            . ', '
                            . $lastOrdinal
                            . '] new message ['
                            . $tid
                            . ','
                            . $quicktipNode->nid
                            . ','
                            . $quicktipNode->title
                            . ']'
    );

    if ($quicktipNode) {
        return hfstx_message_log_get_default(
                $account->uid, $program_tid, 'address not set yet', $quicktipNode->title, $quicktipNode->nid, 1, 0
        );
    } else {
        watchdog(
                'hfstx', 'Could not get quicktip! (tid=%tid lastOrdinal=%lastOrdinal uid=%uid)', array(
            '%tid' => $tid,
            '%lastOrdinal' => $lastOrdinal,
            '%uid' => $account->uid,
                ), WATCHDOG_ALERT
        );
        return FALSE;
    }
}