<?php


class HealthimoInboundMessage {

    // message parts in normalized formats
    public $rawMessage;
    public $words;
    public $trimmed;
    
    // message concepts
    // these really come out of parser right?
    // /parser methods on here to do certain things for input
    // if isSet return else process and return
    public $timesOfDay;
    public $hasWordStop;
    
    public function __construct() {
        echo "HealthimoInboundMessage\n";
        //take a string and create all the parts
    }
    
    private function parseTimesOfDay() {
        
    }

}

?>
