<?php

////////////////////////////////////////////////
bootstrap();
date_default_timezone_set('America/Chicago');
msg('import_users.php');
////////////////////////////////////////////////
//clear_watchdog();
msg("bootstrap completed");

////////////////////////////////////////////////
process_rows_import_user_rural();

//test_user_create();
//test_profile_fields();
//show_watchdog();

function process_rows_import_user_rural() {
    msg('process_rows_import_user_rural');

    $result = db_query(
            //"SELECT * from import_user_rural limit 43"
            "SELECT * from import_user_rural"
    );

    $i = 0;
    while ($row = db_fetch_object($result)) {
        /////////////////////////////////////////////////
        $i++;
        //if ($i % 10 == 0) {
            msg("Processing row $i rid ".$row->record_id );
        //}
        /////////////////////////////////////////////////
//        show_phone_nums($row);
//        show_phone_nums_valid($row);
        /////////////////////////////////////////////////
        $user = create_new_user($row);
        if ($user) {
            enroll_new_user($user, $row);
        }
        /////////////////////////////////////////////////
    }
    msg("Processed $i rows");
}

function create_new_user(&$row) {
    msg('create_new_user');

    if (!$row->is_cell_valid && !$row->is_email_valid) {
        msg("no email and no cell");
        $log = import_user_log_get_default($user->uid, $row->record_id, 'import', 'no-cell-or-email');
        import_user_log_insert($log);
        return false;
    }


    if (user_exists_by_phone($row)) {
        msg('user_exists_by_phone');
        return false;
    }
    if (user_exists_by_email($row)) {
        msg('user_exists_by_email');
        return false;
    }

    msg('creating');

    $log_type = 'import';
    $log_value = 'new';
    $existing_log = import_user_log_get_by_rid_type($row->record_id, $log_type);

    if ($existing_log) {
        msg('already have a log for this id,type ' . $row->record_id . ',' . $log_type);
        return false;
    }

    // cell
    $number = $row->cell;
    if (strlen($number) > 0) {
        $sms_user[0] = array(
            status => 2,
            number => $number
        );
    }

    // password 
    // just a random sha hash including time so we can set the password
    // we don't know/don't care what it is
    // it's secure and will need to be reset by the user if they register via email later
    $token = base64_encode(hash_hmac('sha256', $number, drupal_get_private_key() . time(), TRUE));
    $token = strtr($token, array('+' => '-', '/' => '_', '=' => ''));

    $details = array(
        'name' => strlen($row->handle) > 1 ? $row->handle . $row->record_id : $number,
        'pass' => $token,
        'mail' => $row->is_email_valid ? $row->email : $number,
        'access' => 0,
        'status' => 1,
        'sms_user' => $sms_user
    );

    $user = user_save(null, $details);
    // set values for the imported profile fields
    healthimo_profile_save($user, 'profile_age', $row->age, null);
    healthimo_profile_save($user, 'profile_zip_code', $row->zip, null);
    healthimo_profile_save($user, 'profile_gender', $row->gender, null);
    healthimo_profile_save($user, 'profile_goal', $row->goal, null);
    healthimo_profile_save($user, 'profile_areas_of_interest_reply', $row->interest_areas, null);
    healthimo_profile_save($user, 'profile_areas_of_interest_diabetes', $row->interest_diabetes, null);
    //healthimo_profile_save($user, 'xxxxxxxxxxx', $row->interest_asthma, null);

    if ($user) {
        msg("import_user created user $user->uid");

        // link to import record
        $log = import_user_log_get_default($user->uid, $row->record_id, $log_type, $log_value);
        import_user_log_insert($log);
        print_r($user);
        return $user;
    }

    return false;
}

