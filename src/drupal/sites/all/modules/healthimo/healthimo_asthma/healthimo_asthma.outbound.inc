<?php

function healthimo_asthma_find_unmessaged_users() {
    healthimo_log(' healthimo_asthma_find_unmessaged_users');
    return db_query(
            "
        SELECT DISTINCT u.uid
        FROM {users} u
        JOIN {profile_values} pv
            ON pv.uid = u.uid
        LEFT OUTER JOIN {healthimo_asthma_message_log} ml
            ON ml.uid = u.uid
        WHERE NOT EXISTS (
            SELECT 1 FROM {healthimo_asthma_message_log} ml2 WHERE ml2.uid = u.uid
        )
        AND
        pv.value = 1
        AND
        pv.fid IN (
            SELECT pf.fid
            FROM {profile_fields} pf
            WHERE 
            pf.name = 'profile_asthma_enrolled'
        )
        "
    );
}

/**
 * NB: if we haven't processed in the last MIN_MESSAGE_INTERVAL_DAYS * 4 days,
 * then the query will have to look back further.
 * @return <type> users that have had a message_log created in the last 30 days
 */
function healthimo_asthma_find_candidate_messaged_users() {
    healthimo_log(' healthimo_asthma_find_candidate_messaged_users');
    $minIntervalDays = MIN_MESSAGE_INTERVAL_DAYS * 24 * 60 * 60;
    $timeMaxDaysBackToLook = time() - ($minIntervalDays * 4);
    //all $timeMaxDaysBackToLook = 0;
    return db_query(
            "
        SELECT DISTINCT u.uid
        FROM {users} u
        JOIN {profile_values} pv
            ON pv.uid = u.uid
        JOIN {healthimo_asthma_message_log} ml
            ON ml.uid = u.uid
        WHERE
            ml.created > %d
        AND
        pv.value = 1
        AND
        pv.fid IN (
            SELECT pf.fid
            FROM {profile_fields} pf
            WHERE
            pf.name = 'profile_asthma_enrolled'
        )
        ",
            $timeMaxDaysBackToLook
    );
}

function healthimo_asthma_is_time_to_send_another_message($uid) {
    //too noisy healthimo_log( "healthimo_asthma_message_is_time_to_send_another_message=" .$uid);
    if ( !$uid > 0 ) {
        return FALSE;
    }
    $message_log = healthimo_asthma_message_log_load_most_recent_sent($uid);
    return healthimo_date_is_diff_n_or_more_days($message_log->created, time(), MIN_MESSAGE_INTERVAL_DAYS);
}

function healthimo_asthma_message_user($uid) {
    healthimo_log( "healthimo_asthma_message_user=" .$uid);

    if ( !$uid  > 0 ) {
        return;
    }

    $account = user_load($uid);

    $hasEmail = !empty($account->mail);
    $hasSms   = !empty($account->sms_user) && $account->sms_user[0]['status'] == 2 && !empty($account->sms_user[0]['number']);

    if (!$hasEmail && !$hasSms) {
        healthimo_log( "no info for user=" .$uid);
        return FALSE;
    }

    // start a new question cycle
    $account = healthimo_profile_save(
            $account, 'profile_question_count', 0, PROFILE_CATEGORY_PHR
    );
    
    $quicktip = healthimo_asthma_get_next_message($account);

    return healthimo_asthma_send($account, $quicktip, $hasEmail, $hasSms);
}

function healthimo_asthma_send_outbound_messages() {
    $usersMessaged = 0;
    $healthimo_asthma_max_to_send_per_run = variable_get('healthimo_asthma_max_to_send_per_run', 1);
        $result = healthimo_asthma_find_unmessaged_users();
        if ( $result ) {
            while ($row = db_fetch_object($result)) {
                if ( healthimo_asthma_message_user($row->uid) ) {
                    $usersMessaged++;
                    if ( $usersMessaged >= $healthimo_asthma_max_to_send_per_run ) {
                        return $usersMessaged;
                    }
                }
            }
        }

    return $usersMessaged;
}
