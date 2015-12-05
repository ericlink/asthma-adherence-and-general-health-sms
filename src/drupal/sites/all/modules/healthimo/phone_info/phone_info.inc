<?php

/**
 * Lookup the phone info, and if it's not found, add it to the list and process it
 * @return FALSE if can't get info, or info object
 */
function phone_info_get_info($phone) {
    //healthimo_log_debug('phone_info', 'phone %p', array('%p'=>$phone));
    $phone_input = $phone;
    $phone = phone_info_normalize_phone_number($phone);

    if (!$phone) {
        healthimo_log_alert('phone_info',
                'phone number failed to normalize [%s]',
                array('%s' => $phone_input)
        );
        return false;
    }

    $phone_info = phone_info_phone_number_load($phone);
    $phone_info = phone_info_create_phone_info($phone, $phone_info);
    $phone_info = phone_info_process($phone_info);

    // if processed ok set the info
    if ($phone_info && $phone_info->processed > 0) {
        if ($phone_info->is_wireless && $phone_info->gid > 0) {
            $obj = phone_info_sms_gateway_load($phone_info);
            if ($obj) {
                // all current gateways are 10@domain.com so this is ok for now
                $phone_info->smtp_address = $phone_info->phone . '@' . $obj->smtp_gateway_domain;
            } else {
                $phone_info->smtp_address = false;
            }
        } else {
            $phone_info->smtp_address = false;
        }
        return $phone_info;
    }

    return false;
}

function phone_info_create_phone_info($phone, $phone_info) {
    // create phone record if needed
    if (!$phone_info) {
        $phone_info = phone_info_phone_number_get_default($phone);
        $phone_info = phone_info_phone_number_save($phone_info);
    }

    return $phone_info;
}

function phone_info_process($phone_info) {
    //process, or reprocess
    if ($phone_info->processed == 0) {
        $cloudvox_info = phone_info_cloudvox_info_lookup($phone_info);
        if ($cloudvox_info) {
            $phone_info->is_wireless = (strcmp('wireless', $cloudvox_info->access_type) == 0);
            if ($phone_info->is_wireless) {
                $clubtexting_info = phone_info_clubtexting_info_lookup($phone_info);
                if ($clubtexting_info) {
                    $gateway_info = phone_info_sms_gateway_find_gid($phone_info);
                    if ($gateway_info) {
                        $phone_info->gid = $gateway_info->gid;
                    } else {
                        healthimo_log_alert('phone_info',
                                'new gateway [%g] for phone info id %s',
                                array('%s' => $phone_info->pid, '%g' => $clubtexting_info->carrier)
                        );
                        return false;
                    }
                    $phone_info->processed = time();
                    $phone_info = phone_info_phone_number_save($phone_info);
                    return $phone_info;
                } else {
                    healthimo_log_alert('phone_info',
                            'error looking up club texting for phone info [%s]',
                            array('%s' => $phone_info->pid)
                    );
                    return false;
                }
            } else {
                $phone_info->processed = time();
                $phone_info = phone_info_phone_number_save($phone_info);
                return $phone_info;
            }
        } else {
            healthimo_log_alert('phone_info',
                    'error looking up cloudvox for phone info [%s]',
                    array('%s' => $phone_info->pid)
            );
            return false;
        }
    }

    return $phone_info;
}

/**
 * Normalize to 10 Digit number (USA only for now)
 * @param <type> $phone (either 10 digit or country code + 10 digit)
 * @return normalized phone number or FALSE if can't be normalized
 */
function phone_info_normalize_phone_number($phone) {
    //usa only for now
    $phone = trim($phone);
    $len = strlen($phone);
    if ($len == 10) {
        return $phone;
    } else if ($len == 11) {
        // grab last 10
        return substr($phone, 1);
    } else {
        return false;
    }
}

function phone_info_phone_number_load($phone) {
    $result = db_query(
                    "
        SELECT *
        FROM {phone_info_phone_number}
        WHERE
        phone = %s
        ",
                    $phone
    );
    return db_fetch_object($result);
}

function phone_info_phone_number_get_default($phone) {
    $phone_info = new stdClass();
    $phone_info->pid = 0;
    $phone_info->created = time();
    $phone_info->updated = 0;
    $phone_info->processed = 0;
    $phone_info->phone = $phone;
    $phone_info->is_wirelesss = false;
    $phone_info->smtp_address = false;
    return $phone_info;
}

