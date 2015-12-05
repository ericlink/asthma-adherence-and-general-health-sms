<?php

define('AM', 'am');
define('PM', 'pm');
define('MAX_CONTROLLER_REMINDER_TO_SEND_PER_RUN', 100);

// for each asthma user w/ am OR pm time set,
//if pm time is set on profile and we are past local pm time (>= reminder time)
//	and no controller reminder message has been sent yet >= the pm time
//	then find and send pm message
//else if am time is set on profile and we are past local am time
//	and no controller reminder message has been sent yet >= the am time
//	then find and send pm message
//else nothing to do

function healthimo_asthma_send_controller_reminders() {
    $usersMessaged = 0;

    $result = healthimo_asthma_find_users();
    if ($result) {
        while ($row = db_fetch_object($result)) {
            $user = user_load($row->uid);
            //profile_keyword_stop
            if ($user->profile_keyword_stop != 1
                    && $user->profile_asthma_feature_controller_reminder_active == 1) {
                $sent = false;
//////////////////////////////////
                if (healthimo_asthma_need_to_send_controller_reminder($user, PM)) {
                    $sent = healthimo_asthma_send_controller_reminder($user, PM);
                } else if (healthimo_asthma_need_to_send_controller_reminder($user, AM)) {
                    $sent = healthimo_asthma_send_controller_reminder($user, AM);
                } else {
                    healthimo_asthma_log("no controller reminder to send for user " . $user->uid);
                }
//////////////////////////////////
                if ($sent) {
                    $usersMessaged++;
                    if ($usersMessaged >= MAX_CONTROLLER_REMINDER_TO_SEND_PER_RUN) {
                        return $usersMessaged;
                    }
                }
            }
        }
    }

    return $usersMessaged;
}

function healthimo_asthma_need_to_send_controller_reminder($user, $period) {
    if ($period == PM
            && isset($user->profile_asthma_feature_controller_reminder_pm_time)
            && !empty($user->profile_asthma_feature_controller_reminder_pm_time)
    ) {
        $controller_reminder_profile_time_of_day = $user->profile_asthma_feature_controller_reminder_pm_time;
    } else if ($period == AM
            && isset($user->profile_asthma_feature_controller_reminder_am_time)
            && !empty($user->profile_asthma_feature_controller_reminder_am_time)
    ) {
        $controller_reminder_profile_time_of_day = $user->profile_asthma_feature_controller_reminder_am_time;
    } else {
        // can happen if user not fully provisioned yet for some reason
        return FALSE;
    }

    healthimo_asthma_log("cr td=[" . $controller_reminder_profile_time_of_day . "]");

    $tz = 'America/Chicago';

    // profile reminder time to date time obj matching local time
    $controller_reminder_time = healthimo_date_time_of_day_to_today_time($controller_reminder_profile_time_of_day, $tz);
    if ($controller_reminder_time === FALSE) {
        return FALSE;
    }

    // date time obj for 'now' local time
    $now = healthimo_date_get_local_time_now($tz);
    if (healthimo_date_diff($controller_reminder_time, $now) >= 0) {
        $last_controller_reminder_sent = healthimo_asthma_get_last_controller_reminder_sent($user);
        if ($last_controller_reminder_sent) {
            $last_sent_in_local_time = healthimo_date_get_local_time($last_controller_reminder_sent->created, $tz);
            if (healthimo_date_diff($last_sent_in_local_time, $controller_reminder_time) > 0) {
                return TRUE;
            }
        } else {
            // no reminders sent yet, so need to send one
            return TRUE;
        }
    }

    return FALSE;
}

function healthimo_asthma_send_controller_reminder($user, $period) {
    if (PM == $period) {
        return healthimo_asthma_get_next_controller_reminder_pm($user);
    } else if (AM == $period) {
        return healthimo_asthma_get_next_controller_reminder_am($user);
    }


    return FALSE;
}

function healthimo_asthma_controller_reminder_send_to_sms($user, $quickTip) {
    //$address = DEBUG ? '2146415465' : healthimo_profile_get_sms($user);
    $address = healthimo_profile_get_sms($user);

    $message = $quickTip->title;
    $message_nid = $quickTip->nid;
    $message_log = healthimo_asthma_message_log_get_default($user->uid, 2, $address, $message, $message_nid);
    $message_log->message_type = MESSAGE_TYPE_CONTROLLER_REMINDER;
    $message_log->response_expected = $quickTip->response_expected;
    $message_log->response_type_expected = $quickTip->response_type_expected;

    $message_log = healthimo_asthma_message_log_save($message_log);
    healthimo_asthma_log(' send controller reminder sms');
    //if (SEND_ENABLED) {
    //healthimo_sms_send
    if ($message_log->response_expected) {
        $account = healthimo_profile_save(
                $account, 'profile_question_count', $account->profile_question_count + 1, PROFILE_CATEGORY_PHR
        );
    }
    if (healthimo_sms_outgoing($address, trim($message_log->message), array('response_expected' => $message_log->response_expected,))) {
//    if (sms_send($address, trim($message_log->message))) {
        $message_log->processed = time();
        $message_log = healthimo_asthma_message_log_save($message_log);
        return TRUE;
    }
//    } else {
//        // set these if not really sending for debug purposes
//        $message_log->processed = time();
//        $message_log = healthimo_asthma_message_log_save($message_log);
//        return TRUE;
//    }

    return FALSE;
}

