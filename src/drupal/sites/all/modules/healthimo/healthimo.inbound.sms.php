<?php 
  $data = file_get_contents("php://stdin");

file_put_contents(
'/var/www/healthimo.com/sites/default/files/postfixcatch.log',
""
.date("Y-m-d H:i:s")
."\t"
.$_GET['PhoneNumber']
."\t"
.$data
."\t"
.$_GET['Message']
."\n",
 FILE_APPEND
);

print $data;

print 'test test test';

?>
