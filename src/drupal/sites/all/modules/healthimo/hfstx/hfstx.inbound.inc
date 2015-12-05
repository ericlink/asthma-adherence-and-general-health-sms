<?php

define('REPLY_KEYWORD_HEALTHY_START', 'Thank you for restarting your healthy family programs!');
define('REPLY_KEYWORD_HEALTHY', 'Welcome to Healthy Families, invite a friend!');
define('REPLY_KEYWORD_HEALTHY_BCBS', 'Thank you for your interest in your health plan sponsored program. A Care Specialist will contact you soon with more information.');
define('REPLY_KEYWORD_HEALTHY_CCISD', 'Welcome to Healthy Families for CCISD! Invite a friend');
define('REPLY_KEYWORD_HEALTHY_MOM', 'Welcome to Healthy Families for Mission of Mercy! Invite a friend');
define('REPLY_KEYWORD_HEALTHY_FLINT_HILLS', 'Welcome to Healthy Families for Flint Hills! Invite a friend');
define('REPLY_KEYWORD_HEALTHY_TAMUCC', 'Welcome to Healthy Families for TAMUCC! Invite a friend');
define('REPLY_KEYWORD_HEALTHY_HEB', 'Welcome to Healthy Families for HEB! Invite a friend');
define('REPLY_KEYWORD_HEALTHY_WALMART', 'Welcome to Healthy Families for Walmart! Invite a friend');
define('REPLY_KEYWORD_BCBS', 'Thanks, you\'re now enrolled in the healthimo bcbs program, TX ONLY fully covered modern, convenient diabetes education program!');
define('REPLY_KEYWORD_TTUHSCPB', 'Welcome to the healthimo program for TTUHSC PB members! One of our care specialists will contact you to set up your custom program');
define('REPLY_KEYWORD_CHALLENGE', 'Congrats! You have pre-registered for the 2011 Fit & Free By the Sea 6-Month Challenge starting in January. More texts coming soon!');
define('REPLY_KEYWORD_HEALTHY_ASTHMA', 'Lets setup your profile, do you take asthma controller medicine twice a day? (reply yes or no)');
//define('REPLY_KEYWORD_HEALTHY_ASTHMA', 'Thank you.  You are now in the healthimo asthma program and will begin getting asthma education on your phone.');
define('REPLY_KEYWORD_HEALTHY_ASTHMA_SPANISH', 'Gracias. Ahora se encuentra en el programa de asma healthimo y comenzara a recibir educacion sobre el asma en su telefono.');

function hfstx_process_incoming_sms($user, $number, $message, $isKeyword = false) {
    global $user;
// set $reply and that will be returned to the user as a txt
    $reply = NULL;
    if ($user->uid > 0) {
        $reply = hfstx_sms_handle_existing_user($number, $message, $isKeyword);
    } else {
        $reply = hfstx_sms_handle_new_user($number, $message);
    }

//hfstx_sms_reply($user, $reply);
    return $reply;
}

function hfstx_is_enrolled_in_hfstx($user) {
    return
    ( isset($user->profile_keyword_healthy)
    &&
    $user->profile_keyword_healthy == 1)
    ||
    ( isset($user->profile_keyword_bcbs)
    &&
    $user->profile_keyword_bcbs == 1)
    ;
}

function hfstx_sms_handle_existing_user($number, $message, $isKeyword = false) {
    global $user;
    $reply = NULL;

    if (hfstx_is_enrolled_in_hfstx($user)) {
        // save raw message inbound, may save again later more correlated/parsed etc...
        $message_log = hfstx_message_log_get_default($user->uid, 1, $number, $message, 0);
        $message_log->outbound = 0;
        $message_log->response_expected = 0;
        $message_log->processed = time();
        $message_log = hfstx_message_log_save($message_log);

        if ($isKeyword == true) {
            // we know this is a 'healthy' ... so process it as keyword only
            // nb: if first word of a response is healthy, it will be processed as a keyword
            $reply = hfstx_sms_manage_keywords($number, $message);
        } else {
            $message_log_last_request = hfstx_message_log_load_most_recent_request($user->uid);
            if ($message_log_last_request) {
                $reply = hfstx_sms_process_reply_for_data_request($number, $user, $message, $message_log_last_request);
            }
        }
    } else if (healthimo_asthma_is_enrolled($user)) {
        $reply = hfstx_sms_manage_keywords($number, $message);
    } else {
        $reply = hfstx_sms_manage_keywords($number, $message);
    }

    return $reply;
}