function healthimo_asthma_get_last_controller_reminder_sent($user) {
    return healthimo_asthma_message_log_load_most_recent_by_type($user->uid, MESSAGE_TYPE_CONTROLLER_REMINDER);
}

// if qtip is >= date from sunday, type type has been seen this week
//      //// get it's type(s) and save to a list
// subtract all the 'seen' types from teh 'weekly candidate types'
// // get a quicktip or a requiest message randomly from what's left
// send it
function healthimo_asthma_get_next_controller_reminder_am($user) {
    $tz = 'America/Chicago';
    $termsSeenAlready = array('');
    $startDate = healthimo_date_get_sunday_beginning_of_this_week($tz);

    // get the terms for quick tip reminders already seen
    $recentControllerReminderMessages
            = healthimo_asthma_message_log_load_last_n_by_type($user->uid, 14, MESSAGE_TYPE_CONTROLLER_REMINDER);
    while ($previouslySent = db_fetch_object($recentControllerReminderMessages)) {
        $sent_in_local_time = healthimo_date_get_local_time($previouslySent->created, $tz);
        if (healthimo_date_diff($startDate, $sent_in_local_time) >= 0
                && $previouslySent->message_nid > 0
        ) {
            // tid has AM ?  record all tids for this one
            healthimo_asthma_log('after start date, put this one in the list ' . $previouslySent->message);
            $previouslySent->nid = $previouslySent->message_nid; //look ma, i'm a node ;)
            $terms = taxonomy_node_get_terms(node_load($previouslySent->message_nid));
            rsort($terms);
            $isAmTerm = FALSE;
            foreach ($terms as $term) {
                healthimo_asthma_log('terms ' . $term->name);
                if ($term->name == AM) {
                    $isAmTerm = TRUE;
                }
            }
            healthimo_asthma_log('am?' . $isAmTerm);
            if ($isAmTerm) {
                foreach ($terms as $term) {
                    if ($term->name != AM && !in_array($term->name, $termsSeenAlready)) {
                        $termsSeenAlready[] = $term->name;
                        healthimo_asthma_log('terms seen already' . $term->name);
                    }
                }
            }
        }
    }

    // controller requests?
    $lastControllerRequest
            = healthimo_asthma_message_log_load_last_n_by_request_type($user->uid, 1, REQUEST_TYPE_CONTROLLER);
    while ($controllerRequest = db_fetch_object($lastControllerRequest)) {
        $sent_in_local_time = healthimo_date_get_local_time($controllerRequest->created, $tz);
        if (healthimo_date_diff($startDate, $sent_in_local_time) >= 0) {
            $termsSeenAlready[] = 'Asthma Controller';
        }
    }

    // zone requests?
    $lastZoneRequest
            = healthimo_asthma_message_log_load_last_n_by_request_type($user->uid, 1, REQUEST_TYPE_ZONE);
    while ($zoneRequest = db_fetch_object($lastZoneRequest)) {
        $sent_in_local_time = healthimo_date_get_local_time($zoneRequest->created, $tz);
        if (healthimo_date_diff($startDate, $sent_in_local_time) >= 0) {
            $termsSeenAlready[] = 'Asthma Zone';
        }
    }
    foreach ($termsSeenAlready as $term) {
        healthimo_asthma_log('terms seen already' . $term);
    }

    $candidateTerms = array('Asthma Controller', 'Asthma Encouragement', 'Asthma Zone', 'Asthma Environment Trigger', 'Asthma Health', 'Asthma Medication', 'Asthma Symptoms');
    foreach ($candidateTerms as $term) {
        healthimo_asthma_log('candidate terms ' . $term);
    }

    $termsToPickFrom = array_diff($candidateTerms, $termsSeenAlready);
    foreach ($termsToPickFrom as $term) {
        healthimo_asthma_log('terms to pick from terms ' . $term);
    }

    if (count($termsToPickFrom) == 0) {
        watchdog('healthimo_asthma_controller_reminder', 'No terms to pick from for controller reminder', array(), WATCHDOG_ALERT
        );
        return FALSE;
    }

    $randomTerm = $termsToPickFrom[array_rand($termsToPickFrom)];
    healthimo_asthma_log('random term ' . $randomTerm);


    if ($randomTerm == 'Asthma Zone' || $randomTerm == 'Asthma Controller') {
// for contest questions
//        //75% of the time, use the OAT question 
//        if (time() % 4 != 0) {
//            $response_type_expected =
//                    $randomTerm == 'Asthma Zone' ? REQUEST_TYPE_ZONE : REQUEST_TYPE_CONTROLLER;
//            $randomTerm = 'OAT Question';
//        }
        $randomTerm = 'OAT Question';
    }

    healthimo_asthma_log('random term ' . $randomTerm);
    if ($randomTerm == 'OAT Question') {
        // create asthma log here saying OAT question sent for this controller reminder type $response_type_expected
        $address = healthimo_profile_get_sms($user);
        $message_log = healthimo_asthma_message_log_get_default($user->uid, 2, $address, 'OAT Question', 0);
        $message_log->message_type = MESSAGE_TYPE_CONTROLLER_REMINDER;
        $message_log->response_expected = 1;
        $message_log->response_type_expected = $response_type_expected;
        $message_log->processed = time();
        $response = hfstx_ask_question($user, $number);
        if ($response) {
            // only save log if oat message was sent, 
            // may not have sent oat message, so try controller again 
            $message_log = healthimo_asthma_message_log_save($message_log);
        }
        return $response;
    } else if ($randomTerm == 'Asthma Zone') {
        $request = new stdClass();
        $request->nid = 0;
        $request->title = 'How are you feeling... what zone are you in? Reply with green, yellow or red.';
        $request->response_expected = 1;
        $request->response_type_expected = REQUEST_TYPE_ZONE;
        $reply = $request;
    } else if ($randomTerm == 'Asthma Controller') {
        $request = new stdClass();
        $request->nid = 0;
        $request->title = 'Everyone forgets their controller sometimes. How many times did you miss this week? Reply with the number you missed.';
        $request->response_expected = 1;
        $request->response_type_expected = REQUEST_TYPE_CONTROLLER;
        $reply = $request;
    } else {
        //get AM + randomTerm that hasn't been seen yet
        $unseenQuicktips = healthimo_asthma_get_unseen_am_quicktips($user, $randomTerm);
        $tipArray = array();
        while ($tip = db_fetch_object($unseenQuicktips)) {
            $tipArray[] = $tip;
        }

        if (count($tipArray) == 0) {
            watchdog('healthimo_asthma_controller_reminder', 'No unseen quicktips to pick from for controller reminder', array(), WATCHDOG_ALERT
            );
            return FALSE;
        }

        $randomTip = $tipArray[array_rand($tipArray)];
        $randomTip->response_expected = 0;
        $randomTip->response_type_expected = null;
        $reply = $randomTip;
    }


    if (healthimo_profile_has_sms($user)) {
        return healthimo_asthma_controller_reminder_send_to_sms($user, $reply);
    }

    return false;
}

