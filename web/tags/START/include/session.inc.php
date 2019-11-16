<?php
// this src is written under the terms of the GPL-licence, see gpl.txt for futher details
// ************************************************************************************
// SESSION HANDLING FUNCTIONS
// ************************************************************************************
session_set_save_handler("sess_open", "sess_close", "sess_read", "sess_write", "sess_destroy", "sess_gc");

function sess_open($save_path, $sess_name) {
	return true;
}
function sess_close() {
	return(true);
}
function sess_read($sess_id) {
	$sess_id=mysql_escape_string($sess_id);
	$query = mysql_query("SELECT user_session.string FROM user_session WHERE user_session.ID = '$sess_id'");
	
	switch(mysql_affected_rows())
	{
		case -1:
			return false;
			break;
		case 0:
			return '';
			break;
		case 1:
			$stext = mysql_fetch_row($query);
			return $stext[0];
	}
}
function sess_write($sess_id, $var){
	$sess_id=mysql_escape_string($sess_id);
	$var=mysql_escape_string($var);

	mysql_query("REPLACE INTO user_session (ID, lifetime, string, userID, last_active) VALUES ('$sess_id', UNIX_TIMESTAMP(), '$var',".$_SESSION["userID"].", NOW())");
	switch(mysql_affected_rows())
	{
		case -1:
			return false;
			break;
		case 1:
		case 2:
			return true;
			break;
	}
}
function sess_destroy($sess_id)
{
	$sess_id=mysql_escape_string($sess_id);
	$query = mysql_query("DELETE FROM user_session WHERE user_session.ID = '$sess_id'");
	return true;
}
function sess_gc($max_lifetime)
{
	return true;
}
?>