function hfstx_sms_process_reply_for_data_request($number, $user, $message, $message_log_last_request) {
    $value = $message;
//$cat = hfstx_sms_find_profile_category($user, $message_log_last_request);
// save the reply to the correct profile field category
    $user = healthimo_profile_save(
            $user, $message_log_last_request->response_type_expected, $value, $cat
    );
// saved ok so upated message log as received
    if ($user) {
        $message_log_last_request->response_received = time();
        if (hfstx_message_log_save($message_log_last_request)) {
            hfstx_message_log_mark_all_pending_requests_received($user->uid);
        }

        $sentAsthmaProfileQuestion = false;

        if ($message_log_last_request->response_type_expected == 'profile_diabetes_asthma_in_home') {
// message is already cleaned and lowercased upstream
            $words = healthimo_string_get_words($value);
// log message
            if (in_array('d', $words) && strcmp('d', $words[0]) == 0) {
                $user = healthimo_profile_save(
                        $user, 'profile_has_diabetes_in_household', 1, NULL
                );
                $user = healthimo_profile_save(
                        $user, 'profile_areas_of_interest_diabetes', 1, NULL
                );
            } else if (in_array('a', $words) && strcmp('a', $words[0]) == 0) {
                $user = healthimo_profile_save(
                        $user, 'profile_has_asthma_in_household', 1, NULL
                );
                hfstx_kickoff_asthma_profile($user, $number);
                $sentAsthmaProfileQuestion = true;
            } else if (in_array('b', $words) && strcmp('b', $words[0]) == 0) {
                $user = healthimo_profile_save(
                        $user, 'profile_has_diabetes_in_household', 1, NULL
                );
                $user = healthimo_profile_save(
                        $user, 'profile_areas_of_interest_diabetes', 1, NULL
                );
                $user = healthimo_profile_save(
                        $user, 'profile_has_asthma_in_household', 1, NULL
                );
                hfstx_kickoff_asthma_profile($user, $number);
                $sentAsthmaProfileQuestion = true;
            }
        }

        if ($sentAsthmaProfileQuestion == false) {
            hfstx_ask_question($user, $number);
        }
    } else {
        watchdog('hfstx_inbound', 'Error saving reply for message=[%msg], user=%uid', array('%msg' => $message, '%uid' => $user->uid), WATCHDOG_ALERT
        );
    }


    return NULL; //no replies for these right now
}

function hfstx_kickoff_asthma_profile($user, $number) {
    if (!isset($account->profile_keyword_asthma)
            || empty($account->profile_keyword_asthma)
            || $account->profile_keyword_asthma == 0
    ) {
        healthimo_profile_save($user, 'profile_asthma_process_state', HealthimoProcess::REQUEST_CONTROLLER_STATUS, PROFILE_CATEGORY_ASTHMA);
        healthimo_profile_save($user, 'profile_asthma_enrolled', 0, PROFILE_CATEGORY_ASTHMA);
        healthimo_profile_save($user, 'profile_asthma_feature_controller_reminder_active', 0, PROFILE_CATEGORY_ASTHMA);
        healthimo_profile_save($user, 'profile_keyword_asthma', 1, NULL);
        healthimo_profile_save($user, 'profile_keyword_healthy', 1, NULL);
        $message_log = healthimo_asthma_message_log_get_default($user->uid, 4, $number, $message, 0);
        $message_log->outbound = 0;
        healthimo_asthma_message_log_save($message_log);
        $message_log = healthimo_asthma_message_log_get_default($user->uid, 4, $number, REPLY_KEYWORD_HEALTHY_ASTHMA, 0, 1, 1);
        healthimo_asthma_message_log_save($message_log);
        healthimo_sms_outgoing(
                $number, REPLY_KEYWORD_HEALTHY_ASTHMA, array('response_expected' => 1,)
        );

// set question count to ~'high values'
// so no hfstx questions are asked during the enrollment session
// will be reset to 0 on next cron initiated message send
        $account = healthimo_profile_save(
                $account, 'profile_question_count', 99, PROFILE_CATEGORY_PHR
        );
    }
}

