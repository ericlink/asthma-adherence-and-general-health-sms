<?php

/**
 * Display the sms blast form
 */
function healthimo_sms_form() {
    $form['number'] = array(
        '#type' => 'textfield',
        '#title' => t('Number'),
        '#cols' => 20,
        '#maxlength' => 20,
        '#required' => TRUE,
        '#default_value' => '2146415465',
    );
    $form['message'] = array(
        '#type' => 'textfield',
        '#title' => t('Sms'),
        '#cols' => 130,
        '#maxlength' => 130,
        '#required' => TRUE,
    );
    $form['express'] = array(
        '#type' => 'checkbox',
        '#title' => t('Express'),
        '#cols' => 60,
    );

    $form['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Send'),
    );

    return $form;
}

/**
 * Submit handler for the sms blast form
 */
function healthimo_sms_form_submit(&$form, $form_state) {
//  $result = db_query("SELECT uid FROM {sms_user} WHERE status = 2");
//  while ($row = db_fetch_array($result)) {
//    $users[] = $row;
//  }
//
//  foreach ($users as $user) {
//    sms_user_send($user['uid'], $form_state['values']['message']);
//  }
//  drupal_set_message(t('The message was sent to %count users.', array('%count' => count($users))));
    healthimo_log('Submitting test sms message');

    $express = $form_state['values']['express'] ? 1 : 0;

//  rc? message? function sms_clubtexting_error_codes() {
    $rc = healthimo_sms_outgoing(
            $form_state['values']['number'], $form_state['values']['message'], array('response_expected' => $express,)
    );

//    sms_send(
//            '2146415465',
//            $form_state['values']['message']
//    );
    if ($rc) {
        drupal_set_message(t('The message was sent'));
    } else {
        drupal_set_message(t('The message was not sent, see message log'));
    }
}

/**
 * Lookup smtp status and send using smtp if possible
 * Callback for sending messages.
 */
//don't name healthimo_sms_send, when it's not part of a gateway module it's still called by sms_send == LOOP!
function healthimo_sms_outgoing($number, $message, $options) {
    global $user;
    watchdog('healthimo_sms_outgoing', '%uid %phone %reply', array('%uid' => $user->uid, '%phone' => $number, '%reply' => $message));

    /// log only?
    if (strcmp('log', variable_get('sms_default_gateway', 'log')) == 0) {
        watchdog('healthimo_sms_outgoing', 'logging only reply %uid %phone %reply', array('%uid' => $user->uid, '%phone' => $number, '%reply' => $message));
        //drupal_set_header('Content-Type: text/plain');
        //print $reply;
        return TRUE;
    }

    /// twilio?
    if (strcmp('twilio', variable_get('sms_default_gateway', 'twilio')) == 0) {
        return sms_twilio_send($number, $message, $options);
    }

    /// club texting?
    if (strcmp('clubtexting', variable_get('sms_default_gateway', 'clubtexting')) == 0) {
        return sms_clubtexting_send($number, $message, $options);
    }

    /// no gateway 
    watchdog('healthimo_sms_outgoing', 'No gateway configured');
    return FALSE;

//    $phone_info = phone_info_get_info($number);
//    if ($phone_info->is_wireless && $phone_info->smtp_address) {
//        $email = array(
//            'to' => $phone_info->smtp_address,
//            'body' => $message,
//            'headers' => array('From' => 'txt@healthimo.com'),
//        );
//        healthimo_log_debug('healthimo_sms', 'mailing %t [%b]', array('%t' => $email['to'], '%f' => $email['headers']['From'], '%b' => $email['body']));
//        return drupal_mail_send($email);
//    } else {
//        healthimo_log_debug('healthimo_sms', 'sending to %p via sms_send', array('%p' => $number));
//        // send w/ default sms_framework gateway (club texting)
//        return sms_send($number, $message, $options);
//    }
}

// hook for sms framework module incoming messages
// handle all incoming in one function since we want to 
// deal with keywords and stop before any other module is called
function healthimo_sms_incoming($op, $number, $message, $options = array()) {
    global $user;
    switch ($op) {
//        case 'pre process':
//            break;
//        case 'post process':
//            break;
        case 'process':
            //echo "USER" . $user->uid;
            $cleanNumber = healthimo_string_clean_input($number);
            $cleanMessage = healthimo_string_clean_input($message);
            watchdog('healthimo_sms_process_inbound', $cleanNumber . ' [' . $cleanMessage . ']');
            $cleanMessage = trim($cleanMessage);
            $cleanMessage = strtolower($cleanMessage);
            healthimo_sms_process_inbound($user, $cleanNumber, $cleanMessage);
            session_destroy();
            $user = drupal_anonymous_user();
            break;
    }
}

function healthimo_sms_process_inbound($user, $cleanNumber, $cleanMessage) {
    global $user;

    $words = healthimo_string_get_words($cleanMessage);
    // log message
    if (in_array('stop', $words) && strcmp('stop', $words[0]) == 0) {
        watchdog('healthimo_sms_process_inbound', 'stop received for user %uid', array('%uid' => $user->uid), WATCHDOG_ERROR, l('Edit Mobile Profile', '../../../user/' . $user->uid . '/edit/Mobile'));
        $user = healthimo_profile_save($user, 'profile_keyword_stop', 1, PROFILE_CATEGORY_MOBILE);
        // can't send to them anymore, they are opted out at txt gateway, smtp could
        //$optOutReply = 'You have been opted out of all healthimo programs. To start again, text HEALTHY START to 25827';
        //healthimo_sms_outgoing($cleanNumber, $optOutReply);
    } else if (in_array('healthy', $words) && strcmp('healthy', $words[0]) == 0) {
        $isKeyword = true;
        // is keyword? process as keyword
        watchdog('healthimo_sms_process_inbound', 'healthy ' . $user->uid . ' ' . $words[0] . ' ' . $cleanNumber . ' [' . $cleanMessage . ']');
        //ensure opted in, multiple profile field updates not working, so do direct update
        hfstx_process_incoming_sms($user, $cleanNumber, $cleanMessage, $isKeyword);
        healthimo_profile_save($user, 'profile_keyword_stop', 0, PROFILE_CATEGORY_MOBILE);
    } else {
        $asthmaSentReply = false;
        // is not a keyword? process by features
        if ($user->profile_keyword_asthma == 1) {//|| $user->profile_keyword_asma == 1) {
            $asthmaSentReply = healthimo_asthma_process_incoming_sms($user, $cleanNumber, $cleanMessage);
        }

        // process the reply trough hfstx also 
        if ($asthmaSentReply == false && hfstx_is_enrolled_in_hfstx($user)) {
            // process replies to hfstx requests 
            // may have a reply if asking profile questions
            hfstx_process_incoming_sms($user, $cleanNumber, $cleanMessage);
        }
    }
}