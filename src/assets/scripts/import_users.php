<?php

////////////////////////////////////////////////
bootstrap();
msg('import_users.php');
////////////////////////////////////////////////
//clear_watchdog();
msg("bootstrap completed");
////////////////////////////////////////////////
//process_rows_import_user_asthma_dchp_file_01();
//show_watchdog();

function process_rows_import_user_asthma_dchp_file_01() {
    msg('process_rows_import_user_asthma_dchp_file_01');

    /**
     * make any fields i'm using updateable 
     * then scrub/clean/validate them and then use as business keys
     */
    $result = db_query(
            //"SELECT * from import_user_asthma_dchp_file_01 limit 15"
            "SELECT * from import_user_asthma_dchp_file_01"
            //skips phone processing be careful w/ these where recordId not in (select rid from import_user_log where log_type = 'import')"
    );

    $i = 0;
    while ($row = db_fetch_object($result)) {
        /////////////////////////////////////////////////
        $i++;
        if ($i % 100 == 0) {
            echo( "Processing row $i\n");
        }
        /////////////////////////////////////////////////
        //
        //
        /////////////////////////////////////////////////
        //// Phone Num Edits ////////////////////////////
        /////////////////////////////////////////////////
        //show_phone_nums($row);
        //fix_phone_nulls($row);
        //fix_phone_non_digit($row);
        //show_phone_nums($row);
        //
        //
        /////////////////////////////////////////////////
//        import_user($row);
//        find_primary_cell($row);
        /////////////////////////////////////////////////
    }
    msg("Processed $i rows");
}

function test_user_create(&$row) {
    $user = create_new_account_for_handle($row->caseId);
    if ($user) {
        msg("create user for $row->caseId");
        print_r($user);
    } else {
        $user = user_load(array("name" => $row->caseId));
        print_r($user);
    }
}

function find_primary_cell(&$import_record) {

    // already processed? 
    if (import_user_log_get_by_rid_type($import_record->recordId, 'id-primary-cell')) {
        msg("find_primary_cell already processed rid $import_record->recordId");
        return;
    }

    // existing sms_user?
    //   mark as processed
    if ($row = import_user_log_get_by_rid_type_value($import_record->recordId, 'import', 'existing')) {
        msg("find_primary_cell existing sms_user for rid $import_record->recordId");
        $sms_user = db_fetch_object(db_query(
                        "select * from sms_user where uid = %d limit 1", $row->uid
                ));

        $log = import_user_log_get_default(
                $row->uid, $import_record->recordId, 'id-primary-cell', $sms_user->number
        );
        import_user_log_insert($log);
        return;
    }

    //  has a cell phone?
    //    create sms_user record
    //    mark as processed
    $row = import_user_log_get_by_rid_type_value($import_record->recordId, 'import', 'new');
    $uniq_phone = get_unique_phones($import_record);
    foreach ($uniq_phone as $phone) {
        // else lookup by api and save
        $url = 'http://digits.cloudvox.com/' . $phone . '.json';
        $ch = curl_init($url);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
        $data = curl_exec($ch);
        $json = json_decode($data, false);
        foreach ($json as $cloudvox_info) {
            if ($cloudvox_info->access_type == "wireless") {
                msg("find_primary_cell wireless found $row->uid, $phone");
                $result = db_query(
                        "
                        insert into sms_user 
                        (uid,number, status) values (%d, '%s', %d)
                        ", $row->uid, $phone, 2
                );
                $log = import_user_log_get_default(
                        $row->uid, $import_record->recordId, 'id-primary-cell', $phone);
                import_user_log_insert($log);
                // only take the primary, we're done
                return;
            }
        }
    }
}

function get_unique_phones(&$row) {
    $phones = array();
    if (!empty($row->phone)) {
        $phones[] = $row->phone;
    }
    if (!empty($row->phone1)) {
        $phones[] = $row->phone1;
    }
    if (!empty($row->phone2)) {
        $phones[] = $row->phone2;
    }
    if (!empty($row->phone3)) {
        $phones[] = $row->phone3;
    }
    return array_unique($phones);
}

function import_user(&$row) {
    //print_r($row);
    $uniq_phones = get_unique_phones($row);
    //print_r($uniq_phones);
    $existing_user = FALSE;
    foreach ($uniq_phones as $phone) {
        $phone_row = user_exists_by_phone($phone);
        if ($phone_row) {
            msg("import_user existing user for $phone_row->uid");
            $existing_user = TRUE;
            $type = 'import';
            $existing_log = import_user_log_get_by_rid_type($row->recordId, $type);
            if (!$existing_log) {
                $value = 'existing';
                $log = import_user_log_get_default($phone_row->uid, $row->recordId, $type, $value);
                import_user_log_insert($log);
            } else {
                msg("import_user existing log for $row->recordId, $phone_row->uid");
            }
            break; // just use the first found
        }
    }
    if (!$existing_user) {
        $type = 'import';
        $value = 'new';
        $existing_log = import_user_log_get_by_rid_type($row->recordId, $type);
        //print_r($existing_log);
        if (!$existing_log) {
            // create new user
            $user = create_new_account_for_handle($row->caseId);
            if ($user) {
                msg("import_user created user $user->uid");
                
                // link to import record
                $log = import_user_log_get_default($user->uid, $row->recordId, $type, $value);
                import_user_log_insert($log);
            }
        } else {
            msg("import_user skipping create, existing log for $row->recordId");
        }
    }
}