//function hfstx_sms_find_profile_category($user, $message_log_last_request) {
//    $cat = NULL;
//
//    if (array_key_exists(
//                    $message_log_last_request->response_type_expected, healthimo_profile_save_get_edit_array($user, PROFILE_CATEGORY_PERSONAL_INFO)
//            )
//    ) {
//        $cat = PROFILE_CATEGORY_PERSONAL_INFO;
//    } elseif (array_key_exists(
//                    $message_log_last_request->response_type_expected, healthimo_profile_save_get_edit_array($user, PROFILE_CATEGORY_PHR)
//            )
//    ) {
//        $cat = PROFILE_CATEGORY_PHR;
//    } else {
//        watchdog('hfstx_inbound', 'Reply field not found for response type=%rt, category=%cat, user=%uid', array(
//            '%rt' => $message_log_last_request->response_type_expected,
//            '%cat' => $cat,
//            '%uid' => $user->uid
//                ), WATCHDOG_ALERT
//        );
//    }
//
//    return $cat;
//}
function hfstx_sms_update_keyword($keywordField) {
    global $user;

    $user = healthimo_profile_save($user, $keywordField, 1, PROFILE_CATEGORY_MOBILE);

    if (!$user) {
        watchdog('hfstx', 'hfstx_sms_handle_new_user error setting keyword');
    }

    return $user;
}

function hfstx_sms_handle_new_user($number, $message) {
    global $user;
    $user = hfstx_sms_create_new_account($number);
    if ($user) {
        watchdog('hfstx', 'hfstx_sms_handle_new_user created account ' . $user->uid);
        return hfstx_sms_manage_keywords($number, $message);
    } else {
        watchdog('hfstx', 'hfstx_sms_handle_new_user error creating account');
        return '';
    }
}

