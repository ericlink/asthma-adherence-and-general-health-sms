<?php

define('MAX_APPOINTMENT_REMINDER_TO_SEND_PER_RUN', 100);

function healthimo_asthma_send_appointment_reminders() {
    $usersMessaged = 0;

    $result = healthimo_asthma_find_users();
    if ($result) {
        while ($row = db_fetch_object($result)) {
            $user = user_load($row->uid);
            //profile_keyword_stop
            if ($user->profile_keyword_stop != 1
                    && $user->profile_asthma_feature_appt_reminder_active == 1) {
                $sent = false;
//////////////////////////////////
                $sent = healthimo_asthma_send_appointment_reminder($user);
//////////////////////////////////
                if ($sent) {
                    $usersMessaged++;
                    if ($usersMessaged >= MAX_APPOINTMENT_REMINDER_TO_SEND_PER_RUN) {
                        return $usersMessaged;
                    }
                }
            }
        }
    }

    return $usersMessaged;
}

/**
 *
 * Appointments must be 'in order'
 * @param <type> $user
 * @return <type>
 */
function healthimo_asthma_send_appointment_reminder($user) {
    if (isset($user->profile_asthma_feature_appt_reminder_appt_list)
            && !empty($user->profile_asthma_feature_appt_reminder_appt_list)
    ) {
        $appt_reminder_list = healthimo_string_get_newline_delimited_items(trim($user->profile_asthma_feature_appt_reminder_appt_list));
    } else {
        return FALSE;
    }

    foreach ($appt_reminder_list as $appt) {
        $appt = trim($appt);
        healthimo_asthma_log("appt time=[" . $appt . "]");

        $tz = 'America/Chicago';

        $appointment_date = healthimo_date_make_local_time($appt, $tz);
        if ($appointment_date === FALSE) {
            return FALSE;
        }

        $appointment_reminder_time = healthimo_date_make_local_time($appt, $tz);
        $appointment_reminder_time->modify('-1 day');
        healthimo_asthma_log("appt reminder time=[" . $appointment_reminder_time->format('r') . "]");

        // date time obj for 'now' local time
        $now = healthimo_date_get_local_time_now($tz);
        if (healthimo_date_diff($appointment_date, $now) < 0
                && healthimo_date_diff($appointment_reminder_time, $now) >= 0
        ) {
            $last_appointment_reminder_sent = healthimo_asthma_get_last_appointment_reminder_sent($user);
            if ($last_appointment_reminder_sent) {
                $last_sent_in_local_time = healthimo_date_get_local_time($last_appointment_reminder_sent->created, $tz);
                if (healthimo_date_diff($last_sent_in_local_time, $appointment_reminder_time) > 0) {
                    return healthimo_asthma_send_appointment_reminder_to_all_destinations($user, $appointment_date);
                }
            } else {
                // no reminders sent yet, so need to send one
                return healthimo_asthma_send_appointment_reminder_to_all_destinations($user, $appointment_date);
            }
        }
    } ////// END LOOP FOR EACH

    return FALSE;
}

function healthimo_asthma_send_appointment_reminder_to_all_destinations($user, $appointment_date) {
    $reminder = $user->name . ' has a home visit tomorrow at ' . $appointment_date->format('g') . ':' . $appointment_date->format('i') . ' ' . $appointment_date->format('a') . ' at your home. Thanks, your home health nurse.';
    return healthimo_asthma_send_to_all_destinations($user, 'Home Health Appointment Reminder', $reminder, MESSAGE_TYPE_APPOINTMENT_REMINDER);
}

function healthimo_asthma_get_last_appointment_reminder_sent($user) {
    return healthimo_asthma_message_log_load_most_recent_by_type($user->uid, MESSAGE_TYPE_APPOINTMENT_REMINDER);
}
