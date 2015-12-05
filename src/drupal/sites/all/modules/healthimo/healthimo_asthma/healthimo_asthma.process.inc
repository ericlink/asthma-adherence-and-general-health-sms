<?php

require_once 'FSM.php';

//require_once 'FSM/GraphViz.php';
//include 'HealthimoInboundMessage.php';
//include 'HealthimoProcess.php';
//asthma_process_test_with_controller();
//function asthma_process_test_with_controller() {
//    //$inboundMessage = new HealthimoInboundMessage();
//    $inboundMessage;
//    //$inboundMessage->hasWordStop;
//    //$asthmaProcess = new HealthimoAsthmaProcess(HealthimoProcess::START, new stdClass());
//    $asthmaProcess = new HealthimoAsthmaProcess(HealthimoProcess::START, $inboundMessage);
//    $asthmaProcess->process(HealthimoProcess::INPUT_HEALTHY_ASTHMA);
//    $asthmaProcess->process(HealthimoProcess::INPUT_YES);
//    $asthmaProcess->process(HealthimoProcess::INPUT_TWO_VALID_TIMES_OF_DAY);
//    $asthmaProcess->process(HealthimoProcess::INPUT_GENERAL_MESSAGE);
//    $asthmaProcess->process(HealthimoProcess::MESSAGE_TRIGGER);
//    $asthmaProcess->process(HealthimoProcess::MESSAGE_TRIGGER);
//    $asthmaProcess->process(HealthimoProcess::INPUT_GENERAL_MESSAGE);
//    $asthmaProcess->process(HealthimoProcess::INPUT_GENERAL_MESSAGE);
//    $asthmaProcess->process(HealthimoProcess::INPUT_GENERAL_MESSAGE);
//    $asthmaProcess->process(HealthimoProcess::INPUT_GENERAL_MESSAGE);
//}

/*
 * graphviz requires functions for drawing, so use scoped names on regular php functions (insted of methods) to make it happy (they are just visible in configure process so no biggie)
 */

class HealthimoAsthmaProcess {

    private $data;
    private $fsm;

    public function __construct($current_state, &$data = null) {
        $this->data = $data;
        $this->data->previousState = $current_state;
        $this->configureProcess($current_state);
        //$this->exportImage();
    }

