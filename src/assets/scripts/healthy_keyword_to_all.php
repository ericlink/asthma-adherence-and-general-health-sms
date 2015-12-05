<?php
//
//define('PROFILE_CATEGORY_MOBILE', 'Mobile');
//define('PROFILE_CATEGORY_PERSONAL_INFO', 'Personal Information');
//define('PROFILE_CATEGORY_PHR', 'Personal Private Health Record');
//
//
//require_once './includes/bootstrap.inc';
//drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
//
//$result = db_query(
//        "SELECT uid from {users}"
//);
//while ($row = db_fetch_object($result)) {
//    healthimo_profile_save(
//            (object) array('uid' => $row->uid),
//            'profile_keyword_bcbs', 1, PROFILE_CATEGORY_MOBILE
//    );
//    healthimo_profile_save(
//            (object) array('uid' => $row->uid),
//            'profile_keyword_healthy', 1, PROFILE_CATEGORY_MOBILE
//    );
//}
//
//function healthimo_profile_save($account, $field, $value, $category ) {
//    // run as admin so can see all fields
//    global $user;
//    $olduser = $user;
//    $user = user_load(1);
//
//    // default to existing values
//    profile_load_profile($account);
//    $edit = _healthimo_profile_save_get_edit_array($account, $category);
//
//    // use whatever was passed in
//    $edit[$field] = $value;
//
//    user_save(
//            (object) array('uid' => $account->uid),
//            $edit,
//            $category
//    );
//
//    $user = $olduser;
//
//    return $user;
//}
//
//function _healthimo_profile_save_get_edit_array($account, $category ) {
//    $categories = array(
//            PROFILE_CATEGORY_MOBILE =>  array(
//                    'profile_keyword_bcbs' => $account->profile_keyword_bcbs,
//                    'profile_keyword_healthy' => $account->profile_keyword_healthy,
//            ),
//            PROFILE_CATEGORY_PERSONAL_INFO =>  array(
//                    'profile_goal' => $account->profile_goal,
//                    'profile_activity' => $account->profile_activity,
//                    'profile_zip_code' => $account->profile_zip_code,
//                    'profile_areas_of_interest_diabetes' => $account->profile_areas_of_interest_diabetes,
//                    'profile_areas_of_interest_general' => $account->profile_areas_of_interest_general,
//                    'profile_areas_of_interest_weight' => $account->profile_areas_of_interest_weight,
//                    'profile_areas_of_interest_reply' => $account->profile_areas_of_interest_reply,
//            ),
//            PROFILE_CATEGORY_PHR =>  array(
//                    'profile_height' => $account->profile_height,
//                    'profile_weight' => $account->profile_weight,
//                    'profile_gender' => $account->profile_gender,
//                    'profile_last_12mos_a1c_count' => $account->profile_last_12mos_a1c_count,
//                    'profile_most_recent_a1c' => $account->profile_most_recent_a1c,
//                    'profile_last_kidney_checkup' => $account->profile_last_kidney_checkup,
//                    'profile_last_lipid_profile' => $account->profile_last_lipid_profile,
//                    'profile_ldl_cholesterol' => $account->profile_ldl_cholesterol,
//                    'profile_bp_reading' => $account->profile_bp_reading,
//                    'profile_last_dilated_eye_exam' => $account->profile_last_dilated_eye_exam,
//                    'profile_last_foot_exam' => $account->profile_last_foot_exam,
//                    'profile_had_smoking_cessation_counseling' => $account->profile_had_smoking_cessation_counseling,
//            ),
//    );
//    return $categories[$category];
//}
//
//
////while ($row = db_fetch_object($result)) {
////    $edit = array(
////            'profile_zip_code' => 'newziposfiuosudfs',
////    );
////    user_save(
////            (object) array('uid' => $row->uid),
////            $edit,
////            'Personal Information' // category
////    );
////}
//
//
////$result = db_query(
////        "SELECT uid from {users}"
////);
////
////while ($row = db_fetch_object($result)) {
////    $account = user_load($row->uid);
////    print " $account->profile_keyword_healthy";
////}
?>