////////////////////////////////////////////////
//// Misc //////////////////////////////////////
////////////////////////////////////////////////
// existing users in users/sms_user
//select uid, phone from ( select uid, name as phone from users where length(name) = 10 and name REGEXP '^[0-9]+$' union distinct select uid, number as phone from sms_user where length(number) = 10 and number REGEXP '^[0-9]+$' ) x;


function bootstrap() {
    //bootstrap drupal. set remote address to quiet errors during bootstrap
    $_SERVER['REMOTE_ADDR'] = "127.1.1.1";
    require_once './includes/bootstrap.inc';
    drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
}

function msg($msg) {
    echo $msg . "\n";
    watchdog(WATCHDOG_DEBUG, $msg);
}

function clear_watchdog() {
    $result = db_query(
            "truncate table {watchdog}"
    );
}

function show_watchdog($limit = 20) {
    $result = db_query(
            "SELECT * from {watchdog} order by timestamp desc limit $limit"
    );

    while ($row = db_fetch_object($result)) {
        $variables = unserialize($row->variables);
        if (is_array($variables)) {
            $row->message = strtr($row->message, $variables);
        }
        unset($row->variables);
        echo $row->timestamp . ": " . $row->message . "\n";
    }
}

function test() {
    $result = db_query(
            "SELECT * from {users} limit 1"
    );

    while ($row = db_fetch_object($result)) {
        msg($row->uid . ' ' . $row->uid);
    }
}

function create_new_account_for_handle($handle) {
    // just a random sha hash including time so we can set the password
    // we don't know/don't care what it is
    // it's secure and will need to be reset by the user if they register via email later
    $token = base64_encode(hash_hmac('sha256', $handle, drupal_get_private_key() . time(), TRUE));
    $token = strtr($token, array('+' => '-', '/' => '_', '=' => ''));
    $result = db_fetch_object(db_query(
                    "SELECT max(uid)+1 as maxuid from {users}"
            ));
    $details = array(
        'name' => $handle . "-$result->maxuid",
        'pass' => $token,
        'mail' => $handle,
        'access' => 0,
        'status' => 1,
    );
    return user_save(null, $details);
}

function create_new_account_with_sms($number) {
    $sms_user[0] = array(
        status => 2,
        number => $number,
    );

    // just a random sha hash including time so we can set the password
    // we don't know/don't care what it is
    // it's secure and will need to be reset by the user if they register via email later
    $token = base64_encode(hash_hmac('sha256', $number, drupal_get_private_key() . time(), TRUE));
    $token = strtr($token, array('+' => '-', '/' => '_', '=' => ''));

    $details = array(
        'name' => $number,
        'pass' => $token,
        'mail' => $number,
        'access' => 0,
        'status' => 1,
        'sms_user' => $sms_user
    );
    return user_save(null, $details);
}

////////////////////////////////////////////////
//// exists by phone ///////////////////////////
////////////////////////////////////////////////
function user_exists_by_phone($phone) {
    if ($phone == 0) {
        return FALSE;
    }

//    select u.uid, u.name, su.number from users u left outer join sms_user su on su.uid = u.uid where u.name = '4692486914' or su.number = '4692486914' limit 1;
    $result = db_query(
            "
    select su.uid, su.number from sms_user su  where su.number = '%s' limit 1        
", $phone
    );
    return db_fetch_object($result);
}

////////////////////////////////////////////////
//// import_user_log db  ///////////////////////
////////////////////////////////////////////////
function import_user_log_get_default(
$uid, $rid, $type, $value
) {
    $log = new stdClass();
    $log->created = time();
    $log->uid = $uid;
    $log->rid = $rid;
    $log->import_table = 'import_user_asthma_dchp_file_01'; //$import_table;
    $log->type = $type;
    $log->value = $value;
    return $log;
}

/**
 * type = 'import', value = 'new', 'existing'
 * 
 * type = 'x', value = 'y', 'z'
 * 
 * @param type $log 
 */
function import_user_log_insert(&$log) {
    $result = db_query(
            "
            insert into import_user_log 
            (created, rid, uid, import_table, log_type, log_value)
            values
            (%d,%d,%d,'%s','%s','%s')
            "
            , $log->created, $log->rid, $log->uid, $log->import_table, $log->type, $log->value);
}