    private function configureProcess($current_state) {
        $process_name = 'healthimo_asthma_process';
        message("$process_name\n");
        // process factory.. configure process, set start state and data pointer
        $fsm = new FSM($current_state, $this->data);
        // enroll
        $fsm->addTransition(HealthimoProcess::INPUT_HEALTHY_ASTHMA, HealthimoProcess::START, HealthimoProcess::REQUEST_CONTROLLER_STATUS, 'healthimo_asthma_process_get_controller_status');
        // controller?
        $fsm->addTransition(HealthimoProcess::INPUT_YES, HealthimoProcess::REQUEST_CONTROLLER_STATUS, HealthimoProcess::REQUEST_CONTROLLER_TIMES, 'healthimo_asthma_process_request_controller_times');
        $fsm->addTransition(HealthimoProcess::INPUT_NO, HealthimoProcess::REQUEST_CONTROLLER_STATUS, HealthimoProcess::REQUEST_EDUCATION_TIME, 'healthimo_asthma_process_request_education_time');
        $fsm->addTransition(HealthimoProcess::INPUT_HEALTHY_ASTHMA, HealthimoProcess::REQUEST_CONTROLLER_STATUS, HealthimoProcess::REQUEST_CONTROLLER_STATUS, 'healthimo_asthma_process_get_controller_status');
        $fsm->addTransition(HealthimoProcess::INPUT_TWO_VALID_TIMES_OF_DAY, HealthimoProcess::REQUEST_CONTROLLER_STATUS, HealthimoProcess::REQUEST_CONTROLLER_STATUS, 'healthimo_asthma_process_get_controller_status');
        $fsm->addTransition(HealthimoProcess::INPUT_ONE_VALID_TIME_OF_DAY, HealthimoProcess::REQUEST_CONTROLLER_STATUS, HealthimoProcess::REQUEST_CONTROLLER_STATUS, 'healthimo_asthma_process_get_controller_status');
        $fsm->addTransition(HealthimoProcess::INPUT_GENERAL_MESSAGE, HealthimoProcess::REQUEST_CONTROLLER_STATUS, HealthimoProcess::REQUEST_CONTROLLER_STATUS, 'healthimo_asthma_process_get_controller_status');
        // profile configuration
        // controller times
        $fsm->addTransition(HealthimoProcess::INPUT_TWO_VALID_TIMES_OF_DAY, HealthimoProcess::REQUEST_CONTROLLER_TIMES, HealthimoProcess::PROFILE_COMPLETE, 'healthimo_asthma_process_receive_controller_times');

        $fsm->addTransition(HealthimoProcess::INPUT_HEALTHY_ASTHMA, HealthimoProcess::REQUEST_CONTROLLER_TIMES, HealthimoProcess::REQUEST_CONTROLLER_TIMES, 'healthimo_asthma_process_request_controller_times');
        $fsm->addTransition(HealthimoProcess::INPUT_YES, HealthimoProcess::REQUEST_CONTROLLER_TIMES, HealthimoProcess::REQUEST_CONTROLLER_TIMES, 'healthimo_asthma_process_request_controller_times');
        $fsm->addTransition(HealthimoProcess::INPUT_NO, HealthimoProcess::REQUEST_CONTROLLER_TIMES, HealthimoProcess::REQUEST_CONTROLLER_TIMES, 'healthimo_asthma_process_request_controller_times');
        $fsm->addTransition(HealthimoProcess::INPUT_ONE_VALID_TIME_OF_DAY, HealthimoProcess::REQUEST_CONTROLLER_TIMES, HealthimoProcess::REQUEST_CONTROLLER_TIMES, 'healthimo_asthma_process_request_controller_times');
        $fsm->addTransition(HealthimoProcess::INPUT_GENERAL_MESSAGE, HealthimoProcess::REQUEST_CONTROLLER_TIMES, HealthimoProcess::REQUEST_CONTROLLER_TIMES, 'healthimo_asthma_process_request_controller_times');
        // education time
        $fsm->addTransition(HealthimoProcess::INPUT_ONE_VALID_TIME_OF_DAY, HealthimoProcess::REQUEST_EDUCATION_TIME, HealthimoProcess::PROFILE_COMPLETE, 'healthimo_asthma_process_receive_education_time');

        $fsm->addTransition(HealthimoProcess::INPUT_HEALTHY_ASTHMA, HealthimoProcess::REQUEST_EDUCATION_TIME, HealthimoProcess::REQUEST_EDUCATION_TIME, 'healthimo_asthma_process_request_education_time');
        $fsm->addTransition(HealthimoProcess::INPUT_YES, HealthimoProcess::REQUEST_EDUCATION_TIME, HealthimoProcess::REQUEST_EDUCATION_TIME, 'healthimo_asthma_process_request_education_time');
        $fsm->addTransition(HealthimoProcess::INPUT_NO, HealthimoProcess::REQUEST_EDUCATION_TIME, HealthimoProcess::REQUEST_EDUCATION_TIME, 'healthimo_asthma_process_request_education_time');
        $fsm->addTransition(HealthimoProcess::INPUT_TWO_VALID_TIMES_OF_DAY, HealthimoProcess::REQUEST_EDUCATION_TIME, HealthimoProcess::REQUEST_EDUCATION_TIME, 'healthimo_asthma_process_request_education_time');
        $fsm->addTransition(HealthimoProcess::INPUT_GENERAL_MESSAGE, HealthimoProcess::REQUEST_EDUCATION_TIME, HealthimoProcess::REQUEST_EDUCATION_TIME, 'healthimo_asthma_process_request_education_time');
        // welcome message
        // just send welcome from profile complete  
        // for diagram
        $fsm->addTransition(HealthimoProcess::PROFILE_COMPLETE, HealthimoProcess::PROFILE_COMPLETE, HealthimoProcess::ACTIVE, 'healthimo_asthma_process_active'); //for diagram
        // active messaging
        $fsm->addTransition(HealthimoProcess::INPUT_HEALTHY_ASTHMA, HealthimoProcess::ACTIVE, HealthimoProcess::ACTIVE, 'healthimo_asthma_process_active_message_input');
        $fsm->addTransition(HealthimoProcess::INPUT_ONE_VALID_TIME_OF_DAY, HealthimoProcess::ACTIVE, HealthimoProcess::ACTIVE, 'healthimo_asthma_process_active_message_input');
        $fsm->addTransition(HealthimoProcess::INPUT_TWO_VALID_TIMES_OF_DAY, HealthimoProcess::ACTIVE, HealthimoProcess::ACTIVE, 'healthimo_asthma_process_active_message_input');
        $fsm->addTransition(HealthimoProcess::INPUT_YES, HealthimoProcess::ACTIVE, HealthimoProcess::ACTIVE, 'healthimo_asthma_process_active_message_input');
        $fsm->addTransition(HealthimoProcess::INPUT_NO, HealthimoProcess::ACTIVE, HealthimoProcess::ACTIVE, 'healthimo_asthma_process_active_message_input');
        $fsm->addTransition(HealthimoProcess::INPUT_GENERAL_MESSAGE, HealthimoProcess::ACTIVE, HealthimoProcess::ACTIVE, 'healthimo_asthma_process_active_message_input');
        $fsm->addTransition(HealthimoProcess::MESSAGE_TRIGGER, HealthimoProcess::ACTIVE, HealthimoProcess::ACTIVE, 'healthimo_asthma_process_active_message');
        // done messaging
        //$fsm->addTransition(PROTOCOL_COMPLETE, ACTIVE, SURVEY, 'healthimo_asthma_process_survey');
        //for diagram
        $fsm->addTransition(HealthimoProcess::PROTOCOL_COMPLETE, HealthimoProcess::ACTIVE, HealthimoProcess::PROTOCOL_COMPLETE); //for diagram
        $fsm->addTransition(HealthimoProcess::MESSAGE_TRIGGER, HealthimoProcess::PROTOCOL_COMPLETE, HealthimoProcess::SURVEY, 'healthimo_asthma_process_survey');
        // survey
        $fsm->addTransition(HealthimoProcess::INPUT_HEALTHY_ASTHMA, HealthimoProcess::SURVEY, HealthimoProcess::END, 'healthimo_asthma_process_end');
        $fsm->addTransition(HealthimoProcess::INPUT_ONE_VALID_TIME_OF_DAY, HealthimoProcess::SURVEY, HealthimoProcess::END, 'healthimo_asthma_process_end');
        $fsm->addTransition(HealthimoProcess::INPUT_TWO_VALID_TIMES_OF_DAY, HealthimoProcess::SURVEY, HealthimoProcess::END, 'healthimo_asthma_process_end');
        $fsm->addTransition(HealthimoProcess::INPUT_YES, HealthimoProcess::SURVEY, HealthimoProcess::END, 'healthimo_asthma_process_end');
        $fsm->addTransition(HealthimoProcess::INPUT_NO, HealthimoProcess::SURVEY, HealthimoProcess::END, 'healthimo_asthma_process_end');
        $fsm->addTransition(HealthimoProcess::INPUT_GENERAL_MESSAGE, HealthimoProcess::SURVEY, HealthimoProcess::END, 'healthimo_asthma_process_end');
        // done
        // any at end is no-op end 
        // $fsm->addTransition(INPUT_GENERAL_MESSAGE, END, END, 'healthimo_asthma_process_end');
        //nop if don't register, otherwise transitions to this one specified here
        //$fsm->setDefaultTransition(END, 'healthimo_asthma_process_default_transition');
        $this->fsm = $fsm;
    }

