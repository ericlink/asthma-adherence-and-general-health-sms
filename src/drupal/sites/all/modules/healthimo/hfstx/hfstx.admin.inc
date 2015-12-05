<?php

/**
 * Implementation of hook_admin_settings() for configuring the module
 */
function hfstx_admin_settings_form(&$form_state) {
    $form['settings'] = array(
        '#type' => 'fieldset',
        '#title' => t('Enable module features'),
        '#collapsible' => TRUE,
        '#collapsed' => FALSE,
    );
    $form['settings']['hfstx_cron_active'] = array(
        '#type' => 'checkbox',
        '#title' => t('Enable cron for the module'),
        '#default_value' => variable_get('hfstx_cron_active', 1),
        '#description' => t('Enables / disable cron for the module.')
    );
    $form['settings']['hfstx_min_message_interval_days_for_request'] = array(
        '#type' => 'textfield',
        '#cols' => 20,
        '#maxlength' => 20,
        '#required' => TRUE,
        '#title' => t('Min days between requests (regardless of messages sent out)'),
        '#default_value' => variable_get('hfstx_min_message_interval_days_for_request', 7),
        '#description' => t('hfstx_min_message_interval_days_for_request')
    );
    $form['settings']['hfstx_min_times_to_send_before_first_request'] = array(
        '#type' => 'textfield',
        '#cols' => 20,
        '#maxlength' => 20,
        '#required' => TRUE,
        '#title' => t('Min numbers of outbound messages (education) before first request'),
        '#default_value' => variable_get('hfstx_min_times_to_send_before_first_request', 4),
        '#description' => t('hfstx_min_times_to_send_before_first_request')
    );
    $form['settings']['hfstx_window_start_hour'] = array(
        '#type' => 'textfield',
        '#cols' => 20,
        '#maxlength' => 20,
        '#required' => TRUE,
        '#title' => t('Sending window - start hour'),
        '#default_value' => variable_get('hfstx_window_start_hour', 8),
        '#description' => t('hfstx_window_start_hour')
    );
    $form['settings']['hfstx_window_end_hour'] = array(
        '#type' => 'textfield',
        '#cols' => 20,
        '#maxlength' => 20,
        '#required' => TRUE,
        '#title' => t('Sending window - end hour'),
        '#default_value' => variable_get('hfstx_window_end_hour', 20),
        '#description' => t('hfstx_window_end_hour')
    );
    $form['settings']['hfstx_min_message_interval_days'] = array(
        '#type' => 'textfield',
        '#cols' => 20,
        '#maxlength' => 20,
        '#required' => TRUE,
        '#title' => t('Min days between messages'),
        '#default_value' => variable_get('hfstx_min_message_interval_days', 3),
        '#description' => t('hfstx_min_message_interval_days')
    );
    $form['settings']['hfstx_max_to_send_per_run'] = array(
        '#type' => 'textfield',
        '#cols' => 20,
        '#maxlength' => 20,
        '#required' => TRUE,
        '#title' => t('Max users to messsage per cron run'),
        '#default_value' => variable_get('hfstx_max_to_send_per_run', 1),
        '#description' => t('hfstx_max_to_send_per_run')
    );
    return system_settings_form($form);
}

function hfstx_admin_settings_form_validate($form, &$form_state) {
//  if (!preg_match('/^UA-\d{4,}-\d+$/', $form_state['values']['googleanalytics_account'])) {
//    form_set_error('googleanalytics_account', t('A valid Google Analytics account number is case sensitive and formatted like UA-xxxxxx-x.'));
//  }
}
