<?php

function hfstx_send($account, $message_log, $hasEmail, $hasSms) {
    $success = TRUE;

    if ($message_log) {
        if ($hasEmail) {
            $success = hfstx_send_to_email($account, $message_log) && $success;
        }
        if ($hasSms) {
            $success = hfstx_send_to_sms($account, $message_log) && $success;
        }
    }

    return $success;
}

function hfstx_send_to_email($account, $message_log) {
    //$email = DEBUG ? 'link@sandlion.com' : $account->mail;
    $email = $account->mail;

    if (strpos($email, '@') === false) {
        // return ok even though email was no good,
        // because we have bogus emails that are really tel nums for text registrations
        // (needed to create the account)
        return true;
    }

    $program_tid = 1; //lookup program tid
    // shallow clone is ok for message struct. Also, has no mid
    $message_log = clone($message_log);
    $message_log->address = $email;
    $message_log = hfstx_message_log_save($message_log);

    if ($message_log->response_expected) {
        $message_log->message = "Update your profile " . $message_log->message;
    }

    $subject =
            'Healthy Families - '
            . trim($message_log->message)
            . ' '
    ;

    $body =
            'Hello ' . $account->name . ','
            . "\n"
            . "\n"
            . trim($message_log->message)
            . "\n"
            . "\n"
    ;

    if ($message_log->response_expected) {
        $body .=
                "\n\nClick on the following link to update your profile:\n"
                . "https://hfstx.healthimo.com/user/$account->uid/edit/Personal%20Information"
        ;
    }

    $email = array(
        'to' => $email,
        'subject' => $subject,
        'body' => $body,
        'headers' => array('From' => 'guide@healthimo.com'),
    );

    healthimo_log('send email to ' . $email['to']);

//    //if (SEND_ENABLED) {
//        drupal_mail_send($email);
//    //}

    $message_log->processed = time();
    $message_log = hfstx_message_log_save($message_log);

    return TRUE;
}

function hfstx_send_to_sms($account, $message_log) {
    //$phone = DEBUG ? '2146415465' : $account->sms_user[0]['number'];
    $phone = $account->sms_user[0]['number'];

    $program_tid = 1; //lookup program tid
    // shallow clone is ok for message struct. Also, has no mid
    $message_log = clone($message_log);
    $message_log->address = $phone;
    $message_log = hfstx_message_log_save($message_log);

    if ($message_log->response_expected) {
        $account = healthimo_profile_save(
                $account, 'profile_question_count', $account->profile_question_count + 1, PROFILE_CATEGORY_PHR
        );
    }

    healthimo_log(' send sms');

    //if (SEND_ENABLED) {
    //healthimo_sms_send
    healthimo_sms_outgoing(
            $phone, trim($message_log->message), array('response_expected' => $message_log->response_expected,)
    );
    //}

    $message_log->processed = time();
    $message_log = hfstx_message_log_save($message_log);

    return TRUE;
}