    public function process($event_token) {
        message("fsm->process($event_token|" . $this->data->previousState . ")");
        //print_r($this->data);
        $this->fsm->process($event_token);
        $this->data->previousState = $this->fsm->getCurrentState();
        message("new state is " . $this->fsm->getCurrentState() . "");
        return $this->fsm->getCurrentState();
    }

    private function exportImage() {
        $outfile = fopen('asthma_process.svg', 'w');
        $converter = new FSM_GraphViz($this->fsm);
        fwrite($outfile, $converter->export()->fetch());
        fclose($outfile);
    }

}

////////////////////////////////////////////////////////////////////////////////////
// methods that call out to external dependencies (from state machine callbacks)
////////////////////////////////////////////////////////////////////////////////////
function healthimo_process_send_message(&$data, $msg) {
    message("healthimo_process_send_message");
    $data->response_expected = 0;
    $data->reply = $msg;
}

function healthimo_process_reply(&$data, $msg) {
    message("healthimo_process_reply");
    $data->response_expected = 1;
    $data->reply = $msg;
}

function healthimo_process_save_data($data) {
    message("healthimo_process_save_data $data");
}

function message($msg) {
    watchdog('healthimo_asthma_process', $msg);
    //echo $msg . "\n";
}

////////////////////////////////////////////////////////////////////////////////////
// state machine callbacks
////////////////////////////////////////////////////////////////////////////////////
function healthimo_asthma_process_default_transition($symbol, &$data) {
    message("healthimo_asthma_process_default_transition: $symbol");
}

