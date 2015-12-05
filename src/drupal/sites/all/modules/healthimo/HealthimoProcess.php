<?php

class HealthimoProcess {
    const START = 'Start';
    const ACTIVE='Active';
    const END='End';
    const INPUT_GENERAL_MESSAGE='Input (General Message)';
    const INPUT_YES='Input Yes';
    const INPUT_NO='Input No';
    const INPUT_ONE_VALID_TIME_OF_DAY='Input One Valid Time of Day';
    const INPUT_TWO_VALID_TIMES_OF_DAY='Input Two Valid Times of Day';
    const PROFILE_COMPLETE='Profile Complete';
    const SURVEY='Survey';
    const MESSAGE_TRIGGER='Message Trigger';
    const PROTOCOL_COMPLETE='Protocol Complete';
///
    const INPUT_HEALTHY_ASTHMA='healthy asthma';
    const REQUEST_CONTROLLER_STATUS='Request Controller Status';
    const REQUEST_CONTROLLER_TIMES='Request Controller Times';
    const REQUEST_EDUCATION_TIME='Request Education Time';

    public function __construct() {
        echo "HealthimoProcess\n";
    }

    public static function parseInputType($msg) {
        //$msg is already trimmed and lower case from healthimo_sms_incoming
        $words = healthimo_string_get_words($msg);

        // right now processed by keyword processor, so real start state is request controller status
        // return HealthimoProcess::INPUT_HEALTHY_ASTHMA;
        // y/n?
        if (strcmp('yes', $words[0]) == 0) {
            return HealthimoProcess::INPUT_YES;
        } else if (strcmp('no', $words[0]) == 0) {
            return HealthimoProcess::INPUT_NO;
        } else if (strcmp('y', $words[0]) == 0) {
            return HealthimoProcess::INPUT_YES;
        } else if (strcmp('n', $words[0]) == 0) {
            return HealthimoProcess::INPUT_NO;
        }

        //TIMES?    
        $date_parts = HealthimoProcess::parseTimesOfDay($msg);
        if (HealthimoProcess::isTwoTimesOfDay($date_parts)) {
            return HealthimoProcess::INPUT_TWO_VALID_TIMES_OF_DAY;
        } else if (HealthimoProcess::isOneTimeOfDay($date_parts)) {
            return HealthimoProcess::INPUT_ONE_VALID_TIME_OF_DAY;
        }

        return HealthimoProcess::INPUT_GENERAL_MESSAGE;
    }

//        php > $date = '12 am 2:02 pm';;print_r($date_parts);
//		Array
//		(
//			[0] => Array
//				(
//					[0] => 12 am
//					[1] => 2:02 pm
//				)
//			[1] => Array
//				(
//					[0] => 12
//					[1] => 2
//				)
//			[2] => Array
//				(
//					[0] => 
//					[1] => 02
//				)
//			[3] => Array
//				(
//					[0] => am
//					[1] => pm
//				)
//		)
    //array(0,0), 0,0 are formatted dates.
    public static function parseTimesOfDay($msg) {
        preg_match_all('/(\d{1,2}):?(\d{2})?.*?(am|pm)/', $msg, $date_parts);
        return $date_parts;
    }

    private static function isTwoTimesOfDay(&$date_parts) {
        // are formatted date and hour available for two times?
        return
        !empty($date_parts[0][1])
        && !empty($date_parts[0][0])
        && !empty($date_parts[1][1])
        && !empty($date_parts[1][0])
        && HealthimoProcess::isValidHourOfDay($date_parts[1][1])
        && HealthimoProcess::isValidHourOfDay($date_parts[1][0])
        ;
    }

    private static function isOneTimeOfDay(&$date_parts) {
        // is formatted date and hour available for one time?
        return
        empty($date_parts[0][1])
        && empty($date_parts[1][1])
        && !empty($date_parts[0][0])
        && !empty($date_parts[1][0])
        && HealthimoProcess::isValidHourOfDay($date_parts[1][0])
        ;
    }

    private static function isValidHourOfDay($hour) {
        return is_numeric($hour) && $hour >= 0 && $hour < 13;
    }

}

?>
