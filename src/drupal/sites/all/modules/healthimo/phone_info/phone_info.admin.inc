<?php
/**
 * Implementation of hook_admin_settings() for configuring the module
 */
function phone_info_admin_settings_form(&$form_state) {
  $form['settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Enable module features'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );
  $form['settings']['healthimo_phone_info_cron_active'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable cron for the module'),
    '#default_value' => variable_get('healthimo_phone_info_cron_active', 1),
    '#description' => t('Enables / disable cron for the module.')
  );
  return system_settings_form($form);
}

function phone_info_admin_settings_form_validate($form, &$form_state) {
//  if (!preg_match('/^UA-\d{4,}-\d+$/', $form_state['values']['googleanalytics_account'])) {
//    form_set_error('googleanalytics_account', t('A valid Google Analytics account number is case sensitive and formatted like UA-xxxxxx-x.'));
//  }
}