function healthimo_asthma_process_get_controller_status($symbol, &$data) {
    $message_log = healthimo_asthma_message_log_get_default($data->user->uid, 4, $data->number, $data->message, 0, 0);
    $message_log->processed = time();
    $message_log->response_expected = 0;
    $message_log->outbound = 0;
    $message_log = healthimo_asthma_message_log_save($message_log);

    healthimo_process_reply($data, 'Lets setup your profile, do you take asthma controller medicine twice a day? (reply yes or no)');
}

function healthimo_asthma_process_active($symbol, &$data) {
    message("healthimo_asthma_process_active");
}

function healthimo_asthma_process_request_controller_times($symbol, &$data) {
    message("healthimo_asthma_process_request_controller_times");
    $message_log = healthimo_asthma_message_log_get_default($data->user->uid, 4, $data->number, $data->message, 0, 0);
    $message_log->processed = time();
    $message_log->response_expected = 0;
    $message_log->outbound = 0;
    $message_log = healthimo_asthma_message_log_save($message_log);
    healthimo_process_reply($data, "Reply with the times of day you take your controller medicine (examples: 7am 8pm OR 8:15am 6:30pm)");
}

function healthimo_asthma_process_request_education_time($symbol, &$data) {
    message("healthimo_asthma_process_request_education_time");
    $message_log = healthimo_asthma_message_log_get_default($data->user->uid, 4, $data->number, $data->message, 0, 0);
    $message_log->processed = time();
    $message_log->response_expected = 0;
    $message_log->outbound = 0;
    $message_log = healthimo_asthma_message_log_save($message_log);
    healthimo_process_reply($data, "Reply with time of day you would like your asthma education (examples: 7am OR 8:15am)");
}

function healthimo_asthma_process_receive_education_time($symbol, &$data) {
    message("healthimo_asthma_process_receive_education_time");
    $message_log = healthimo_asthma_message_log_get_default($data->user->uid, 4, $data->number, $data->message, 0, 0);
    $message_log->processed = time();
    $message_log->response_expected = 0;
    $message_log->outbound = 0;
    $message_log = healthimo_asthma_message_log_save($message_log);
    $date_parts = HealthimoProcess::parseTimesOfDay($data->message);
    $time1 = healthimo_asthma_process_formatted_time($date_parts, 0);
    healthimo_profile_save($data->user, 'profile_asthma_feature_controller_reminder_am_time', $time1, PROFILE_CATEGORY_ASTHMA);
    healthimo_profile_save($data->user, 'profile_asthma_feature_controller_reminder_pm_time', '', PROFILE_CATEGORY_ASTHMA);
    return healthimo_asthma_process_profile_complete($symbol, $data);
}

function healthimo_asthma_process_formatted_time($date_parts, $i) {
    $time1 = $date_parts[0][$i];
    $time1_hour = $date_parts[1][$i];
    $time1_min = $date_parts[2][$i];
    $time1_ampm = $date_parts[3][$i];
    $time1_isAM = strcmp('am', $time1_ampm) == 0;
    $time1_isPM = strcmp('pm', $time1_ampm) == 0;
    if ($time1_isPM && $time1_hour < 12) {
        $hour = $time1_hour + 12;
    } else {
        $hour = $time1_hour;
    }
    return sprintf("%02d:%02d", $hour, $time1_min);
}

