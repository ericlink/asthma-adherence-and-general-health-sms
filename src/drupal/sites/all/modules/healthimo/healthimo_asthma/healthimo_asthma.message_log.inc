<?php

/**
 * @param <type> $uid
 * @param <type> $program_tid
 * @param <type> $address
 * @param <type> $message
 * @param <type> $message_nid
 * @param <type> $outbound (1=out, 0=in)
 * @param <type> $response_expected
 * @param <type> $response_type_expected
 */
function healthimo_asthma_message_log_get_default(
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
    $message_log->message_type = null;
    $message_log->outbound = $outbound;
    $message_log->response_expected = $response_expected;
    $message_log->response_type_expected = $response_type_expected;
    $message_log->response_received = null;
    $message_log->mid = 0;
    $message_log->updated = 0;
    return $message_log;
}

function healthimo_asthma_message_log_save($message_log) {
    healthimo_log(' healthimo_asthma_message_log_save');

    $message_log->updated = time();

    if ($message_log->mid > 0) {
        drupal_write_record('healthimo_asthma_message_log', $message_log, 'mid' );
    } else {
        drupal_write_record('healthimo_asthma_message_log', $message_log);
        $message_log->mid = db_last_insert_id('healthimo_asthma_message_log', 'mid');
    }

    return $message_log;
}

function healthimo_asthma_message_log_load($message_log) {
    healthimo_log(' healthimo_asthma_message_log_load');

    if (is_object($message_log)) {
        $mid = $message_log->mid;
    } else {
        $mid = $message_log;
    }

    $result = db_query(
            "
        SELECT *
        FROM {healthimo_asthma_message_log}
        WHERE
        mid = %d
        ",
            $mid
    );
    return db_fetch_object($result);
}

function healthimo_asthma_message_log_load_most_recent_by_type($uid, $type) {
    //too noisy healthimo_log(' healthimo_asthma_message_log_load_most_recent_sent');
    $result = db_query_range(
            "
        SELECT *
        FROM {healthimo_asthma_message_log} ml
        WHERE
        ml.uid = %d
        AND
        ml.message_type = '%s'
        AND
        ml.created = (
            SELECT MAX(ml2.created)
            FROM {healthimo_asthma_message_log} ml2
            WHERE
                ml2.uid = ml.uid
                and
                ml2.message_type = ml.message_type
        )
        ",
            array($uid, $type),
            0,
            1
    );
    return $result ? db_fetch_object($result) : FALSE;
}

function healthimo_asthma_message_log_load_last_n_by_type($uid, $count, $type) {
    //too noisy healthimo_log(' healthimo_asthma_message_log_load_most_recent_sent');
    return db_query_range(
            "
        SELECT *
        FROM {healthimo_asthma_message_log} ml
        WHERE
        ml.uid = %d
        AND
        ml.message_type = '%s'
        order by ml.created desc
        ",
            array($uid, $type),
            0,
            $count
    );
}

function healthimo_asthma_message_log_load_last_n_by_request_type($uid, $count, $type) {
    //too noisy healthimo_log(' healthimo_asthma_message_log_load_most_recent_sent');
    return db_query_range(
            "
        SELECT *
        FROM {healthimo_asthma_message_log} ml
        WHERE
        ml.uid = %d
        AND
        ml.response_type_expected = '%s'
        order by ml.created desc
        ",
            array($uid, $type),
            0,
            $count
    );
}


function healthimo_asthma_message_log_load_most_recent_sent($uid) {
    //too noisy healthimo_log(' healthimo_asthma_message_log_load_most_recent_sent');
    $result = db_query(
            "
        SELECT *
        FROM {healthimo_asthma_message_log} ml
        WHERE
        ml.uid = %d
        AND
        ml.created = (
            SELECT MAX(ml2.created)
            FROM {healthimo_asthma_message_log} ml2
            WHERE
                ml2.uid = ml.uid
                and
                ml2.outbound = 1
        )
        ",
            $uid
    );
    return db_fetch_object($result);
}

function healthimo_asthma_message_log_load_most_recent_sent_education($uid) {
    healthimo_log(' healthimo_asthma_message_log_load_most_recent_sent');
    $result = db_query(
            "
        SELECT *
        FROM {healthimo_asthma_message_log} ml
        WHERE
        ml.uid = %d
        AND
        ml.created = (
            SELECT MAX(ml2.created)
            FROM {healthimo_asthma_message_log} ml2
            WHERE
                ml2.uid = ml.uid
                and
                ml2.outbound = 1
                and
                ml2.response_expected = 0
        )
        ",
            $uid
    );
    return db_fetch_object($result);
}

function healthimo_asthma_message_log_count_sent($uid) {
    healthimo_log(' healthimo_asthma_message_log_count_sent');
    $result = db_query(
            "
        SELECT count(mid) as 'count'
        FROM {healthimo_asthma_message_log} ml
        WHERE
        ml.uid = %d
        AND
        ml.outbound = 1
        ",
            $uid
    );

    $result = db_fetch_object($result);
    return $result->count;
}

function healthimo_asthma_message_log_count_received($uid) {
    healthimo_log(' healthimo_asthma_message_log_count_received');
    $result = db_query(
            "
        SELECT count(mid) as 'count'
        FROM {healthimo_asthma_message_log} ml
        WHERE
        ml.uid = %d
        AND
        ml.outbound = 0
        ",
            $uid
    );

    $result = db_fetch_object($result);
    return $result->count;
}

function healthimo_asthma_message_log_load_most_recent_request($uid) {
    healthimo_log(' healthimo_asthma_message_log_load_most_recent_request');
    $result = db_query_range(
            "
        SELECT *
        FROM {healthimo_asthma_message_log} ml
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
            FROM {healthimo_asthma_message_log} ml2
            WHERE
                ml2.uid = ml.uid
                AND
                ml2.outbound = 1
                AND
                ml2.response_expected = 1
                AND
                ml2.response_received is null
        )
        ",
            array($uid),
            0,
            1
    );
    return $result ? db_fetch_object($result) : FALSE;
}

function healthimo_asthma_message_log_mark_all_pending_requests_received($uid) {
    healthimo_log(' healthimo_asthma_message_log_mark_all_pending_requests_received');
    $result = db_query(
            "
        UPDATE
        {healthimo_asthma_message_log}
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
