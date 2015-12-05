<?php

/**
 * Finds the difference in days between two calendar dates.
 *
 * @param Date $startDate as parsable date string
 * @param Date $endDate as parsable date string
 * @return Int
 */
function healthimo_date_jd_diff($startDate, $endDate) {
    // Parse dates for conversion
    $startArry = date_parse($startDate);
    $endArry = date_parse($endDate);

    // Convert dates to Julian Days
    $start_date = gregoriantojd($startArry["month"], $startArry["day"], $startArry["year"]);
    $end_date = gregoriantojd($endArry["month"], $endArry["day"], $endArry["year"]);

    // Return difference
    return round(($end_date - $start_date), 0);
}

/**
 * @param <type> $d1 DateTime object one
 * @param <type> $d2  DateTime object two
 * @return difference in seconds, + means d2 is after d1, - means d2 is before d1
 */
function healthimo_date_diff($d1, $d2 ) {
    date_default_timezone_set('UTC');
    $d1format = $d1->format("Y-m-d H:i:s");
    $d2format = $d2->format("Y-m-d H:i:s");
    // tz ok whatever it is they are compared in like tz
    //need php 5.3 to use diff... return $d2->diff($d1)->format('s');
    return strtotime($d2format) - strtotime($d1format);
}

/**
 *
 * @param <type> $startTimestamp
 * @param <type> $endTimestamp
 * @return <type>
 */
function healthimo_date_timestamp_jd_diff($startTimestamp, $endTimestamp ) {
    return healthimo_date_jd_diff( gmdate(DATE_ATOM,$startTimestamp), gmdate(DATE_ATOM,$endTimestamp) );
}

/**
 *
 * @param <type> $date1
 * @param <type> $date2
 * @param <type> $days
 * @return <type>
 */
function healthimo_date_is_diff_n_or_more_days($date1, $date2, $days) {
    return
            $date1 > 0
            &&
            healthimo_date_timestamp_jd_diff( $date1, $date2 ) >= $days;
//	return healthimo_date_jd_diff( $date1, $date2) >= $days;
}

/**
 * @param <type> $timestamp point in time
 * @param <type> $tz olsen time zone
 * @return <type> DateTime objected with timestamp displaye in tz
 */
function healthimo_date_get_local_time($timestamp, $tz) {
    date_default_timezone_set('UTC');
    $dtzone = new DateTimeZone($tz);
    // first convert the timestamp into a string representing the local time
    $time = date('r', $timestamp);
    // now create the DateTime object for this time
    $dtime = new DateTime($time);
    // convert this to the user's timezone using the DateTimeZone object
    $dtime->setTimeZone($dtzone);
    return $dtime;
}

function healthimo_date_get_local_time_now($tz) {
    return healthimo_date_get_local_time(time(), $tz);
}

/**
 * @param <type> $timestamp
 * @param <type> $tz
 * @return <type>
 */
function healthimo_date_get_local_hour_of_day($timestamp, $tz) {
    return healthimo_date_get_local_time($timestamp, $tz)->format('H');
}

/**
 * 8am - 5pm == 8, 18 (>=8 && < 18)
 * @param <type> $timestamp
 * @param <type> $startHour
 * @param <type> $endHour
 * @param <type> $tz
 * @return <type>
 */
function healthimo_date_is_in_window_hours($timestamp, $startHour, $endHour, $tz = "America/Chicago") {
    $hour = healthimo_date_get_local_time($timestamp, $tz)->format('H');
    healthimo_log('healthimo_date_is_in_window_hours hour=' .$hour);
    return $hour >= $startHour && $hour < $endHour;
}

function healthimo_date_now_as_atom() {
    return gmdate(DATE_ATOM,time());
}

/**
 * @param <type> $time_of_day
 * @return <type> datetime object for 'now' with hour and minute set to the paseed in time of day
 */
function healthimo_date_time_of_day_to_today_time($time_of_day, $tz) {
    if (empty($time_of_day)) {
        return FALSE;
    }

    $now = healthimo_date_get_local_time_now($tz);
    sscanf( trim($time_of_day), "%d:%d", $h, $m );
    date_time_set($now,$h,$m);
    return $now;
}

function healthimo_date_get_sunday_beginning_of_this_week($tz) {
    date_default_timezone_set($tz);
    $dow = date("w");
    $timestamp = mktime(0,0,0,date('m'), date('d')-$dow, date('Y'));
    $date = healthimo_date_get_local_time($timestamp, $tz);
    date_time_set($date,0,0);
    date_default_timezone_set('UTC');
    return $date;
}

/**
 *
 * @param <type> $timeString time parsable by strtotime
 * @param <type> $tz olson time zone string
 * @return <type> datetime object representing timestring in tz timezone
 */
function healthimo_date_make_local_time($timeString, $tz) {
    if (empty($timeString)) {
        return FALSE;
    }
    date_default_timezone_set($tz);
    $timestamp = strtotime($timeString);
    $date = healthimo_date_get_local_time($timestamp, $tz);
    date_default_timezone_set('UTC');
    return $date;
}