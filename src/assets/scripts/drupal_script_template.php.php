<?php

//bootstrap drupal. set remote address to quiet errors during bootstrap
$_SERVER['REMOTE_ADDR'] = "127.1.1.1";
require_once './includes/bootstrap.inc';
drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);

$result = db_query(
        "SELECT * from zipnxx"
);

while ($row = db_fetch_object($result)) {
    echo $row->zip . ' ' . $row->nxx . "\n";
}
?>
