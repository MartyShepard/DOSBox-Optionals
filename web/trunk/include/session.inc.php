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
	global $db;
	$sess_id=mysqli_escape_string($db, $sess_id);
	$query = mysqli_query($db,"SELECT user_session.string FROM user_session WHERE user_session.ID = '$sess_id'");
	
	switch(mysqli_affected_rows($db))
	{
		case -1:
			return false;
			break;
		case 0:
			return '';
			break;
		case 1:
			$stext = mysqli_fetch_row($query);
			return $stext[0];
	}
}
function sess_write($sess_id, $var){
	if(!isset($_SESSION["userID"])) return true; // nothing to do. leave early
	
	global $db;
	db_connect();
	$sess_id=mysqli_escape_string( $db ,$sess_id);
	$var=mysqli_escape_string( $db, $var);


	mysqli_query($db, "REPLACE INTO user_session (ID, lifetime, string, userID, last_active) VALUES ('$sess_id', UNIX_TIMESTAMP(), '$var',".$_SESSION["userID"].", NOW())");
	switch(mysqli_affected_rows($db))
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
	global $db;
	$sess_id = mysqli_escape_string( $db, $sess_id);
	$query = mysqli_query( $db, "DELETE FROM user_session WHERE user_session.ID = '$sess_id'");
	return true;
}
function sess_gc($max_lifetime)
{
	return true;
}
?>
