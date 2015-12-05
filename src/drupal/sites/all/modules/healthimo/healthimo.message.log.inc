<?php

/**
 * 
 * proper way to fetch groups of objects from db into my object
 * 
 * 
 * @param <type> $uid
 * @param <type> $program_tid
 * @param <type> $address
 * @param <type> $message
 * @param <type> $message_nid
 * @param <type> $outbound (1=out, 0=in)
 * @param <type> $response_expected
 * @param <type> $response_type_expected

class HealthimoMessageLog {

    public $created;
    public $uid;
    public $program_tid;
    public $address;
    public $message;
    public $message_nid;
    public $message_type;
    public $outbound;
    public $response_expected;
    public $response_type_expected;
    public $response_received;
    public $mid;
    public $updated;

    public static function fromStdClass($stdClass) {
        return new HealthimoMessageLog(
                $stdClass->uid,
                $stdClass->program_tid,
                $stdClass->address,
                $stdClass->message,
                $stdClass->message_nid,
                $stdClass->outbound,
                $stdClass->response_expected,
                $stdClass->response_type_expected
        );
    }

    public function __construct() {
        echo "HealthimoMessageLog()\n";
    }

    public function __construct(
    $uid, $program_tid, $address, $message, $message_nid, $outbound = 1, $response_expected = 1, $response_type_expected = null
    ) {
        echo "HealthimoMessageLog\n";
        $this->created = time();
        $this->uid = $uid;
        $this->program_tid = $program_tid;
        $this->address = $address;
        $this->message = $message;
        $this->message_nid = $message_nid;
        $this->message_type = null;
        $this->outbound = $outbound;
        $this->response_expected = $response_expected;
        $this->response_type_expected = $response_type_expected;
        $this->response_received = null;
        $this->mid = 0;
        $this->updated = 0;
    }

    public function save() {
        healthimo_log(' healthimo_message_log_save');

        $message_log->updated = time();

        if ($message_log->mid > 0) {
            drupal_write_record('healthimo_message_log', $this, 'mid');
        } else {
            drupal_write_record('healthimo_message_log', $this);
            $message_log->mid = db_last_insert_id('healthimo_message_log', 'mid');
        }

        return $message_log;
    }

    public function load($message_log) {
        healthimo_log(' healthimo_message_log_load');

        if (is_object($message_log)) {
            $mid = $message_log->mid;
        } else {
            $mid = $message_log;
        }

        $result = db_query(
                "
        SELECT *
        FROM {healthimo_message_log}
        WHERE
        mid = %d
        ", $mid
        );
        return db_fetch_object($result);
    }

    public function load_most_recent_by_type($uid, $type) {
        //too noisy healthimo_log(' healthimo_message_log_load_most_recent_sent');
        $result = db_query_range(
                "
        SELECT *
        FROM {healthimo_message_log} ml
        WHERE
        ml.uid = %d
        AND
        ml.message_type = '%s'
        AND
        ml.created = (
            SELECT MAX(ml2.created)
            FROM {healthimo_message_log} ml2
            WHERE
                ml2.uid = ml.uid
                and
                ml2.message_type = ml.message_type
        )
        ", array($uid, $type), 0, 1
        );
        return $result ? db_fetch_object($result) : FALSE;
    }

    public function load_last_n_by_type($uid, $count, $type) {
        //too noisy healthimo_log(' healthimo_message_log_load_most_recent_sent');
        return db_query_range(
                "
        SELECT *
        FROM {healthimo_message_log} ml
        WHERE
        ml.uid = %d
        AND
        ml.message_type = '%s'
        order by ml.created desc
        ", array($uid, $type), 0, $count
        );
    }

    public function load_last_n_by_request_type($uid, $count, $type) {
        //too noisy healthimo_log(' healthimo_message_log_load_most_recent_sent');
        return db_query_range(
                "
        SELECT *
        FROM {healthimo_message_log} ml
        WHERE
        ml.uid = %d
        AND
        ml.response_type_expected = '%s'
        order by ml.created desc
        ", array($uid, $type), 0, $count
        );
    }

    public function load_most_recent_sent($uid) {
        //too noisy healthimo_log(' healthimo_message_log_load_most_recent_sent');
        $result = db_query(
                "
        SELECT *
        FROM {healthimo_message_log} ml
        WHERE
        ml.uid = %d
        AND
        ml.created = (
            SELECT MAX(ml2.created)
            FROM {healthimo_message_log} ml2
            WHERE
                ml2.uid = ml.uid
                and
                ml2.outbound = 1
        )
        ", $uid
        );
        return db_fetch_object($result);
    }

    public function load_most_recent_sent_education($uid) {
        healthimo_log(' healthimo_message_log_load_most_recent_sent');
        $result = db_query(
                "
        SELECT *
        FROM {healthimo_message_log} ml
        WHERE
        ml.uid = %d
        AND
        ml.created = (
            SELECT MAX(ml2.created)
            FROM {healthimo_message_log} ml2
            WHERE
                ml2.uid = ml.uid
                and
                ml2.outbound = 1
                and
                ml2.response_expected = 0
        )
        ", $uid
        );
        return db_fetch_object($result);
    }

    public function count_sent($uid) {
        healthimo_log(' healthimo_message_log_count_sent');
        $result = db_query(
                "
        SELECT count(mid) as 'count'
        FROM {healthimo_message_log} ml
        WHERE
        ml.uid = %d
        AND
        ml.outbound = 1
        ", $uid
        );

        $result = db_fetch_object($result);
        return $result->count;
    }

    public function count_received($uid) {
        healthimo_log(' healthimo_message_log_count_received');
        $result = db_query(
                "
        SELECT count(mid) as 'count'
        FROM {healthimo_message_log} ml
        WHERE
        ml.uid = %d
        AND
        ml.outbound = 0
        ", $uid
        );

        $result = db_fetch_object($result);
        return $result->count;
    }

    public function load_most_recent_request($uid) {
        healthimo_log(' healthimo_message_log_load_most_recent_request');
        $result = db_query_range(
                "
        SELECT *
        FROM {healthimo_message_log} ml
        WHERE
        ml.uid = %d
        AND
        ml.outbound = 1
        AND
        ml.response_expected = 1
        AND
        ml.response_received is null
        AND
        ml.created = (
            SELECT MAX(ml2.created)
            FROM {healthimo_message_log} ml2
            WHERE
                ml2.uid = ml.uid
                AND
                ml2.outbound = 1
                AND
                ml2.response_expected = 1
                AND
                ml2.response_received is null
        )
        ", array($uid), 0, 1
        );
        return $result ? db_fetch_object($result) : FALSE;
    }

    public function mark_all_pending_requests_received($uid) {
        healthimo_log(' healthimo_message_log_mark_all_pending_requests_received');
        $result = db_query(
                "
        UPDATE
        {healthimo_message_log}
        SET response_received = %d
        WHERE
        uid = %d
        AND
        outbound = 1
        AND
        response_expected = 1
        AND
        response_received is null
        ", time(), $uid
        );
        return $result;
    }

}
 * 
 *  
 */