<?php
// this src is written under the terms of the GPL-licence, see gpl.txt for futher details
unset($GLOBALS['settings']);

$settings['cookiedir']='/DOSBox/';
$settings['upload_images']='/';
$settings['sql_url']='localhost';
$settings['sql_username']='';
$settings['sql_password']='';
$settings['sql_db']='dosbox'; 
$settings['obsolete']='';

$settings['real_path']='http://dosbox.sourceforge.net';

$settings['melbdate']= date("Y-n-d_H-i-s",time() + $timeadjust);

$settings['online']=60*6;
$settings['lifetime']=3600*24*3;
?>