function healthimo_asthma_process_receive_controller_times($symbol, &$data) {
    message("healthimo_asthma_process_receive_controller_times " . $data->user->uid);
    $message_log = healthimo_asthma_message_log_get_default($data->user->uid, 4, $data->number, $data->message, 0, 0);
    $message_log->processed = time();
    $message_log->response_expected = 0;
    $message_log->outbound = 0;
    $message_log = healthimo_asthma_message_log_save($message_log);

    $date_parts = HealthimoProcess::parseTimesOfDay($data->message);
    $time1 = healthimo_asthma_process_formatted_time($date_parts, 0);
    $time2 = healthimo_asthma_process_formatted_time($date_parts, 1);
    healthimo_profile_save($data->user, 'profile_asthma_feature_controller_reminder_am_time', $time1, PROFILE_CATEGORY_ASTHMA);
    healthimo_profile_save($data->user, 'profile_asthma_feature_controller_reminder_pm_time', $time2, PROFILE_CATEGORY_ASTHMA);
    return healthimo_asthma_process_profile_complete($symbol, $data);
}

function healthimo_asthma_process_profile_complete($symbol, &$data) {
    message("healthimo_asthma_process_profile_complete " . $data->user->uid);
    // Enrolled in healthimo Asthma Program
    healthimo_profile_save($data->user, 'profile_asthma_enrolled', 1, PROFILE_CATEGORY_ASTHMA);
    healthimo_profile_save($data->user, 'profile_asthma_feature_controller_reminder_active', 1, PROFILE_CATEGORY_ASTHMA);
    healthimo_profile_save($data->user, 'profile_has_asthma_in_household', 1, PROFILE_CATEGORY_ASTHMA);
    healthimo_process_send_message($data, "Your Healthy Families asthma program profile is complete. To update anytime, text HEALTHY ASTHMA to 3617920457");
    return HealthimoProcess::ACTIVE;
}

function healthimo_asthma_process_active_message($symbol, &$data) {
    message("healthimo_asthma_process_active_message");
    //    // handled in controller.inc
    //    // handled in controller.inc
    //    // handled in controller.inc
    //    // handled in controller.inc
    //    // handled in controller.inc
    //    // handled in controller.inc
    //    //
    //    ///
    //    message("find asthma message per algorithm");
    //    healthimo_process_send_message($data, "Send asthma message per existing algorithm");
    //    $outOfMessages = TRUE;
    //    if ($outOfMessages) {
    ////        return RAN_OUT_OF_MESSAGES;
    //        $user->profile_asthma_feature_controller_reminder_active == 0;
    //        return HealthimoProcess::PROTOCOL_COMPLETE;
    //    }
}