/**
 * @param <type> $user
 * @return <type> A randomly selected quicktip from 'the pm collection' that hasn't been seen by this user before
 */
function healthimo_asthma_get_next_controller_reminder_pm($user) {
    $unseenQuicktips = healthimo_asthma_get_unseen_pm_quicktips($user);
    $tipArray = array();
    while ($tip = db_fetch_object($unseenQuicktips)) {
        $tipArray[] = $tip;
    }
    $randomTip = $tipArray[array_rand($tipArray)];
    $randomTip->response_expected = 0;
    $randomTip->response_type_expected = null;

    if (healthimo_profile_has_sms($user)) {
        return healthimo_asthma_controller_reminder_send_to_sms($user, $randomTip);
    }
}

function healthimo_asthma_get_unseen_pm_quicktips($user) {
    $term1 = healthimo_taxonomy_get_term_by_name(PM);

    return db_query(
            "
        SELECT c.*, n.* FROM
        content_type_quicktip c
        JOIN node n
            ON n.nid = c.nid
        JOIN term_node tn
            ON tn.nid = n.nid AND tn.vid = n.vid
        JOIN term_node tn2
            ON tn2.nid = n.nid AND tn2.vid = n.vid
        WHERE
            n.type = 'quicktip'
            AND
            tn.tid = %d
            AND
            (n.nid NOT IN (
                SELECT message_nid
                FROM {healthimo_asthma_message_log}
                WHERE uid = %d
                )
            )
        ORDER BY n.nid
        ", $term1->tid, $user->uid
    );
}

function healthimo_asthma_get_unseen_am_quicktips($user, $termName) {
    $termAm = healthimo_taxonomy_get_term_by_name(AM);
    $term1 = healthimo_taxonomy_get_term_by_name($termName);

    return db_query(
            "
        SELECT c.*, n.* FROM
        content_type_quicktip c
        JOIN node n
            ON n.nid = c.nid
        JOIN term_node tn
            ON tn.nid = n.nid AND tn.vid = n.vid
        JOIN term_node tn2
            ON tn2.nid = n.nid AND tn2.vid = n.vid
        WHERE
            n.type = 'quicktip'
            AND
            tn.tid = %d
            AND
            tn2.tid = %d
            AND
            (n.nid NOT IN (
                SELECT message_nid
                FROM {healthimo_asthma_message_log}
                WHERE uid = %d
                )
            )
        ORDER BY n.nid
        ", $termAm->tid, $term1->tid, $user->uid
    );
}