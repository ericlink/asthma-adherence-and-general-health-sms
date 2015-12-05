<?php

/**
 * @return <type> all quicktip nodes for the given term ids, with custom fields included
 */
function healthimo_quicktip_load_by_taxonomy_terms($tid1, $tid2) {
    healthimo_log(' healthimo_quicktip_load_by_taxonomy_terms');

    return db_query(
            "
        SELECT c.*, n.* FROM
        content_type_quicktip c
        JOIN node n
            ON n.nid = c.nid
        JOIN term_node tn
            ON tn.nid = n.nid AND tn.vid = n.vid
        JOIN term_node tn2
            ON tn2.nid = n.nid AND tn2.vid = n.vid
        WHERE
            n.type = 'quicktip'
            AND
            tn.tid = %d
	    and
            tn2.tid = %d
	    and
	    tn.nid = tn2.nid
        ORDER BY n.nid
        ", $tid1, $tid2
    );
}

function healthimo_quicktip_load_next_in_series($tid, $lastOrdinal) {
    healthimo_log(' healthimo_quicktip_load_next_in_series tid ' .$tid .' lastOrdinal ' .$lastOrdinal);

    $result = db_query_range(
            "
        SELECT * FROM
        {content_type_quicktip} c
        JOIN {node} n
            ON n.nid = c.nid AND n.vid = c.vid
        JOIN {term_node} tn
            ON tn.nid = n.nid AND tn.vid = n.vid
        JOIN {term_data} td
            ON td.tid = tn.tid
        WHERE
            n.type = 'quicktip'
            AND
            td.tid = %d
            AND
            c.field_ordinal_value > %d
        ORDER BY c.field_ordinal_value ASC
        ",
            $tid,
            $lastOrdinal,
            0,
            1
    );
    return db_fetch_object($result);
}

function healthimo_quicktip_load($quicktip) {
    healthimo_log(' healthimo_quicktip_load');

    if (is_object($quicktip)) {
        $qid = $quicktip->qid;
    } else {
        $qid = $quicktip;
    }

    $result = db_query(
            "
        SELECT * FROM
        {content_type_quicktip} c
        JOIN {node} n
            ON n.nid = c.nid
            AND n.vid = c.vid
        WHERE
            n.type = 'quicktip'
            AND
            n.nid = %d
        ",
            $qid
    );
    return db_fetch_object($result);
}