function healthimo_asthma_process_active_message_input($symbol, &$data) {
    message("healthimo_asthma_process_active_message_input");

    $user = $data->user;
    $message = $data->message;

    message("active message");
    healthimo_asthma_log("inbound message");
    $message_log_last_request = healthimo_asthma_message_log_load_most_recent_request($user->uid);
//    if ($message_log_last_request
//            &&
//            MESSAGE_TYPE_CONTROLLER_REMINDER == $message_log_last_request->message_type
//            &&
//            ( REQUEST_TYPE_ZONE == $message_log_last_request->response_type_expected
//            || REQUEST_TYPE_CONTROLLER == $message_log_last_request->response_type_expected )
//    ) {
//        // get reply, and mark response as received if reply is a valid response
//        $is_valid_response = FALSE;
//        if (REQUEST_TYPE_ZONE == $message_log_last_request->response_type_expected) {
//            $is_valid_response = is_valid_healthimo_asthma_handle_zone_response($message);
//            //reply based on content
//            $reply = healthimo_asthma_handle_zone_response($message);
//        } else if (REQUEST_TYPE_CONTROLLER == $message_log_last_request->response_type_expected) {
//            //log it
//            $is_valid_response = TRUE;
//        }
//        if ($is_valid_response) {
//            $message_log_last_request->response_received = time();
//            if (healthimo_asthma_message_log_save($message_log_last_request)) {
//                healthimo_asthma_message_log_mark_all_pending_requests_received($user->uid);
//            }
//        }
//    }


    if ($message_log_last_request) {
        // save raw message inbound, may save again later more correlated/parsed etc...
        $message_log = healthimo_asthma_message_log_get_default($user->uid, 4, $data->number, $message, 0, 0);
        $message_log->processed = time();
        $message_log->response_expected = 0;
        $message_log->outbound = 0;
        $message_log = healthimo_asthma_message_log_save($message_log);

        // request specific
        $message_log_last_request->response_received = time();
        if (healthimo_asthma_message_log_save($message_log_last_request)) {
            healthimo_asthma_message_log_mark_all_pending_requests_received($user->uid);
        }
        //no more zone replies, for oat, ask another question instead. 
        //healthimo_process_reply($data, $reply);
        // don't ask here, 
        // will flow through to hfstx module next.
        // if this was an fstx question, not a zone question, then will trigger further questions
        // hfstx_ask_question($user, $number);
    }
}

function healthimo_asthma_process_survey($symbol, &$data) {
    message("healthimo_asthma_process_survey");

    // no longer active controller reminders
    //    $user->profile_asthma_feature_controller_reminder_active == 0;        

    message("Survey process (sub process)");
    healthimo_process_reply($data, "Thank you for participating in the healthimo asthma program. Please reply with any comments you have.");
}

function healthimo_asthma_process_end($symbol, &$data) {
    message("healthimo_asthma_process_end");
    message("healthimo_asthma_process_end survey[" . $data->message . "]");
}

/////////////////// helpers
function is_valid_healthimo_asthma_handle_zone_response($message) {
    message("is_valid_healthimo_asthma_handle_zone_response");

    $stripThese = array("\r", "\n");
    $cleanedMessage = strtolower(str_replace($stripThese, "\t", $message));

    if (strpos($cleanedMessage, "green") !== false) {
        return TRUE;
    } else if (strpos($cleanedMessage, "yellow") !== false) {
        return TRUE;
    } else if (strpos($cleanedMessage, "red") !== false) {
        return TRUE;
    }

    return FALSE;
}

/**
 *
 * @param <type> $message
 * @return <type>         Once per week a message asking the patient what zone they are in
  3.4.1.3     How are you feeling, what zone are you in (green, yellow or red)?
  • If they reply with 'green', they will get the following response:
  ◦ Follow your action plan. In case of medical emergency, call 911.
  • If they reply with 'yellow', they will get the following response:
  ◦ Follow your action plan, and if you don't improve, call your Dr. In case of medical emergency, call 911.
  • If they reply with 'red', they will get the following response:
  ◦ Follow your action plan, and if you don't improve, call your Dr. or go to the ER. In case of medical emergency, call 911.
  • If they reply with anything else, they will get the following response:
  ◦ Please reply with green, yellow or red for zone. In case of medical emergency, call 911.
  3.4.1.4     The following default message will be sent for any unexpected or
  uninterpretable inbound message:
  • In case of medical emergency, call 911.
 */
function healthimo_asthma_handle_zone_response($message) {
    message("healthimo_asthma_handle_zone_response");

    $stripThese = array("\r", "\n");
    $cleanedMessage = strtolower(str_replace($stripThese, "\t", $message));

    if (strpos($cleanedMessage, "green") !== false) {
        return "Follow your action plan. In case of medical emergency, call 911.";
    } else if (strpos($cleanedMessage, "yellow") !== false) {
        return "Follow your action plan, and if you don't improve, call your Dr. In case of medical emergency, call 911.";
    } else if (strpos($cleanedMessage, "red") !== false) {
        return "Follow your action plan, and if you don't improve, call your Dr. or go to the ER. In case of medical emergency, call 911.";
    }

    return "Please reply with green, yellow or red for zone. In case of medical emergency, call 911.";
}

?>