function enroll_new_user($user, $row) {
    // must be type import/new
    if (!import_user_log_get_by_rid_type_value($row->record_id, 'import', 'new')) {
        msg("not import new rid $row->record_id");
        return;
    }


    if (!import_user_log_get_by_rid_type_value($row->record_id, 'welcome', 'messaged')) {
        // send via API
        // Welcome, you signed up for Healthy Families texts with a paper form (DCH-Dr. Ponder). 
        // Reply STOP to stop all messages.
        $message = "Welcome, you signed up for Healthy Families texts with a paper form (" .
                $row->referred_by
                . "). Reply STOP to stop all messages.";
        msg($message);
        if ($row->is_cell_valid) {
            healthimo_sms_outgoing($row->cell, $message, $options);
        }
        $log = import_user_log_get_default($user->uid, $row->record_id, 'welcome', 'messaged');
        import_user_log_insert($log);
    }
//    if ($row->is_email_valid) {
//        healthimo_email_outgoing($row->email, $message, $options);
//    }
    if (!import_user_log_get_by_rid_type_value($row->record_id, 'welcome', 'enrolled')) {
        msg("profile set flags");
        healthimo_profile_save($user, 'profile_keyword_healthy', 1, NULL);
        $log = import_user_log_get_default($user->uid, $row->record_id, 'welcome', 'enrolled');
        import_user_log_insert($log);
    }
}

//function test_profile_fields() {
//    $user = user_load(array("uid" => 1));
//    print_r($user);
//    $user->profile_age = 44;
//    $sms_user[0] = array(
//        status => 2,
//        number => 123,
//    );
//    $details = array(
//        'uid' => 1,
//        'name' => $number,
//        'pass' => $token,
//        'mail' => $number,
//        'access' => 0,
//        'status' => 1,
//        'sms_user' => $sms_user
//    );
//    user_save($user, $details);
//    print_r($user);
//    profile_load_profile($user);
//    // do same a question list of fields for merge
//    // list of interesting fields
//    $user->profile_age = 44;
//    healthimo_profile_save($user, 'profile_age', 44, null);
//    print_r($user);
//}
//function test_user_create(&$row) {
//    $handle = time();
//    $user = create_new_account_for_handle();
//    if ($user) {
//        msg("create user for $handle");
//        print_r($user);
//        $user->profile_age = 44;
//        user_save($user);
//        print_r($user);
//        $user = user_load(array("uid" => 1));
//        print_r($user);
//    } else {
//        $user = user_load(array("name" => $handle));
//        print_r($user);
//    }
//}
//function find_primary_cell(&$import_record) {
//
//    // already processed? 
//    if (import_user_log_get_by_rid_type($import_record->record_id, 'id-primary-cell')) {
//        msg("find_primary_cell already processed rid $import_record->record_id");
//        return;
//    }
//
//    // existing sms_user?
//    //   mark as processed
//    if ($row = import_user_log_get_by_rid_type_value($import_record->record_id, 'import', 'existing')) {
//        msg("find_primary_cell existing sms_user for rid $import_record->record_id");
//        $sms_user = db_fetch_object(db_query(
//                        "select * from sms_user where uid = %d limit 1", $row->uid
//                ));
//
//        $log = import_user_log_get_default(
//                $row->uid, $import_record->record_id, 'id-primary-cell', $sms_user->number
//        );
//        import_user_log_insert($log);
//        return;
//    }
//
//    //  has a cell phone?
//    //    create sms_user record
//    //    mark as processed
//    $row = import_user_log_get_by_rid_type_value($import_record->record_id, 'import', 'new');
//    $uniq_phone = get_unique_phones($import_record);
//    foreach ($uniq_phone as $phone) {
//        // else lookup by api and save
//        $url = 'http://digits.cloudvox.com/' . $phone . '.json';
//        $ch = curl_init($url);
//        curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
//        $data = curl_exec($ch);
//        $json = json_decode($data, false);
//        foreach ($json as $cloudvox_info) {
//            if ($cloudvox_info->access_type == "wireless") {
//                msg("find_primary_cell wireless found $row->uid, $phone");
//                $result = db_query(
//                        "
//                        insert into sms_user 
//                        (uid,number, status) values (%d, '%s', %d)
//                        ", $row->uid, $phone, 2
//                );
//                $log = import_user_log_get_default(
//                        $row->uid, $import_record->record_id, 'id-primary-cell', $phone);
//                import_user_log_insert($log);
//                // only take the primary, we're done
//                return;
//            }
//        }
//    }
//}

