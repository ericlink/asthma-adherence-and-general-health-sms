<?php
//require_once './includes/bootstrap.inc';
//drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
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
//function loadFile($file1, $ordinal, $terms) {
//    $lines = file($file1);
//    foreach($lines as $line_num => $line) {
//        createTip($line, $ordinal, $terms);
//        $ordinal = $ordinal + 5;
//    }
//}
//
//function createTip($line, $ordinal, $terms) {
//    $content = explode( "|", $line );
//    $tip = $content[0];
//    $source = $content[1];
//
//    $node = new stdClass();
//    $node->type = 'quicktip';
//    $node->title = $tip;
//    $node->body = '';
//    $node->teaser = '';
//    $node->uid = 1;
//    $node->language = 'en';
//    $node->status = 0;
//    $node->promote = 0;
//
//    $node->field_ordinal = array( array( 'value' => $ordinal ) );
//    $node->field_source = array( array( 'value' => $source ) );
//
//    $node->taxonomy = $terms;
//    node_save($node);
//}
?>