function phone_info_phone_number_save($phone_info) {
    healthimo_log(' phone_info_phone_number_save');
    $phone_info->updated = time();

    if ($phone_info->pid > 0) {
        drupal_write_record('phone_info_phone_number', $phone_info, 'pid');
    } else {
        drupal_write_record('phone_info_phone_number', $phone_info);
        $phone_info->mid = db_last_insert_id('phone_info_phone_number', 'pid');
    }

    return $phone_info;
}

function phone_info_cloudvox_info_lookup($phone_info) {
    $cloudvox_info = phone_info_cloudvox_info_load($phone_info);
    if ($cloudvox_info->pid > 0) {
        return $cloudvox_info;
    }

    // else lookup by api and save
    $url = 'http://digits.cloudvox.com/' . $phone_info->phone . '.json';
    $ch = curl_init($url);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
    $data = curl_exec($ch);
    $json = json_decode($data, false);
    foreach ($json as $cloudvox_info) {
        $cloudvox_info->pid = $phone_info->pid;
        phone_info_cloudvox_info_save($cloudvox_info);
        return $cloudvox_info;
    }

    healthimo_log_alert('phone_info',
            'did not find in cloudvox lookup [%s]',
            array('%s' => $phone_info->pid)
    );
}

function phone_info_cloudvox_info_load($phone_info) {
    $result = db_query(
                    "
        SELECT *
        FROM {phone_info_cloudvox_info}
        WHERE
        pid = %d
        ",
                    $phone_info->pid
    );
    return db_fetch_object($result);
}

function phone_info_cloudvox_info_save($cloudvox_info) {
    db_query("
        delete
        FROM {phone_info_cloudvox_info}
        WHERE
        pid = %d
        ",
            $cloudvox_info->pid);

    $cloudvox_info->created = time();
    $cloudvox_info->updated = time();

    drupal_write_record('phone_info_cloudvox_info', $cloudvox_info);
}

/**
  NEXTELUS
  SPRINTUS
  CINGULARUS
  BANDWIDTHCOMUS
 * @param <type> $phone_info
 * @return stdClass
 */
function phone_info_clubtexting_info_lookup($phone_info) {
    $clubtexting_info = phone_info_clubtexting_info_load($phone_info);
    if ($clubtexting_info->pid > 0) {
        return $clubtexting_info;
    }

    // else lookup by api and save
    $ch = curl_init('https://app.clubtexting.com/api/lookup/');
    curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
    curl_setopt($ch, CURLOPT_POST, 1);
    curl_setopt($ch, CURLOPT_POSTFIELDS, "user=healthimoca&pass=2arwAHBH99op&phonenumber=$phone_info->phone");
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
    $data = curl_exec($ch);
    if ($data < 0) {
        healthimo_log_alert('phone_info',
                'could not lookup info club texting rc %rc for pid %s',
                array('%s' => $phone_info->pid, '%rc' => $data)
        );
        return false;
    }


    $clubtexting_info = new stdClass();
    $clubtexting_info->pid = $phone_info->pid;
    $clubtexting_info->carrier = $data;
    phone_info_clubtexting_info_save($clubtexting_info);

    return $clubtexting_info;
}

function phone_info_clubtexting_info_load($phone_info) {
    $result = db_query(
                    "
        SELECT *
        FROM {phone_info_clubtexting_info}
        WHERE
        pid = %d
        ",
                    $phone_info->pid
    );
    return db_fetch_object($result);
}

function phone_info_clubtexting_info_save($clubtexting_info) {
    db_query("
        delete
        FROM {phone_info_clubtexting_info}
        WHERE
        pid = %d
        ",
            $clubtexting_info->pid);

    $clubtexting_info->created = time();
    $clubtexting_info->updated = time();

    drupal_write_record('phone_info_clubtexting_info', $clubtexting_info);
}

function phone_info_sms_gateway_find_gid($phone_info) {
    $result = db_query(
                    "
        select gt.gid
        from {phone_info_phone_number} pn
        join {phone_info_clubtexting_info} cti on cti.pid = pn.pid
        join {phone_info_sms_gateway_types} gt on gt.clubtexting_string = cti.carrier
        where pn.pid = %d
        ",
                    $phone_info->pid
    );
    return db_fetch_object($result);
}

function phone_info_sms_gateway_load($phone_info) {
    $result = db_query(
                    "
        select smtp_gateway_domain
        from {phone_info_sms_gateway_types} gt
        where gt.gid = %d
        ",
                    $phone_info->gid
    );
    return db_fetch_object($result);
}