/**
 * change this to create / update the accounts
 * or just create the initial import txn?
 * created-user
 * updated-user
 */
//function import_user(&$row) {
//    //print_r($row);
//    $existing_user = FALSE;
//    $phone_row = user_exists_by_phone($row);
//    if ($phone_row) {
//        msg("import_user existing user for $phone_row->uid\n" . print_r($row, true));
//        // load user
//        // update fields
//        $existing_user = TRUE;
//        $type = 'import';
//        // if people filled out multiple forms
//        $existing_log = import_user_log_get_by_rid_type($row->record_id, $type);
//        if (!$existing_log) {
//            $value = 'existing';
//            $log = import_user_log_get_default($phone_row->uid, $row->record_id, $type, $value);
//            //import_user_log_insert($log);
//        } else {
//            msg("import_user existing log for $row->record_id, $phone_row->uid");
//        }
//    }
//    if (!$existing_user) {
//        $type = 'import';
//        $value = 'new';
//        $existing_log = import_user_log_get_by_rid_type($row->record_id, $type);
//        //print_r($existing_log);
//        if (!$existing_log) {
//            // create new user
//            $user = false; //create_new_account_for_handle($row->handle);
//            if ($user) {
//                msg("import_user created user $user->uid");
//
//                // link to import record
//                $log = import_user_log_get_default($user->uid, $row->record_id, $type, $value);
//                //import_user_log_insert($log);
//            }
//        } else {
//            msg("import_user skipping create, existing log for $row->record_id");
//        }
//    }
//}
//function create_new_account_for_handle($handle) {
//    // just a random sha hash including time so we can set the password
//    // we don't know/don't care what it is
//    // it's secure and will need to be reset by the user if they register via email later
//    $token = base64_encode(hash_hmac('sha256', $handle, drupal_get_private_key() . time(), TRUE));
//    $token = strtr($token, array('+' => '-', '/' => '_', '=' => ''));
//    $result = db_fetch_object(db_query(
//                    "SELECT max(uid)+1 as maxuid from {users}"
//            ));
//    $details = array(
//        'name' => $handle . "-$result->maxuid",
//        'pass' => $token,
//        'mail' => $handle,
//        'access' => 0,
//        'status' => 1,
//    );
//    return user_save(null, $details);
//}
//function create_new_account_with_sms($number) {
//    $sms_user[0] = array(
//        status => 2,
//        number => $number,
//    );
//
//    // just a random sha hash including time so we can set the password
//    // we don't know/don't care what it is
//    // it's secure and will need to be reset by the user if they register via email later
//    $token = base64_encode(hash_hmac('sha256', $number, drupal_get_private_key() . time(), TRUE));
//    $token = strtr($token, array('+' => '-', '/' => '_', '=' => ''));
//
//    $details = array(
//        'name' => $number,
//        'pass' => $token,
//        'mail' => $number,
//        'access' => 0,
//        'status' => 1,
//        'sms_user' => $sms_user
//    );
//    return user_save(null, $details);
//}
////////////////////////////////////////////////
//// Misc //////////////////////////////////////
////////////////////////////////////////////////
// existing users in users/sms_user
//select uid, phone from ( select uid, name as phone from users where length(name) = 10 and name REGEXP '^[0-9]+$' union distinct select uid, number as phone from sms_user where length(number) = 10 and number REGEXP '^[0-9]+$' ) x;


