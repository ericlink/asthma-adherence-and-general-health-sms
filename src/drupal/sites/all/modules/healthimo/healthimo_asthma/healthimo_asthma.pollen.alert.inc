<?php

define('MAX_POLLEN_ALERT_TO_SEND_PER_RUN', 100);

function healthimo_asthma_send_pollen_alerts() {
    $usersMessaged = 0;

    $result = healthimo_asthma_find_users();
    if ( $result ) {
        $havePollenAlertData = healthimo_asthma_get_pollen_alert_data();
        while ($havePollenAlertData && $row = db_fetch_object($result)) {
            $user = user_load($row->uid);
            if ( $user->profile_asthma_feature_pollen_alert_active == 1 ) {
                $sent = false;
//////////////////////////////////
                $sent = healthimo_asthma_send_pollen_alert($user);
//////////////////////////////////
                if ( $sent ) {
                    $usersMessaged++;
                    if ( $usersMessaged >= MAX_POLLEN_ALERT_TO_SEND_PER_RUN) {
                        return $usersMessaged;
                    }
                }
            }
        }
    }

    return $usersMessaged;
}

function healthimo_asthma_get_pollen_alert_data() {
    // cron limit vs successful check
    // last checked, vs last successful check (e.g. null message no alert)
    // last checked etc. is local to the stations (e.g. chicago here)
    // if message is null then consider checking
    // when is last checked? today and have message? then done
    //  if was yesterday, go ahead w/ the check
    // check every 15 minutes max?
    // pollen only makes sense to check after 7am since manually done
    // only bother to check until noon or so?
    // cache for today / last fetched config vars
    //healthimo_healthimo_asthma_pollen_alert_last_checked
    //healthimo_healthimo_asthma_pollen_alert_sent_count
    //healthimo_healthimo_asthma_pollen_alert_message
    // set data to null
    // try to get
    // parse
    // find a value
    // put value into var
    // set last fetched time
    return TRUE;
}

function healthimo_asthma_send_pollen_alert($user) {
    // if we have date we are here
    // so are we sending per controller am time or fixed time?
    // is after time to send?
    //   (if db perf is an issue, check if that it is no more than N minutes after time to send?)
    // have not sent yet?
    // send one
    $tz = 'America/Chicago';

    $appointment_date = healthimo_date_make_local_time($appt, $tz);
    if ( $appointment_date === FALSE ) {
        return FALSE;
    }

    $appointment_reminder_time = healthimo_date_make_local_time($appt, $tz);
    $appointment_reminder_time->modify( '-1 day' );
    healthimo_asthma_log("appt reminder time=[".$appointment_reminder_time->format('r')."]");

    // date time obj for 'now' local time
    $now = healthimo_date_get_local_time_now($tz);
    if ( healthimo_date_diff($appointment_date, $now) < 0
            &&  healthimo_date_diff($appointment_reminder_time, $now) >= 0
    ) {
        $last_appointment_reminder_sent = healthimo_asthma_get_last_appointment_reminder_sent($user);
        if ($last_appointment_reminder_sent) {
            $last_sent_in_local_time = healthimo_date_get_local_time($last_appointment_reminder_sent->created, $tz );
            if ( healthimo_date_diff($last_sent_in_local_time, $appointment_reminder_time ) > 0) {
                return healthimo_asthma_send_appointment_reminder_to_all_destinations($user, $appointment_date);
            }
        } else {
            // no reminders sent yet, so need to send one
            return healthimo_asthma_send_appointment_reminder_to_all_destinations($user, $appointment_date);
        }
    }

    return FALSE;
}

function healthimo_asthma_send_pollen_alert_to_all_destinations($user, $level) {
    $alert = '';
    //$level = HIGH, RED, PURPLE, etc...
    //$reminder = $user->name.' has a home visit tomorrow at '.$appointment_date->format('g').':'.$appointment_date->format('i').' '.$appointment_date->format('a').' at your home. Thanks, your home health nurse.';
    return healthimo_asthma_send_to_all_destinations($user, 'Pollen Alert', $alert, MESSAGE_TYPE_APPOINTMENT_REMINDER);
}

function healthimo_asthma_get_last_pollen_alert_sent($user) {
    return healthimo_asthma_message_log_load_most_recent_by_type($user->uid, MESSAGE_TYPE_POLLEN_ALERT);
}
