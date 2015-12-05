<?php

define('START', 'Start');
define('ACTIVE', 'Active');
define('END', 'End');
define('INPUT_GENERAL_MESSAGE', 'Input (General Message)');
define('INPUT_YES', 'Input Yes');
define('INPUT_NO', 'Input No');
define('INPUT_ONE_VALID_TIME_OF_DAY', 'Input One Valid Time of Day');
define('INPUT_TWO_VALID_TIMES_OF_DAY', 'Input Two Valid Times of Day');
define('PROFILE_COMPLETE', 'Profile Complete');
define('WELCOME', 'Welcome');
define('SURVEY', 'Survey');
define('ACTIVE_MESSAGE', 'Active Message');
define('RAN_OUT_OF_MESSAGES', 'Ran Out of Messages');

///
define('INPUT_HEALTHY_ASTHMA', 'healthy asthma');
define('GET_CONTROLLER_STATUS', 'Get Controller Status');
define('CONFIGURE_CONTROLLER_TIMES', 'Configure Controller Times');
define('CONFIGURE_EDUCATION_TIME', 'Configure Education Time');

require 'FSM.php';
require_once 'FSM/GraphViz.php';

/*
  Entry action
  which is performed when entering the state
  Exit action
  which is performed when exiting the state
  Input action
  which is performed depending on present state and input conditions
  Transition action
  which is performed when performing a certain transition
 */

//////////////////////////////////////////////////////////////////////////////
// mainline
//////////////////////////////////////////////////////////////////////////////
echo "start\n";
//export_image(get_asthma_process(), 'asthma_process.svg');
//test_fsm();
$fsm = get_asthma_process(START, new stdClass());
$data = new stdClass();
asthma_process_test_with_controller($fsm, $data);
echo "end\n";

function get_asthma_process($current_state = START, &$data = null) {
    $process_name = 'healthimo_asthma_process';
    echo "$process_name\n";
    // process factory.. configure process, set start state and data pointer
    $fsm = new FSM($current_state, $data);
    $fsm->addTransition(INPUT_HEALTHY_ASTHMA, START, GET_CONTROLLER_STATUS, 'healthimo_asthma_process_get_controller_status');
    $fsm->addTransition(INPUT_YES, GET_CONTROLLER_STATUS, CONFIGURE_CONTROLLER_TIMES, 'healthimo_asthma_process_config_controller_times');
    $fsm->addTransition(INPUT_NO, GET_CONTROLLER_STATUS, CONFIGURE_EDUCATION_TIME, 'healthimo_asthma_process_config_education_time');
    $fsm->addTransition(INPUT_GENERAL_MESSAGE, GET_CONTROLLER_STATUS, GET_CONTROLLER_STATUS, 'healthimo_asthma_process_get_controller_status');
    //$fsm->addTransition(INPUT_TWO_VALID_TIMES_OF_DAY, GET_CONTROLLER_STATUS, GET_CONTROLLER_STATUS, 'healthimo_asthma_process_get_controller_status');
    //$fsm->addTransition(INPUT_ONE_VALID_TIME_OF_DAY, GET_CONTROLLER_STATUS, GET_CONTROLLER_STATUS, 'healthimo_asthma_process_get_controller_status');
    $fsm->addTransition(INPUT_TWO_VALID_TIMES_OF_DAY, CONFIGURE_CONTROLLER_TIMES, PROFILE_COMPLETE, 'healthimo_asthma_process_profile_complete');
    $fsm->addTransition(INPUT_ONE_VALID_TIME_OF_DAY, CONFIGURE_EDUCATION_TIME, PROFILE_COMPLETE, 'healthimo_asthma_process_profile_complete');
    $fsm->addTransition(INPUT_GENERAL_MESSAGE, CONFIGURE_CONTROLLER_TIMES, CONFIGURE_CONTROLLER_TIMES, 'healthimo_asthma_process_config_controller_times');
    $fsm->addTransition(INPUT_GENERAL_MESSAGE, CONFIGURE_EDUCATION_TIME, CONFIGURE_EDUCATION_TIME, 'healthimo_asthma_process_config_education_time');
    $fsm->addTransition(WELCOME, PROFILE_COMPLETE, ACTIVE, 'healthimo_asthma_process_active');
    $fsm->addTransition(ACTIVE_MESSAGE, ACTIVE, ACTIVE, 'healthimo_asthma_process_active_message');
    $fsm->addTransition(RAN_OUT_OF_MESSAGES, ACTIVE, SURVEY, 'healthimo_asthma_process_survey');
    $fsm->addTransition(INPUT_GENERAL_MESSAGE, SURVEY, END, 'healthimo_asthma_process_end');
    $fsm->addTransition(INPUT_GENERAL_MESSAGE, END, END, 'healthimo_asthma_process_end');
    //nop if don't register, otherwise transitions to this one specified here
    //$fsm->setDefaultTransition(END, 'healthimo_asthma_process_default_transition');
    return $fsm;
}

