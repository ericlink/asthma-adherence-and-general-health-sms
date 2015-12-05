<?php

require_once './includes/bootstrap.inc';
drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);

error_reporting(E_ALL & ~E_NOTICE | E_STRICT );
loadAsthmaTips();

function loadAsthmaTips() {
    if (($handle = fopen('./asthma.message.import.csv', "r")) !== FALSE) {
        while (($data = fgetcsv($handle, 1000, ",")) !== FALSE) {
            createTip($data);
        }
        fclose($handle);
    }
}

function createTip($content) {
    $term1 = taxonomy_get_term_by_name($content[0]);
    $term2 = taxonomy_get_term_by_name($content[1]);
    $tip = trim($content[2]);

    print "tip is: " .$content[0].",".$content[1].",".$content[2]."\n";
    print "      : " .$term1[0]->tid.",".$term2[0]->tid.",".$tip."\n";

    $terms = array(
            $term1[0]->tid,
            $term2[0]->tid,
    );

    $node = new stdClass();
    $node->type = 'quicktip';
    $node->title = $tip;
    $node->body = '';
    $node->teaser = '';
    $node->uid = 1;
    $node->language = 'en';
    $node->status = 0;
    $node->promote = 0;

    $node->field_ordinal = array( array( 'value' => $ordinal ) );
    $node->field_source = array( array( 'value' => $source ) );

    $node->taxonomy = $terms;

    node_save($node);
}


//
//loadDiabetes();
//loadDiabetesActivity();
//loadDiabetesNutrition();
//loadActivity();
//loadNutrition();
//
//function loadDiabetes() {
//    $term1 = taxonomy_get_term_by_name('Diabetes');
//    $terms = array(
//            $term1[0]->tid,
//            $term2[0]->tid,
//            $term3[0]->tid,
//            $term4[0]->tid
//    );
//    loadFile("./diabetes.qtips", 10000, $terms);
//}
//function loadDiabetesActivity() {
//    $term1 = taxonomy_get_term_by_name('Diabetes');
//    $term4 = taxonomy_get_term_by_name('Activity');
//    $terms = array(
//            $term1[0]->tid,
//            $term2[0]->tid,
//            $term3[0]->tid,
//            $term4[0]->tid
//    );
//    loadFile("./diabetes.activity.qtips", 10001, $terms);
//}
//function loadDiabetesNutrition() {
//    $term1 = taxonomy_get_term_by_name('Diabetes');
//    $term3 = taxonomy_get_term_by_name('Nutrition');
//    $terms = array(
//            $term1[0]->tid,
//            $term2[0]->tid,
//            $term3[0]->tid,
//            $term4[0]->tid
//    );
//    loadFile("./diabetes.nutrition.qtips", 10002, $terms);
//}
//function loadActivity() {
//    $term2 = taxonomy_get_term_by_name('Health');
//    $term4 = taxonomy_get_term_by_name('Activity');
//
//    $terms = array(
//            $term1[0]->tid,
//            $term2[0]->tid,
//            $term3[0]->tid,
//            $term4[0]->tid
//    );
//    loadFile("./activity.qtips", 10003, $terms);
//}
//function loadNutrition() {
//    $term2 = taxonomy_get_term_by_name('Health');
//    $term3 = taxonomy_get_term_by_name('Nutrition');
//    $terms = array(
//            $term1[0]->tid,
//            $term2[0]->tid,
//            $term3[0]->tid,
//            $term4[0]->tid
//    );
//    loadFile("./nutrition.qtips", 10004, $terms);
//}
//

?>
