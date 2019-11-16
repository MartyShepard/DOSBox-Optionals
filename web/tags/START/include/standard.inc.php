<?php
// this src is written under the terms of the GPL-licence, see gpl.txt for futher details
include "include/config.inc.php";
include "include/session.inc.php";
include "include/function.inc.php";
include "include/template.inc.php";
db_connect();

# erase sessions that are out of date
mysql_query("DELETE FROM user_session WHERE lifetime < ".(time()-$settings['lifetime'])); 


?>