function asthma_process_test_with_controller(FSM &$fsm, &$data) {
    $data->val = 'init';
    state($fsm, $data);
    $fsm->process(INPUT_HEALTHY_ASTHMA);
    state($fsm, $data);
    $fsm->process(INPUT_YES);
    state($fsm, $data);
    $fsm->process(INPUT_UNKNOWN);
    state($fsm, $data);
    $fsm->process(INPUT_TWO_VALID_TIMES_OF_DAY);
    state($fsm, $data);
    // if current state is proflie complete then $fsm->process(WELCOME); (could just process)
    $fsm->process(WELCOME);
    state($fsm, $data);
    $fsm->process(INPUT);
    state($fsm, $data);
    $fsm->process(ACTIVE_MESSAGE);
    state($fsm, $data);
    $fsm->process(ACTIVE_MESSAGE);
    state($fsm, $data);
    $fsm->process(INPUT);
    state($fsm, $data);
    // if we are out of message then     $fsm->process(RAN_OUT_OF_MESSAGES);
    $fsm->process(RAN_OUT_OF_MESSAGES);
    state($fsm, $data);
    $fsm->process(INPUT);
    state($fsm, $data);
    $fsm->process(INPUT);
    state($fsm, $data);
    $fsm->process(INPUT);
    state($fsm, $data);
    $fsm->process(INPUT);
    state($fsm, $data);
    ///
//    $fsm->reset();
//    $fsm->process('INPUT_HEALTHY_ASTHMA');
//    $fsm->process('INPUT_NO');
//    $fsm->process('INPUT_UNKNOWN');
//    $fsm->process('INPUT_ONE_VALID_TIME');
//    $fsm->process('WELCOME');
    state($fsm, $data);
}

function healthimo_asthma_process_default_transition($symbol, &$data) {
    echo "healthimo_asthma_process_default_transition: $symbol\n";
}

function healthimo_asthma_process_get_controller_status($symbol, &$data) {
    healthimo_asthma_process_log($symbol, $data);
    echo "Do you take controller?";
}

function healthimo_asthma_process_active_message($symbol, &$data) {
    healthimo_asthma_process_log($symbol, $data);
}

function healthimo_asthma_process_config_controller_times($symbol, &$data) {
    healthimo_asthma_process_log($symbol, $data);
    echo "SAVE: controller times to profile";
}

function healthimo_asthma_process_config_education_time($symbol, &$data) {
    healthimo_asthma_process_log($symbol, $data);
    echo "SAVE: education time to profile";
}

function healthimo_asthma_process_profile_complete($symbol, &$data) {
    healthimo_asthma_process_log($symbol, $data);
    echo "Welcome to the asthma program your profile is complete";
}

function healthimo_asthma_process_active($symbol, &$data) {
    healthimo_asthma_process_log($symbol, $data);
    echo "Send asthma message per existing algorithm";
}

function healthimo_asthma_process_survey($symbol, &$data) {
    healthimo_asthma_process_log($symbol, $data);
    echo "Survey process (sub process)";
    echo "Did you like the asthma program?";
}

function healthimo_asthma_process_end($symbol, &$data) {
    healthimo_asthma_process_log($symbol, $data);
}

function healthimo_asthma_process_log($symbol, &$data) {
    echo "$symbol\n";
}

//////////////////////////////////////////////////////////////////////////////

function state(&$fsm, &$data) {

    echo "====STATE: $fsm->_currentState\n";
    print_r($data);
    echo "========================\n";
    $data->val = $fsm->getCurrentState();
}

function export_image($fsm, $filename) {
    echo "export_image\n";
    $outfile = fopen($filename, 'w');
    $converter = new FSM_GraphViz($fsm);
    $graph = $converter->export();
    $image = $graph->fetch();
    fwrite($outfile, $image);
    fclose($outfile);
}

//////////////////////////////////////////////////////////////////////////////
// end
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
function test_fsmErrorCallback($symbol, $payload) {
    echo "This symbol does not compute: $symbol $payload->x $payload->y $payload->z\n ";
}

function test_fsm() {
    $stack = new stdClass();
    $stack->x = 1;
    $stack->y = 1;
    $stack->z = 1;

    $fsm = new FSM('START', $stack);
    $fsm->addTransition('FIRST', 'START', 'MIDDLE', 'test_fsmFirstCallback');
    $fsm->addTransition('SECOND', 'MIDDLE', 'END', 'test_fsmSecondCallback');
    $fsm->setDefaultTransition('END', 'test_fsmErrorCallback');
    export_image($fsm, 'test_fsm.svg');


    print_r($stack);
    echo "$fsm->_currentState\n";

    $fsm->process('FIRST');
    print_r($stack);
    echo "$fsm->_currentState\n";
    $fsm->process('SECOND');
    print_r($stack);
    echo "$fsm->_currentState\n";
    echo "test default\n";
    $fsm->process('SECOND');
    print_r($stack);
    echo "$fsm->_currentState\n";
    $fsm->process('FIRST');
    print_r($stack);
    echo "$fsm->_currentState\n";
}

function test_fsmFirstCallback($symbol, $payload) {
    echo "test_fsmFirst Transition $payload->x $payload->y $payload->z\n ";
    $payload->x = 7;
}

function test_fsmSecondCallback($symbol, $payload) {
    echo "test_fsmSecond Transition $payload->x $payload->y $payload->z\n";
    $payload->x = 21;
    $payload->y = 22;
    $payload->z = 23;
}

//////////////////////////////////////////////////////////////////////////////
function bootstrap_drupal() {
//bootstrap drupal. set remote address to quiet errors during bootstrap
    $_SERVER['REMOTE_ADDR'] = "127.1.1.1";
    require_once './includes/bootstrap.inc';
    drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
}

function test_drupal_query() {
    $result = db_query(
            "SELECT * from zipnxx"
    );

    while ($row = db_fetch_object($result)) {
        echo $row->zip . ' ' . $row->nxx . "\n";
    }
}

?>
