<?php

define('DEFAULT_REPLY', 'If this is a medical emergency, call 911');

define('MESSAGE_TYPE_ZIPIT_RELIEVER', 'message_type_reliever'); //string in zipit app
define('MESSAGE_TYPE_ZIPIT_SURVEY', 'message_type_survey'); //string in zipit app

define('REQUEST_TYPE_ZONE', 'zone');
define('REQUEST_TYPE_CONTROLLER', 'controller');

define('MESSAGE_TYPE_CONTROLLER_REMINDER', 'controller_reminder');
define('MESSAGE_TYPE_OZONE_ALERT', 'ozone_alert');
define('MESSAGE_TYPE_POLLEN_ALERT', 'pollen_alert');
define('MESSAGE_TYPE_APPOINTMENT_REMINDER', 'appointment_reminder');
define('MESSAGE_TYPE_RESPONSE', 'response');
define('MESSAGE_TYPE_UNSOLICITED', 'unsolicited');
define('MESSAGE_TYPE_GATEWAY_REPLY', 'gateway reply');

/**
 * @return <type> user ids for all users enrolled in the asthma program
 */
function healthimo_asthma_find_users() {
    $admin_only_clause = variable_get('healthimo_messaging_outbound_to_admin_user_only', 1) == 1 ? " WHERE u.uid = 1" : "";

    return db_query(
            "
        SELECT DISTINCT u.uid
        FROM {users} u
        JOIN {profile_values} pv ON pv.uid = u.uid
        AND
        pv.value = 1
        AND
        pv.fid IN (
            SELECT pf.fid
            FROM {profile_fields} pf
            WHERE
            pf.name = 'profile_asthma_enrolled'
        )
     " . $admin_only_clause
    );
}

function healthimo_asthma_log($log, $level = WATCHDOG_DEBUG) {
    $detailed = variable_get('healthimo_healthimo_asthma_controller_reminder_logging', 0);
    if ($detailed) {
        watchdog('healthimo_asthma_detail', $log, NULL, $level);
    } else if ($level < WATCHDOG_WARNING) {
        watchdog('healthimo_asthma', $log, NULL, $level);
    }
}

/**
 * sends sms when response is not expected
 * @param <type> $user
 * @param <type> $address
 * @param <type> $message
 * @param <type> $message_type
 * @return <type> true if sent ok
 */
function healthimo_asthma_send_to_sms($user, $address, $message, $message_type) {
    //$address = DEBUG ? '2146415465' : $address;
    $address = $address;
    healthimo_asthma_log(' send sms to ' . $address);
    $message_nid = 0;
    $message_log = healthimo_asthma_message_log_get_default($user->uid, 2, $address, $message, $message_nid);
    $message_log->message_type = $message_type;
    $message_log->response_expected = 0;
    $message_log->response_type_expected = null;

    $message_log = healthimo_asthma_message_log_save($message_log);
    //if (SEND_ENABLED) {
    //
        //healthimo_sms_send
    if (healthimo_sms_outgoing($address, trim($message_log->message), array('response_expected' => $message_log->response_expected,))) {
        //    if ( sms_send($address, trim($message_log->message)) ) {
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

function healthimo_asthma_send_to_email($user, $address, $subject, $body, $message_type) {
    //$address = DEBUG ? 'link@sandlion.com' : $address;
    $address = $address;
    healthimo_asthma_log(' send email to ' . $address);
    $message_nid = 0;
    $message_log = healthimo_asthma_message_log_get_default($user->uid, 2, $address, $body, $message_nid);
    $message_log->message_type = $message_type;
    $message_log->response_expected = 0;
    $message_log->response_type_expected = null;

    $message_log = healthimo_asthma_message_log_save($message_log);

    $email = array(
        'to' => $address,
        'subject' => $subject,
        'body' => $body,
        'headers' => array('From' => 'guide@healthimo.com'),
    );


    //if (SEND_ENABLED) {
    $status = drupal_mail_send($email);
    $message_log->processed = time();
    $message_log = healthimo_asthma_message_log_save($message_log);
//    } else {
//        //for debug
//        $message_log->processed = time();
//        $message_log = healthimo_asthma_message_log_save($message_log);
//        $status = TRUE;
//    }

    return $status;
}

function healthimo_asthma_send_to_distro_list($user, $subject, $message, $message_type) {
    $status = FALSE;

    if (isset($user->profile_asthma_notification_addresses)
            && !empty($user->profile_asthma_notification_addresses)
    ) {
        $distro_list = healthimo_string_get_newline_delimited_items(trim($user->profile_asthma_notification_addresses));
    } else {
        return TRUE;
    }

    foreach ($distro_list as $address) {
        $address = trim($address);
        if (strpos($address, '@') === FALSE) {
            $status = healthimo_asthma_send_to_sms($user, $address, $message, $message_type) && $status;
        } else {
            $status = healthimo_asthma_send_to_email(
                            $user, $address, $subject, $message, $message_type
                    ) && $status;
        }
    }

    return $status;
}

function healthimo_asthma_send_to_all_destinations($user, $subject, $message, $message_type) {
    $status = FALSE;

    if (healthimo_profile_has_sms($user)) {
        $status = healthimo_asthma_send_to_sms($user, healthimo_profile_get_sms($user), $message, $message_type);
    }

    return healthimo_asthma_send_to_distro_list($user, $subject, $message, $message_type) && $status;
}