function bootstrap() {
    //bootstrap drupal. set remote address to quiet errors during bootstrap
    $_SERVER['REMOTE_ADDR'] = "127.1.1.1";
    $_SERVER['REQUEST_METHOD'] = null;
    $_SERVER['SERVER_SOFTWARE'] = null; //!== 'PHP CLI') {

    require_once './includes/bootstrap.inc';
    drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
}

function msg($msg) {
    $date = date('m/d/Y h:i:s a', time());
    $msg = $date . ' ' . $msg;
    echo $msg . "\n";
    watchdog('import_user', $msg);
}

//function clear_watchdog() {
//    $result = db_query(
//            "truncate table {watchdog}"
//    );
//}
//function show_watchdog($limit = 20) {
//    $result = db_query(
//            "SELECT * from {watchdog} order by timestamp desc limit $limit"
//    );
//
//    while ($row = db_fetch_object($result)) {
//        $variables = unserialize($row->variables);
//        if (is_array($variables)) {
//            $row->message = strtr($row->message, $variables);
//        }
//        unset($row->variables);
//        echo $row->timestamp . ": " . $row->message . "\n";
//    }
//}
//function test() {
//    $result = db_query(
//            "SELECT * from {users} limit 1"
//    );
//
//    while ($row = db_fetch_object($result)) {
//        msg($row->uid . ' ' . $row->uid);
//    }
//}
////////////////////////////////////////////////
//// exists by phone ///////////////////////////
////////////////////////////////////////////////
function user_exists_by_phone($row) {
    if ($row->is_cell_valid != 1) {
        return FALSE;
    }

    $result = db_query(
            "
    select su.uid, su.number from sms_user su  where su.number = '%s' limit 1
    ", $row->cell
    );
    return db_fetch_object($result);
}

function user_exists_by_email($row) {
    if ($row->is_email_valid != 1) {
        return FALSE;
    }

    $result = db_query(
            "
    select mail from users u  where u.mail = '%s' limit 1
    ", $row->email
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
    $log->import_table = 'import_user_rural'; //$import_table;
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
            and import_table = 'import_user_rural'
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
            and import_table = 'import_user_rural'
            
            "
            , $rid, $type, $value);

    return db_fetch_object($result);
}

//function import_user_log_get_by_rid_uid_type($rid, $uid, $type) {
//    $result = db_query(
//            "
//            select * from import_user_log 
//            where 
//            rid = %d
//            and
//            uid = %d
//            and
//            log_type = '%s'
//            and import_table = 'import_user_rural'
//            
//            "
//            , $rid, $uid, $type);
//
//    return db_fetch_object($result);
//}
//function import_user_log_get_by_type_value($type, $value) {
//    $result = db_query(
//            "
//            select * from import_user_log 
//            where 
//            log_type = '%s'
//            and
//            log_value = '%s'
//            and import_table = 'import_user_rural'
//            
//            "
//            , $type, $value);
//    return $result;
//}
//function import_user_log_get_by_rid_uid_type_value($rid, $uid, $type, $value) {
//    $result = db_query(
//            "
//            select * from import_user_log 
//            where 
//            rid = %d
//            and
//            uid = %d
//            and
//            log_type = '%s'
//            and
//            log_value = '%s'
//            and import_table = 'import_user_rural'
//            
//            "
//            , $rid, $uid, $type, $value);
//
//    echo "rows = " . count($result) . "\n";
//
//    while ($row = db_fetch_object($result)) {
//        print_r($row);
//    }
//}
////////////////////////////////////////////////
//// PHONE NUMBERS /////////////////////////////
////////////////////////////////////////////////
//function show_phone_nums_valid(&$row) {
//    if ($row->is_cell_valid) {
//        show_phone_nums($row);
//    }
//}
//
//function show_phone_nums(&$row) {
//    msg(
//            '[' .
//            $row->record_id
//            . ']['
//            . $row->cell
//            . ']['
//            . $row->is_cell_valid
//            . ']'
//    );
//}
?>
