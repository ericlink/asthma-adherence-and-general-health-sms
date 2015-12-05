<?php

define('TABLE_MESSAGE_LOG', 'hfstx_message_log');
define('TABLE_MESSAGE_LOG_KEY', 'mid');

/**
 *
 * @param <type> $uid
 * @param <type> $program_tid
 * @param <type> $address
 * @param <type> $message
 * @param <type> $message_nid
 * @param <type> $outbound (1=out, 0=in)
 * @param <type> $response_expected
 * @param <type> $response_type_expected
 */
function hfstx_message_log_get_default(
        $uid,
        $program_tid,
        $address,
        $message,
        $message_nid,
        $outbound = 1,
        $response_expected = 1,
        $response_type_expected = null
) {
    $message_log = new stdClass();
    $message_log->created = time();
    $message_log->uid = $uid;
    $message_log->program_tid = $program_tid;
    $message_log->address = $address;
    $message_log->message = $message;
    $message_log->message_nid = $message_nid;
    $message_log->outbound = $outbound;
    $message_log->response_expected = $response_expected;
    $message_log->response_type_expected = $response_type_expected;
    $message_log->response_received = null;
    $message_log->mid = 0;
    $message_log->updated = 0;
    return $message_log;
}

function hfstx_message_log_save($message_log) {
    healthimo_log(' hfstx_message_log_save');

    $message_log->updated = time();

    if ($message_log->mid > 0) {
        drupal_write_record(TABLE_MESSAGE_LOG, $message_log, TABLE_MESSAGE_LOG_KEY );
    } else {
        drupal_write_record(TABLE_MESSAGE_LOG, $message_log);
        $message_log->mid = db_last_insert_id(TABLE_MESSAGE_LOG, TABLE_MESSAGE_LOG_KEY);
    }

    return $message_log;
}

function hfstx_message_log_load($message_log) {
    healthimo_log(' hfstx_message_log_load');

    if (is_object($message_log)) {
        $mid = $message_log->mid;
    } else {
        $mid = $message_log;
    }

    $result = db_query(
            "
        SELECT *
        FROM {" .TABLE_MESSAGE_LOG ."}
        WHERE
        mid = %d
        ",
            $mid
    );
    return db_fetch_object($result);
}

function hfstx_message_log_load_most_recent_sent($uid) {
    //too noisy healthimo_log(' hfstx_message_log_load_most_recent_sent');
    $result = db_query(
            "
        SELECT *
        FROM {" .TABLE_MESSAGE_LOG ."} hfml
        WHERE
        hfml.uid = %d
        AND
        hfml.created = (
            SELECT MAX(hfml2.created)
            FROM {" .TABLE_MESSAGE_LOG ."} hfml2
            WHERE
                hfml2.uid = hfml.uid
                and
                hfml2.outbound = 1
        )
        ",
            $uid
    );
    return db_fetch_object($result);
}

function hfstx_message_log_load_most_recent_sent_education($uid) {
    healthimo_log(' hfstx_message_log_load_most_recent_sent');
    $result = db_query(
            "
        SELECT *
        FROM {" .TABLE_MESSAGE_LOG ."} hfml
        WHERE
        hfml.uid = %d
        AND
        hfml.created = (
            SELECT MAX(hfml2.created)
            FROM {" .TABLE_MESSAGE_LOG ."} hfml2
            WHERE
                hfml2.uid = hfml.uid
                and
                hfml2.outbound = 1
                and
                hfml2.response_expected = 0
        )
        ",
        $uid
    );
    return db_fetch_object($result);
}

function hfstx_message_log_count_sent($uid) {
    healthimo_log(' hfstx_message_log_count_sent');
    $result = db_query(
            "
        SELECT count(mid) as 'count'
        FROM {" .TABLE_MESSAGE_LOG ."} hfml
        WHERE
        hfml.uid = %d
        AND
        hfml.outbound = 1
        ",
            $uid
    );

    $result = db_fetch_object($result);
    return $result->count;
}

function hfstx_message_log_count_received($uid) {
    healthimo_log(' hfstx_message_log_count_received');
    $result = db_query(
            "
        SELECT count(mid) as 'count'
        FROM {" .TABLE_MESSAGE_LOG ."} hfml
        WHERE
        hfml.uid = %d
        AND
        hfml.outbound = 0
        ",
            $uid
    );

    $result = db_fetch_object($result);
    return $result->count;
}

function hfstx_message_log_load_most_recent_request($uid) {
    healthimo_log(' hfstx_message_log_load_most_recent_request');
    $result = db_query_range(
            "
        SELECT *
        FROM {" .TABLE_MESSAGE_LOG ."} hfml
        WHERE
        hfml.uid = %d
        AND
        hfml.outbound = 1
        AND
        hfml.response_expected = 1
        AND
        hfml.response_received is null
        AND
        hfml.created = (
            SELECT MAX(hfml2.created)
            FROM {" .TABLE_MESSAGE_LOG ."} hfml2
            WHERE
                hfml2.uid = hfml.uid
                AND
                hfml2.outbound = 1
                AND
                hfml2.response_expected = 1
                AND
                hfml2.response_received is null
        )
        ",
            array($uid),
            0,
            1
    );
    return $result ? db_fetch_object($result) : FALSE;
}

function hfstx_message_log_mark_all_pending_requests_received($uid) {
    healthimo_log(' hfstx_message_log_mark_all_pending_requests_received');
    $result = db_query(
            "
        UPDATE
        {" .TABLE_MESSAGE_LOG ."}
        SET response_received = %d
        WHERE
        uid = %d
        AND
        outbound = 1
        AND
        response_expected = 1
        AND
        response_received is null
        ",
         time(), $uid
    );
    return $result;
}
