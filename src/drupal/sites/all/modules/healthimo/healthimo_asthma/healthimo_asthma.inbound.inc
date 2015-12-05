<?php

function healthimo_asthma_process_incoming_sms($user, $number, $message) {
    global $user;
    // set $reply and that will be returned to the user as a txt
    if ($user->uid > 0) {
        return healthimo_asthma_sms_handle_existing_user($number, $message);
    }
    return NULL;
}

function healthimo_asthma_is_enrolled($user) {
    return
    isset($user->profile_asthma_enrolled)
    &&
    $user->profile_asthma_enrolled == 1
    ;
}

function healthimo_asthma_sms_handle_existing_user($number, $message) {
    
    global $user;
    $reply = NULL;

    // if (healthimo_asthma_is_enrolled($user)) {
    healthimo_asthma_log("inbound for enrolled");
    //        
    $processContext = new stdClass();
    $processContext->message_log = $message_log;
    $processContext->user = $user;
    $processContext->number = $number;
    $processContext->message = $message;
    $processContext->reply = NULL;
    $processContext->previousState = NULL;
    $processContext->response_expected = 0;
    //
    $currentState = $user->profile_asthma_process_state;
    $inputType = HealthimoProcess::parseInputType($message);
    $asthmaProcess = new HealthimoAsthmaProcess($currentState, $processContext);
    $newState = $asthmaProcess->process($inputType);
    healthimo_asthma_log("newState $newState");
    //
    healthimo_profile_save($user, 'profile_asthma_process_state', $newState, PROFILE_CATEGORY_ASTHMA);
    healthimo_asthma_sms_log_reply($user, $number, $processContext->reply, $processContext->response_expected);
    healthimo_asthma_log("reply:" . $processContext->reply);

    if (strlen($processContext->reply) > 0) {
        return healthimo_sms_outgoing(
                $processContext->number, $processContext->reply, array('response_expected' => $processContext->response_expected,)
        );
    }
}

function healthimo_asthma_sms_log_reply($user, $number, $reply, $response_expected) {
    if (!empty($reply)) {
        watchdog('healthimo_asthma', 'healthimo_asthma_sms_log_reply to %uid %reply', array('%uid' => $user->uid, '%reply' => $reply));
//        drupal_set_header('Content-Type: text/plain');
//        print $reply;
        $message_log = healthimo_asthma_message_log_get_default($user->uid, 4, $number, $reply, 0);
        $message_log->message_type = MESSAGE_TYPE_GATEWAY_REPLY;
        $message_log->response_expected = $response_expected;
        $message_log->processed = time();
        $message_log = healthimo_asthma_message_log_save($message_log);
    }
}
