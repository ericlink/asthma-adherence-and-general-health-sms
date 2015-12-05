<?php

/**
 * Implementation of hook_admin_settings() for configuring the module
 */
function healthimo_admin_settings_form(&$form_state) {
    $form['settings'] = array(
        '#type' => 'fieldset',
        '#title' => t('Enable module features'),
        '#collapsible' => TRUE,
        '#collapsed' => FALSE,
    );
    $form['settings']['healthimo_cron_active'] = array(
        '#type' => 'checkbox',
        '#title' => t('Enable cron for the healthimo module'),
        '#default_value' => variable_get('healthimo_cron_active', 1),
        '#description' => t('Enables / disable cron for the module.')
    );
    $form['settings']['healthimo_messaging_outbound_to_admin_user_only'] = array(
        '#type' => 'checkbox',
        '#title' => t('Send outbound messages to admin user only'),
        '#default_value' => variable_get('healthimo_messaging_outbound_to_admin_user_only', 1),
        '#description' => t('Send outbound messages to admin user only')
    );
    $form['settings']['healthimo_max_questions_for_single_session'] = array(
        '#type' => 'textfield',
        '#cols' => 20,
        '#maxlength' => 20,
        '#required' => TRUE,
        '#title' => t('Max number of questions for a single session'),
        '#default_value' => variable_get('healthimo_max_questions_for_single_session', 4),
        '#description' => t('healthimo_max_questions_for_single_session')
    );
    return system_settings_form($form);
}

function healthimo_admin_settings_form_validate($form, &$form_state) {
//  if (!preg_match('/^UA-\d{4,}-\d+$/', $form_state['values']['googleanalytics_account'])) {
//    form_set_error('googleanalytics_account', t('A valid Google Analytics account number is case sensitive and formatted like UA-xxxxxx-x.'));
//  }
}

function healthimo_admin_reset_admin_form(&$form_state) {
    $form['settings'] = array(
        '#type' => 'fieldset',
        '#title' => t('Enable module features'),
        '#collapsible' => FALSE,
        '#collapsed' => FALSE,
    );
    $form['settings']['healthimo_reset_admin'] = array(
        '#type' => 'checkbox',
        '#title' => t('Reset the admin users messaging state?'),
        '#default_value' => 0,
        '#description' => t('Clears hfstx log, asthma log, profile flags for admin user')
    );
    return system_settings_form($form);
}

function healthimo_admin_reset_admin_form_submit($form, &$form_state) {
    if ($form_state['values']['healthimo_reset_admin'] == 1) {
        // clear hfstx log
        db_query(
                "
                DELETE
                FROM {hfstx_message_log}
                WHERE
                uid = 1        
                "
        );
        // clear asthma log
        db_query(
                "
                DELETE
                FROM {healthimo_asthma_message_log} 
                WHERE
                uid = 1        
                "
        );
        // clear all profile values
        db_query(
                "
                DELETE
                FROM {profile_values} 
                WHERE
                uid = 1        
                "
        );
        form_set_error('test', t('uid 1 reset'));
    }
}









function healthimo_admin_wii_form() {
    $form['enroll'] = array(
        '#type' => 'fieldset',
        '#title' => t('Notify users about wii contest'),
        '#collapsible' => TRUE,
        '#collapsed' => FALSE,
    );
    $form['enroll']['numberToEnroll'] = array(
        '#type' => 'textfield',
        '#title' => t('Number of people to notify'),
        '#cols' => 4,
        '#maxlength' => 4,
        '#required' => TRUE,
        '#default_value' => '',
    );
    $form['enroll']['dryrun'] = array(
        '#type' => 'checkbox',
        '#title' => t('Dry run'),
        '#cols' => 60,
    );

    $form['enroll']['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Enroll'),
    );

    return $form;
}

function healthimo_admin_wii_form_validate($form, &$form_state) {
    $numberToEnroll = trim($form_state['values']['numberToEnroll']);
    if (!ctype_digit($numberToEnroll)) {
        form_set_error('numberToEnroll', t('Number of people to enroll must be a number'));
        return;
    }

    if ($numberToEnroll > 100) {
        form_set_error('numberToEnroll', t('Max number of people to notify is 100'));
        return;
    }
}

/**
 * Submit handler for the sms blast form
 */
function healthimo_admin_wii_form_submit(&$form, $form_state) {
    $numberToEnroll = healthimo_string_clean_input((trim($form_state['values']['numberToEnroll'])));
    $msg = t('Notifying maximum of %numberToEnroll users.', array('%numberToEnroll' => $numberToEnroll));
    $dryrun = $form_state['values']['dryrun'];
    healthimo_log($msg);
    drupal_set_message($msg);

    $result = db_query(
            "
        select uid, number from sms_user where uid in ( select distinct pv.uid from profile_values pv join profile_fields pf on pf.fid = pv.fid   where ( (pf.name = 'profile_keyword_asthma' and pv.value = '1') or (pf.name = 'profile_keyword_healthy' and pv.value = '1')) and pv.uid not in ( select pv.uid from profile_values pv join profile_fields pf on pf.fid = pv.fid where pf.name = 'profile_keyword_stop' and pv.value = '1') and pv.uid not in (select uid from hfstx_message_log where message_nid = 992192))
        limit %d
            ", $numberToEnroll
    );

    $invite = "Win a Wii game system-answer the most Healthy Families questions by 8-31 and be entered to win. Random drawing on 8-31. Healthy Families";
    $i = 0;
    while ($row = db_fetch_object($result)) {
        $uid = $row->uid;
        $phone = $row->number;
        $i++;

        $msg = t('wii notifications to %uid via %phone', array('%uid' => $uid, '%phone' => $phone));
        healthimo_log($msg);
        drupal_set_message($msg);

        //TEST my cell: 
        //$phone = '2144776376';
        //  no response 'per se', since they are texting back with a keyword
        if ($dryrun != 1) {
            $rc = healthimo_sms_outgoing($phone, $invite, array('response_expected' => 0,));
            if ($rc || $rc == -104) {
                $result2 = db_query(
                        "
                        insert into hfstx_message_log
                        (
                        mid, created,updated,processed,
                        outbound,uid,program_tid,address,
                        message,message_nid,response_expected,
                        response_type_expected,response_received      
                        )
                        values
                        (%d,%d,%d,%d,
                        %d,%d, %d, '%s',
                        '%s',%d,%d,
                        %d,%d)
                        ", 
                        null, time(), time(), time(),
                        1, $uid, 2, $phone,
                        $invite, 992192, 0,
                        null, null
                        );
                if ($result2 == FALSE) {
                    $msg = t('Error processing');
                    healthimo_log($msg);
                    form_set_error($msg);
                    break;
                }
            } else {
                form_set_error("Error $rc sending see message log");
            }
        }
    }
    $msg = t('%enrolled people were notified.', array('%enrolled' => $i));
    healthimo_log($msg);
    drupal_set_message($msg);
}


