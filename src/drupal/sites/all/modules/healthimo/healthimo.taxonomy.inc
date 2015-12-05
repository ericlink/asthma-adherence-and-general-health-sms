<?php
/**
 *
 * @param <type> $name
 * @return $vid
 */
function healthimo_taxonomy_get_vocabulary_by_name($name) {
    $vocabularies = taxonomy_get_vocabularies();
    foreach ($vocabularies as $vocabulary) {
        if ( $vocabulary->name == $name) {
            return $vocabulary->vid;
        }
    }
}

/**
 * Alias for taxonomy_get_term_by_name;
 * @param <type> $name
 * @return <type>
 */
function healthimo_taxonomy_get_term_by_name($name) {
    $terms = taxonomy_get_term_by_name($name);
    return  $terms[0];
}


function healthimo_taxonomy_get_sms_keyword_by_name($name) {
    $terms = taxonomy_get_term_by_name($name);
    $term = count($terms) > 0 ? $terms[0] : null;

    if ( !empty($term) && $term->vid > 0 ) {
        $vid_sms = healthimo_taxonomy_get_vocabulary_by_name('SMS Keywords');
        if ( $term->vid == $vid_sms ) {
            return $term;
        }
    }

    return FALSE;
}