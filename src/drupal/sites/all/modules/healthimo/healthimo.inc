<?php

define('PROFILE_CATEGORY_MOBILE', 'Mobile');
define('PROFILE_CATEGORY_ASTHMA', 'Asthma');
define('PROFILE_CATEGORY_PERSONAL_INFO', 'Personal Information');
define('PROFILE_CATEGORY_PHR', 'Personal Private Health Record');

function healthimo_profile_save($account, $field, $value, $category) {
    //$category ignored since fields in this system are all unique and looking it up is problematic
    //$fid_result = db_query("select fid from profile_fields where name = '%s' and category = '%s'", $field, $category);
    $fid_result = db_query("select fid from profile_fields where name = '%s'", $field );
    if ($fid_result) {
        $field = db_fetch_object($fid_result);
        $fid = $field->fid;
        $result = db_query("select 1 as flag from profile_values where fid = %d and uid = %d", $fid, $account->uid);
        if (db_fetch_object($result)->flag == 1) {
            db_query("update profile_values set value = '%s' where fid = %d and uid = %d", $value, $fid, $account->uid);
        } else {
            db_query("insert into profile_values (fid, uid, value) values ( %d, %d, '%s' )", $fid, $account->uid, $value);
        }
        profile_load_profile($account);
    }
    return $account;
}

function healthimo_profile_has_mail($account) {
    return!empty($account->mail) && strpos($email, '@') === true;
}

function healthimo_profile_get_mail($account) {
    return $account->mail;
}

function healthimo_profile_has_sms($account) {
    return!empty($account->sms_user) && $account->sms_user[0]['status'] == 2 && !empty($account->sms_user[0]['number']);
}

function healthimo_profile_get_sms($account) {
    return $account->sms_user[0]['number'];
}

function healthimo_log($log, $level = WATCHDOG_DEBUG) {
    //print( $log );
    watchdog('healthimo', $log, NULL, $level);
}

function healthimo_log_debug($type, $log, $variables = array()) {
    watchdog($type, $log, $variables, WATCHDOG_DEBUG);
}

function healthimo_log_alert($type, $log, $variables = array()) {
    watchdog($type, $log, $variables, WATCHDOG_ALERT);
}

