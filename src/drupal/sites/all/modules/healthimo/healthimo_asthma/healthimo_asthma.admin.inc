<?php

/**
 * Implementation of hook_admin_settings() for configuring the module
 */
function healthimo_asthma_admin_settings_form(&$form_state) {
    $form['settings'] = array(
        '#type' => 'fieldset',
        '#title' => t('Enable module features'),
        '#collapsible' => TRUE,
        '#collapsed' => FALSE,
    );
    $form['settings']['healthimo_healthimo_asthma_cron_active'] = array(
        '#type' => 'checkbox',
        '#title' => t('Enable cron for the module'),
        '#default_value' => variable_get('healthimo_healthimo_asthma_cron_active', 1),
        '#description' => t('Enables / disable cron for the module.')
    );
    $form['settings']['healthimo_healthimo_asthma_ozone_alert_active'] = array(
        '#type' => 'checkbox',
        '#title' => t('Enable ozone alerts'),
        '#default_value' => variable_get('healthimo_healthimo_asthma_ozone_alert_active', 1),
        '#description' => t('Enables ozone alerts for the module')
    );
    $form['settings']['healthimo_healthimo_asthma_pollen_alert_active'] = array(
        '#type' => 'checkbox',
        '#title' => t('Enable pollen alerts'),
        '#default_value' => variable_get('healthimo_healthimo_asthma_pollen_alert_active', 1),
        '#description' => t('Enables pollen alerts for the module')
    );
    $form['settings']['healthimo_healthimo_asthma_controller_reminder_logging'] = array(
        '#type' => 'checkbox',
        '#title' => t('Enable detailed controller reminder logging'),
        '#default_value' => variable_get('healthimo_healthimo_asthma_controller_reminder_logging', 0),
        '#description' => t('Enables detailed logging for controller reminders')
    );
    $form['settings']['healthimo_healthimo_asthma_controller_reminder_active'] = array(
        '#type' => 'checkbox',
        '#title' => t('Enable controller reminders'),
        '#default_value' => variable_get('healthimo_healthimo_asthma_controller_reminder_active', 1),
        '#description' => t('Enables controller reminders for the module')
    );
    $form['settings']['healthimo_healthimo_asthma_appt_reminder_active'] = array(
        '#type' => 'checkbox',
        '#title' => t('Enable appointment reminders'),
        '#default_value' => variable_get('healthimo_healthimo_asthma_appt_reminder_active', 1),
        '#description' => t('Enables appointment reminders for the module')
    );
    $form['settings']['healthimo_asthma_max_to_send_per_run'] = array(
        '#type' => 'textfield',
        '#title' => t('Max users to messsage per cron run'),
        '#cols' => 20,
        '#maxlength' => 20,
        '#required' => TRUE,
        '#default_value' => variable_get('healthimo_asthma_max_to_send_per_run', 1),
        '#description' => t('healthimo_asthma_max_to_send_per_run')
    );
    
    //

    return system_settings_form($form);
}

function healthimo_asthma_admin_settings_form_validate($form, &$form_state) {
    
}

function healthimo_asthma_admin_enroll_form() {
    $form['enroll'] = array(
        '#type' => 'fieldset',
        '#title' => t('Enroll asthma participants'),
        '#collapsible' => TRUE,
        '#collapsed' => FALSE,
    );
    $form['enroll']['numberToEnroll'] = array(
        '#type' => 'textfield',
        '#title' => t('Number of people to enroll'),
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

function healthimo_asthma_admin_enroll_form_validate($form, &$form_state) {
    $numberToEnroll = trim($form_state['values']['numberToEnroll']);
    if (!ctype_digit($numberToEnroll)) {
        form_set_error('numberToEnroll', t('Number of people to enroll must be a number'));
        return;
    }

    if ($numberToEnroll > 100) {
        form_set_error('numberToEnroll', t('Max number of people to enroll is 100'));
        return;
    }
}

/**
 * Submit handler for the sms blast form
 */
function healthimo_asthma_admin_enroll_form_submit(&$form, $form_state) {
    $numberToEnroll = healthimo_string_clean_input((trim($form_state['values']['numberToEnroll'])));
    $msg = t('Enrolling %numberToEnroll users.', array('%numberToEnroll' => $numberToEnroll));
    $dryrun = $form_state['values']['dryrun'];
    healthimo_log($msg);
    drupal_set_message($msg);

    $result = db_query(
            "
        select uid, log_value as value from import_user_log iul where log_type = 'id-primary-cell'
        and not exists (select 1 from import_user_log iul2 where iul2.uid = iul.uid and iul2.log_type = 'invited')
        limit %d
            ", $numberToEnroll
    );

    $invite = "Reply HEALTHY ASTHMA to get FREE asthma education by Driscoll Childrens and healthimo. Reply stop to end.";
    $i = 0;
    while ($row = db_fetch_object($result)) {
        $uid = $row->uid;
        $phone = $row->value;
        $i++;

        $msg = t('healthy asthma invite to %uid via %phone', array('%uid' => $uid, '%phone' => $phone));
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
                        insert into import_user_log
                        (created, rid, uid, import_table, log_type, log_value)
                        values
                        (%d,%d,%d,'%s','%s','%s')
                        ", time(), null, $uid, null, 'invited', $phone);
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
    $msg = t('%enrolled people were invited to healthy asthma.', array('%enrolled' => $i));
    healthimo_log($msg);
    drupal_set_message($msg);
}

