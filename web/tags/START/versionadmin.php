<?php
// this src is written under the terms of the GPL-licence, see gpl.txt for futher details
include("include/standard.inc.php");
sstart();

if ($user['priv']['status_manage']==1)
{
	if ($_GET['createversion']==1)
	{
		if ($_POST['version'] != '')
		{
			$version = mysql_escape_string(stripslashes($_POST['version']));
			mysql_query("INSERT INTO versions (version) VALUES ('$version')");
			Header("Location: versionadmin.php");
		}
		else
			Header("Location: versionadmin.php?add_empty=1");
	}
	if ($_GET['changeversion']==1)
	{
		$version	= mysql_escape_string(stripslashes($_POST['version']));
		$versionID 	= mysql_escape_string(stripslashes($_POST['versionID']));
		
		if ($version != '')
		{
			mysql_query("UPDATE versions SET version='$version' WHERE versions.ID=$versionID");
			Header("Location: versionadmin.php");
		}
		else
			Header("Location: versionadmin.php?change_empty=1");
	}
	if (isset($_GET['removeID']))
	{
		$removeID = mysql_escape_string(stripslashes($_GET['removeID']));
		$query = mysql_query("SELECT COUNT(ID) FROM status_games WHERE versionID=$removeID");
		$count = mysql_fetch_row($query);
		if ($count[0] != 0)
			Header("Location: versionadmin.php?existDB=1");
		else
			mysql_query("DELETE FROM versions WHERE versions.ID=$removeID");
	}



	template_header();
	echo '<br><table width="100%"><tr><td width="14">&nbsp;</td><td>';// start of framespacing-table		
	
	template_pagebox_start("Version management");
	
	echo '<font face="Verdana, Arial, Helvetica, sans-serif" size="2"><br>';
	
	if (isset($_GET['existDB']))
		echo 'This version is "linked" against games in the compatibility page, please remove/change those games first!<br><br>';
	if (isset($_GET['add_empty']))
		echo 'You cannot add a version without a version name/number, use your brain! :D<br><br>';
	if (isset($_GET['change_empty']))
		echo 'You must specify a version name/number brain-iac! :P<br><br>';

	
	echo '<table cellspacing="0" cellpadding="0"><form action="versionadmin.php?createversion=1" method="POST" name="createversion"><tr><td><input type="text" name="version"></td><td>&nbsp;<input type="submit" value="Add version"></td></tr></form></table><br>';

        $query = mysql_query("SELECT ID, version FROM versions ORDER BY versions.version DESC");
	
	echo '<table cellspacing="0" cellpadding="0">';
        while ($result=mysql_fetch_row($query))
	{
		echo '<form action="versionadmin.php?changeversion=1" method="POST" name="changeversion"><tr><td><input type="text" name="version" value="'.$result[1].'"><input type="hidden" name="versionID" value="'.$result[0].'"></td><td>&nbsp;<input type="submit" value="Update version">&nbsp;&nbsp;<a href="versionadmin.php?removeID='.$result[0].'"><img src="site_images/msgboard_remove.gif" border="0" alt="Remove this version from database!"></a></td></tr></form>';
	}
        echo '</table><br>
        <font face="Verdana, Arial, Helvetica, sans-serif" size="2"><a href="login.php">Click here</a> to get back!<br><br>';
	template_pagebox_end();	
	echo '</td></tr></table>';	// end of framespacing-table					
	template_end();		
}



?>