function hfstx_sms_manage_keywords($number, $message) {
    global $user;
//$is_enrolled_healthy = hfstx_is_enrolled_in_hfstx($user);
    $reply = NULL;
    $response_expected = 0;
    if (healthimo_contains_word_ignore_case('healthy', $message)) {
        if (healthimo_contains_word_ignore_case('ccisd', $message)) {
            $reply = REPLY_KEYWORD_HEALTHY_CCISD;
            hfstx_sms_update_keyword('profile_keyword_healthy');
        } else if (healthimo_contains_word_ignore_case('mom', $message)) {
            $reply = REPLY_KEYWORD_HEALTHY_MOM;
            hfstx_sms_update_keyword('profile_keyword_healthy');
        } else if (healthimo_contains_word_ignore_case('flint hills', $message)) {
            $reply = REPLY_KEYWORD_HEALTHY_FLINT_HILLS;
            hfstx_sms_update_keyword('profile_keyword_healthy');
        } else if (healthimo_contains_word_ignore_case('tamucc', $message)) {
            $reply = REPLY_KEYWORD_HEALTHY_TAMUCC;
            hfstx_sms_update_keyword('profile_keyword_healthy');
        } else if (healthimo_contains_word_ignore_case('healthy asthma', $message)) {
            $reply = REPLY_KEYWORD_HEALTHY_ASTHMA;
            $response_expected = 1;
            healthimo_profile_save($user, 'profile_asthma_process_state', HealthimoProcess::REQUEST_CONTROLLER_STATUS, PROFILE_CATEGORY_ASTHMA);
            healthimo_profile_save($user, 'profile_asthma_enrolled', 0, PROFILE_CATEGORY_ASTHMA);
            healthimo_profile_save($user, 'profile_asthma_feature_controller_reminder_active', 0, PROFILE_CATEGORY_ASTHMA);
            healthimo_profile_save($user, 'profile_keyword_asthma', 1, NULL);
            healthimo_profile_save($user, 'profile_keyword_healthy', 1, NULL);
            $message_log = healthimo_asthma_message_log_get_default($user->uid, 4, $number, $message, 0);
            $message_log->outbound = 0;
            healthimo_asthma_message_log_save($message_log);
            $message_log = healthimo_asthma_message_log_get_default($user->uid, 4, $number, REPLY_KEYWORD_HEALTHY_ASTHMA, 0);
            healthimo_asthma_message_log_save($message_log);
        } else if (healthimo_contains_word_ignore_case('ttuhsc pb', $message)) {
            $reply = REPLY_KEYWORD_TTUHSCPB;
            hfstx_sms_update_keyword('profile_keyword_healthy');
            watchdog('hfstx_keyword_alert', 'New ttuhscpb registration', array(), WATCHDOG_ALERT
            );
        } else if (healthimo_contains_word_ignore_case('tech pb', $message)) {
            $reply = REPLY_KEYWORD_TTUHSCPB;
            hfstx_sms_update_keyword('profile_keyword_healthy');
            watchdog('hfstx_keyword_alert', 'New ttuhscpb registration', array(), WATCHDOG_ALERT
            );
        } else if (healthimo_contains_word_ignore_case('heb', $message)) {
            $reply = REPLY_KEYWORD_HEALTHY_HEB;
            hfstx_sms_update_keyword('profile_keyword_healthy');
        } else if (healthimo_contains_word_ignore_case('walmart', $message)) {
            $reply = REPLY_KEYWORD_HEALTHY_WALMART;
            hfstx_sms_update_keyword('profile_keyword_healthy');
        } else if (healthimo_contains_word_ignore_case('challenge', $message)) {
            $reply = REPLY_KEYWORD_CHALLENGE;
            hfstx_sms_update_keyword('profile_keyword_challenge');
        } else if (healthimo_contains_word_ignore_case('start', $message)) {
            $reply = REPLY_KEYWORD_HEALTHY_START;
        } else if (healthimo_contains_word_ignore_case('bcbs', $message)) {
            $reply = REPLY_KEYWORD_HEALTHY_BCBS;
            hfstx_sms_update_keyword('profile_keyword_bcbs');
        } else {
            $reply = REPLY_KEYWORD_HEALTHY;
            hfstx_sms_update_keyword('profile_keyword_healthy');
        }
    } else if (healthimo_contains_word_ignore_case('bcbs', $message)) {
        $reply = REPLY_KEYWORD_BCBS;
        hfstx_sms_update_keyword('profile_keyword_bcbs');
        watchdog('hfstx_keyword_alert', 'New bcbs registration', array(), WATCHDOG_ALERT
        );
    } else if (healthimo_contains_word_ignore_case('asma', $message)) {
        $reply = REPLY_KEYWORD_HEALTHY_ASTHMA_SPANISH;
        hfstx_sms_update_keyword('profile_keyword_asma');
        hfstx_sms_update_keyword('profile_keyword_spanish');
    }

    $user = healthimo_profile_save(
            $user, 'profile_keyword_log', $user->profile_keyword_log . "\n" . gmdate(DATE_ATOM, time()) . ' - ' . $message, PROFILE_CATEGORY_MOBILE
    );


    if (strlen($reply) > 0) {
        healthimo_sms_outgoing(
                $number, $reply, array('response_expected' => $response_expected,)
        );
    }
}

function hfstx_sms_reply($user, $reply) {
    if (!empty($reply)) {
        watchdog('hfstx', 'hfstx_sms_reply to %uid %reply', array('%uid' => $user->uid, '%reply' => $reply));
        drupal_set_header('Content-Type: text/plain');
        print $reply;
    }
}

function hfstx_sms_create_new_account($number) {
    $sms_user[0] = array(
        status => 2,
        number => $number,
    );

// just a random sha hash including time so we can set the password
// we don't know/don't care what it is
// it's secure and will need to be reset by the user if they register via email later
    $token = base64_encode(hash_hmac('sha256', $number, drupal_get_private_key() . time(), TRUE));
    $token = strtr($token, array('+' => '-', '/' => '_', '=' => ''));

    $details = array(
        'name' => $number,
        'pass' => $token,
        'mail' => $number,
        'access' => 0,
        'status' => 1,
        'sms_user' => $sms_user
    );
    return user_save(null, $details);
}

