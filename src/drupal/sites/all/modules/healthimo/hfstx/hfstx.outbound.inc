<?php

/**
 * @return <type> true if current hour of day is in sending window
 */
function hfstx_is_in_sending_window() {
    $hfstx_window_start_hour = variable_get('hfstx_window_start_hour', 8);
    $hfstx_window_end_hour = variable_get('hfstx_window_end_hour', 20);
    healthimo_log('hfstx_is_in_sending_window ' . $hfstx_window_start_hour . '-' . $hfstx_window_end_hour);
    return
    healthimo_date_is_in_window_hours(
            time(), $hfstx_window_start_hour, $hfstx_window_end_hour, "America/Chicago"
    );
}

function hfstx_find_unmessaged_users() {
    healthimo_log(' hfstx_find_unmessaged_users');
    $admin_only_clause = variable_get('healthimo_messaging_outbound_to_admin_user_only', 1) == 1 ? " AND u.uid = 1" : "";

    return db_query(
            "
        SELECT DISTINCT u.uid
        FROM {users} u
        JOIN {profile_values} pv
            ON pv.uid = u.uid
        LEFT OUTER JOIN {" . TABLE_MESSAGE_LOG . "} hfml
            ON hfml.uid = u.uid
        WHERE NOT EXISTS (
            SELECT 1 FROM {hfstx_message_log} hfml2 WHERE hfml2.uid = u.uid
        )
        " .
            $admin_only_clause
            . "
        AND
        pv.value = 1
        AND
        pv.fid IN (
            SELECT pf.fid
            FROM {profile_fields} pf
            WHERE 
            pf.name = 'profile_keyword_healthy'
            OR
            pf.name = 'profile_keyword_bcbs'
        )
        "
    );
}

/**
 * NB: if we haven't processed in the last $hfstx_min_message_interval_days * 4 days,
 * then the query will have to look back further.
 * Why time based not just 'ever' with exists?
 * @return <type> users that have had a message_log created in the last 365 days
 */
function hfstx_find_candidate_messaged_users() {
    healthimo_log(' hfstx_find_candidate_messaged_users');
    $admin_only_clause = variable_get('healthimo_messaging_outbound_to_admin_user_only', 1) == 1 ? " AND u.uid = 1" : "";
    $minIntervalDaysInSeconds = 365 * (24 * 60 * 60);
    $timeMaxDaysBackToLook = time() - ($minIntervalDaysInSeconds * 4);
    return db_query(
            "
        SELECT DISTINCT u.uid
        FROM {users} u
        JOIN {profile_values} pv
            ON pv.uid = u.uid
        JOIN {" . TABLE_MESSAGE_LOG . "} hfml
            ON hfml.uid = u.uid
        WHERE
            hfml.created > %d
            " .
            $admin_only_clause
            . "
        AND
        pv.value = 1
        AND
        pv.fid IN (
            SELECT pf.fid
            FROM {profile_fields} pf
            WHERE
            pf.name = 'profile_keyword_healthy'
            OR
            pf.name = 'profile_keyword_bcbs'
        )
        AND
        NOT EXISTS (select 1 from profile_values pvx join profile_fields pfx on pfx.fid = pvx.fid where pfx.name = 'profile_keyword_stop' and pvx.value = '1' and pvx.uid = u.uid )
        ", $timeMaxDaysBackToLook
    );
}

function hfstx_is_time_to_send_another_message($uid) {
    //too noisy healthimo_log( "hfstx_message_is_time_to_send_another_message=" .$uid);
    if (!$uid > 0) {
        return FALSE;
    }
    $message_log = hfstx_message_log_load_most_recent_sent($uid);

    $hfstx_min_message_interval_days = variable_get('hfstx_min_message_interval_days', 4);

    return
    $hfstx_min_message_interval_days == 0
    ||
    healthimo_date_is_diff_n_or_more_days($message_log->created, time(), $hfstx_min_message_interval_days);
}

function hfstx_message_user($uid) {
    healthimo_log("hfstx_message_user=" . $uid);

    if (!$uid > 0) {
        return;
    }

    $account = user_load($uid);

    if ($account->profile_keyword_stop == 1) {
        return FALSE;
    }

    $hasEmail = !empty($account->mail);
    $hasSms = !empty($account->sms_user) && $account->sms_user[0]['status'] == 2 && !empty($account->sms_user[0]['number']);

    if (!$hasEmail && !$hasSms) {
        healthimo_log("no info for user=" . $uid);
        return FALSE;
    }

    // start a new question cycle
    $account = healthimo_profile_save(
            $account, 'profile_question_count', 0, NULL
    );
    $account = user_save($account);
    
    $quicktip = hfstx_get_next_message($account);

    return hfstx_send($account, $quicktip, $hasEmail, $hasSms);
}

function hfstx_send_outbound_messages() {
    $usersMessaged = 0;
    $hfstx_max_to_send_per_run = variable_get('hfstx_max_to_send_per_run', 1);

    if (hfstx_is_in_sending_window()) {
        $result = hfstx_find_unmessaged_users();
        if ($result) {
            while ($row = db_fetch_object($result)) {
                if (hfstx_message_user($row->uid)) {
                    $usersMessaged++;
                    if ($usersMessaged >= $hfstx_max_to_send_per_run) {
                        return $usersMessaged;
                    }
                }
            }
        }

        $result = hfstx_find_candidate_messaged_users();
        if ($result) {
            while ($row = db_fetch_object($result)) {
                if (hfstx_is_time_to_send_another_message($row->uid)) {
                    if (hfstx_message_user($row->uid)) {
                        $usersMessaged++;
                        if ($usersMessaged >= $hfstx_max_to_send_per_run) {
                            return $usersMessaged;
                        }
                    }
                }
            }
        }
    }
    return $usersMessaged;
}

function hfstx_ask_question($user, $number) {
    if ($user->profile_keyword_stop != 1
            && $user->profile_question_count <
            variable_get('healthimo_max_questions_for_single_session', 4)
    ) {
        $message_log = hfstx_find_question($user);
        if ($message_log->message_nid == 0) {
            return hfstx_send_to_sms($user, $message_log);
        }
    }
    
    return false;
}
