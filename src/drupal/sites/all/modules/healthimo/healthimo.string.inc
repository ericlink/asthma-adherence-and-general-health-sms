<?php

/**
 * @param <type> $word
 * @param <type> $string
 * @return <type> TRUE if word is found in string
 */
function healthimo_contains_word_ignore_case($word, $string) {
    return preg_match("/\b" . $word . "\b/i", $string);
}

function healthimo_string_contains_word_ignore_case($word, $string) {
    return healthimo_contains_word_ignore_case($word, $string);
}

/**
 *
 * @param <type> $word
 * @param <type> $string
 * @return <type> TRUE if contained (regardless of case or word boundary)
 */
function healthimo_string_contains_ignore_case($word, $string) {
    return preg_match("/" . $word . "/i", $string);
}

function healthimo_string_get_newline_delimited_items($packed) {
    return explode("\n", trim($packed));
}

function healthimo_string_clean_input($input) {
    return htmlspecialchars($input, ENT_QUOTES, 'UTF-8');
}

function healthimo_string_get_words($string) {
    return explode(" ", trim($string));
}