function import_user_log_get_by_rid_type($rid, $type) {
    $result = db_query(
            "
            select * from import_user_log 
            where 
            rid = %d
            and            
            log_type = '%s'
            "
            , $rid, $type);

    return db_fetch_object($result);
}

function import_user_log_get_by_rid_type_value($rid, $type, $value) {
    $result = db_query(
            "
            select * from import_user_log 
            where 
            rid = %d
            and            
            log_type = '%s'
            and
            log_value = '%s'
            "
            , $rid, $type, $value);

    return db_fetch_object($result);
}

function import_user_log_get_by_rid_uid_type($rid, $uid, $type) {
    $result = db_query(
            "
            select * from import_user_log 
            where 
            rid = %d
            and
            uid = %d
            and
            log_type = '%s'
            "
            , $rid, $uid, $type);

    return db_fetch_object($result);
}

function import_user_log_get_by_type_value($type, $value) {
    $result = db_query(
            "
            select * from import_user_log 
            where 
            log_type = '%s'
            and
            log_value = '%s'
            "
            , $type, $value);
    return $result;
}

function import_user_log_get_by_rid_uid_type_value($rid, $uid, $type, $value) {
    $result = db_query(
            "
            select * from import_user_log 
            where 
            rid = %d
            and
            uid = %d
            and
            log_type = '%s'
            and
            log_value = '%s'
            "
            , $rid, $uid, $type, $value);

    echo "rows = " . count($result) . "\n";

    while ($row = db_fetch_object($result)) {
        print_r($row);
    }
}

////////////////////////////////////////////////
//// DB Update /////////////////////////////////
////////////////////////////////////////////////

function update_import_user_asthma_dchp_file_01(&$row) {
//    msg('update_import_user_asthma_dchp_file_01');
//    print_r($row);
    return db_query("
            update import_user_asthma_dchp_file_01 
            set
            phone = '%s',
            phone1 = '%s',
            phone2 = '%s',
            phone3 = '%s'
            where recordId = %d
            ", $row->phone, $row->phone1, $row->phone2, $row->phone3, $row->recordId
    );
}

////////////////////////////////////////////////
//// PHONE NUMBERS /////////////////////////////
////////////////////////////////////////////////

function show_phone_nums(&$row) {
    msg(
            '[' .
            $row->recordId
            . ']['
            . $row->memberId
            . ']['
            . $row->phone
            . ']['
            . $row->phone1
            . ']['
            . $row->phone2
            . ']['
            . $row->phone3
            . ']'
    );
}

function edit_phone_number($phone) {
    $items = Array('/\!/', '/\'/', '/\#/', '/\ /', '/\+/', '/\-/', '/\./', '/\,/', '/\(/', '/\)/', '/[a-zA-Z]/');
    if (!empty($phone) && !ctype_digit($phone)) {
        $phone = preg_replace($items, '', $phone);
        $phone = str_replace('-', '', $phone);
        $phone = str_replace('.', '', $phone);
        if (strlen($phone) == 0) {
            $phone = NULL;
        } else if (strlen($phone) != 10) {
            msg("Bad phone [$phone]");
            $phone = NULL;
        }
        return $phone;
    }
}

function fix_phone_non_digit(&$row) {
    $update = false;
    if (!empty($row->phone) && !ctype_digit($row->phone)) {
        $update = true;
        $row->phone = edit_phone_number($row->phone);
        msg($row->recordId . '[' . $row->phone . ']' . strlen($row->phone));
    }
    if (!empty($row->phone1) && !ctype_digit($row->phone1)) {
        $update = true;
        $row->phone1 = edit_phone_number($row->phone1);
        msg($row->recordId . '[' . $row->phone1 . ']' . strlen($row->phone1));
    }
    if (!empty($row->phone2) && !ctype_digit($row->phone2)) {
        $update = true;
        $row->phone2 = edit_phone_number($row->phone2);
        msg($row->recordId . '[' . $row->phone2 . ']' . strlen($row->phone2));
    }
    if (!empty($row->phone3) && !ctype_digit($row->phone3)) {
        $update = true;
        $row->phone3 = edit_phone_number($row->phone3);
        msg($row->recordId . '[' . $row->phone3 . ']' . strlen($row->phone3));
    }
    if ($update) {
        update_import_user_asthma_dchp_file_01($row);
    }
}

function fix_phone_nulls(&$row) {

    if (strcmp('NULL', $row->phone) == 0) {
        $row->phone = NULL;
    }
    if (strcmp('NULL', $row->phone1) == 0) {
        $row->phone1 = NULL;
    }
    if (strcmp('NULL', $row->phone2) == 0) {
        $row->phone2 = NULL;
    }

    if (strcmp('NULL', $row->phone3) == 0) {
        $row->phone3 = NULL;
    } else if (strcmp('Eric', $row->phone3) == 0) {
        $row->phone3 = NULL;
    }

    update_import_user_asthma_dchp_file_01($row);
}

?>
