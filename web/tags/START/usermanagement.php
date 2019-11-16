<?php
// this src is written under the terms of the GPL-licence, see gpl.txt for futher details
	include("include/standard.inc.php");
	sstart();

if ($user['priv']['user_management']==1)
{
	
	if ($_GET['obsolute_delID'])
	{
		$ID = mysql_escape_string(stripslashes($_GET['obsolute_delID']));
		
		
		$query = mysql_query("SELECT ID FROM list_game WHERE ownerID=$ID");
		while($result = mysql_fetch_row($query))
		{
			mysql_query("DELETE FROM status_games WHERE gameID = ".$result[0]);
		}
		
		mysql_query("DELETE FROM list_game WHERE ownerID = $ID");
		mysql_query("DELETE FROM user_session WHERE userID = $ID");
		mysql_query("DELETE FROM userdb WHERE userdb.ID = $ID");
		mysql_query("DELETE FROM list_comment WHERE ownerID = $ID");		
		mysql_query("DELETE FROM news WHERE ownerID = $ID");
		
		
		Header("Location: usermanagement.php?show=1");
	}
	if ($_GET['delID'])
	{
		$ID = mysql_escape_string(stripslashes($_GET['delID']));
		
		mysql_query("UPDATE userdb SET active=0, chg_passwd='' WHERE userdb.ID = $ID");
		Header("Location: usermanagement.php?show=1");
	}
	
	if ($_GET['reactivateID'])
	{
		$ID = mysql_escape_string(stripslashes($_GET['reactivateID']));
		/*
			$reactivateID 	= mysql_escape_string(stripslashes($_GET['reactivateID']));
			$temp_passwd	= md5(rand(0, 1999999999999999));
			mysql_query("UPDATE userdb SET chg_passwd='$temp_passwd', active=0 WHERE ID=$ID");		
			send_activate_email($temp_passwd); 
		*/
					mysql_query("UPDATE userdb SET active=1 WHERE ID=$ID");
		Header("Location: usermanagement.php?show=1");
	}
	if ($_GET['updatingID'])
	{	
		$ID = mysql_escape_string(stripslashes($_GET['updatingID']));
		
		$nickname 	= mysql_escape_string(stripslashes($_POST['nickname']));
		$name 		= mysql_escape_string(stripslashes($_POST['name']));
		$email 		= mysql_escape_string(stripslashes($_POST['email']));
		$website 	= mysql_escape_string(stripslashes($_POST['website']));
		$admin 		= mysql_escape_string(stripslashes($_POST['usergroup']));
		$crew		= mysql_escape_string(stripslashes($_POST['crew']));
		$active		= mysql_escape_string(stripslashes($_POST['active']));
		$temp_passwd	= mysql_escape_string(stripslashes($_POST['temp_passwd']));
		
	
		if ($active == 1)
			mysql_query("UPDATE userdb SET nickname='$nickname', name='$name', email='$email', website='$website', grpID=$admin, active=1, chg_passwd='',part_of_crew=$crew WHERE userdb.ID=$ID");
		else
			mysql_query("UPDATE userdb SET nickname='$nickname', name='$name', email='$email', website='$website', admin=$admin, active=0, chg_passwd='$temp_passwd',part_of_crew=$crew WHERE userdb.ID=$ID");
		
		Header("Location: usermanagement.php?show=1");
	}
	

	template_header();
	echo '<br><table width="100%"><tr><td width="14">&nbsp;</td><td>';// start of framespacing-table		
	
	template_pagebox_start("Register account", 980);			
	
	echo '<font face="Verdana, Arial, Helvetica, sans-serif" size="2"><br>';

		if ($_GET['show']==1)
		{
			show_users(999999);			
		}
		if ($_GET['editID'])
		{
		
			$editID=mysql_escape_string(stripslashes($_GET['editID']));

			$query = mysql_query("
			
			SELECT 
				userdb.ID, userdb.name, userdb.nickname, userdb.email, userdb.website,
				userdb.active, userdb.chg_passwd, usergrp.groupname, usergrp.ID, userdb.part_of_crew
			FROM
				userdb, usergrp
			WHERE
				userdb.ID = $editID AND userdb.grpID=usergrp.ID");
			
			$result = mysql_fetch_row($query);
			
			
			
			if ($result[5] == 0 AND $result[6] == '')
				echo 'The user "<font face="Verdana, Arial, Helvetica, sans-serif" size="2">'.$result[2].'</b>" has been removed. <a href="usermanagement.php?reactivateID='.$result[0].'" target="_top">Click here</a> to activate him/her again!<br><br>';
			
			echo '<a href="usermanagement.php?show=1" target="_top">Click here</a> to get back to the user-list again!';
			
			echo '
				<form action="usermanagement.php?updatingID='.$result[0].'" method="POST"><input type="hidden" name="temp_passwd" value="'.$result[7].'">
				<table cellspacing="0" cellpadding="0">
				<tr valign="top"> 
					<td>
						<font face="Verdana, Arial, Helvetica, sans-serif" size="2">Nickname:</b>
					</td>
					<td width="15">
						&nbsp;
					</td>
					<td>
						<font face="Verdana, Arial, Helvetica, sans-serif" size="2">Website:</b>
					</td>
				</tr>
				<tr valign="top"> 
					<td>
						<input name="nickname" type="text" size="25"  maxlength="25" value="'.$result[2].'">
					</td>
					<td width="5">
						&nbsp;
					</td>
					<td>
						<input name="website" type="text" size="45" maxlength="60" value="'.$result[4].'">
						
					</td>
				</tr>
				<tr valign="top"> 
					<td>
						<font face="Verdana, Arial, Helvetica, sans-serif" size="2">Name:</b>
					</td>
					<td width="15">
						&nbsp;
					</td>
					<td>
						<font face="Verdana, Arial, Helvetica, sans-serif" size="2">Email:</b>
					</td>
				</tr>
				<tr valign="top"> 
					<td>
						<input name="name" type="text" size="25" maxlength="60" value="'.$result[1].'">
					</td>
					<td width="5">
						&nbsp;
					</td>
					<td>
						<input name="email" type="text" size="45" maxlength="50" value="'.$result[3].'">
					</td>
				</tr>
				</table><br>	
				<table cellspacing="0" cellpadding="0">
				<tr valign="top"> 
					<td>
						<font face="Verdana, Arial, Helvetica, sans-serif" size="2">Status:</b>
					</td>
					<td width="15">
						&nbsp;
					</td>
					<td>
						<font face="Verdana, Arial, Helvetica, sans-serif" size="2">Priviliges:</b>
					</td>
					<td width="15">
						&nbsp;
					</td>
					<td>
						<font face="Verdana, Arial, Helvetica, sans-serif" size="2">Part of crew:</b>
					</td>									
				</tr>
				<tr valign="top"> 
					<td>
						<select name="active">

						<option value="1" '; if ($result[5] == 1) echo 'selected'; echo '>Active</option>
						<option value="0" '; if ($result[5] == 0) echo 'selected'; echo '>Not active</option>
	
						</select>
					</td>
					<td>
						&nbsp;
					</td>
					<td>';
						
				get_usergroups($result[8]);
					echo '</td>
					<td>
						&nbsp;
					</td>
					<td>
						<select name="crew">

						<option value="1" '; if ($result[9] == 1) echo 'selected'; echo '>Include in memberlist</option>
						<option value="0" '; if ($result[9] == 0) echo 'selected'; echo '>Not included in memberlist</option>
	
						</select>
					</td>					
				
				</tr></table><br>
				<input type="submit" name="submit" value="Update this user">
				</form>';			
		}
}

else
	template_header($page);
	
	
	template_pagebox_end();	
	echo '</td></tr></table>';	// end of framespacing-table					
	template_end();

#######################################################################################
# Some functions that are only used in this PHP file.
#######################################################################################

function show_users($limit)
{
	$query = mysql_query("
	SELECT
		userdb.ID, userdb.name, userdb.nickname, userdb.email, DATE_FORMAT(userdb.added, '%Y-%m-%d'),
		userdb.active, userdb.chg_passwd, usergrp.groupname
	FROM
		userdb, usergrp
	WHERE
		userdb.grpID = usergrp.ID
	ORDER BY
		userdb.added DESC
	LIMIT 
		$limit
	");
	
	echo '<table cellspacing="0" cellpadding="0" width="100%">
	<tr valign="top">
		<td>
			<font face="Verdana, Arial, Helvetica, sans-serif" size="2"><b>Fullname:</b>
		</td>
		<td>
			<font face="Verdana, Arial, Helvetica, sans-serif" size="2"><b>Nickname:</b>
		</td>
		<td>
			<font face="Verdana, Arial, Helvetica, sans-serif" size="2"><b>E-mail:</b>
		</td>
		<td>
			<font face="Verdana, Arial, Helvetica, sans-serif" size="2"><b>Added:</b>
		</td>
		<td>
			<font face="Verdana, Arial, Helvetica, sans-serif" size="2"><b>Priviliges:</b>
		</td>
		<td>
			<font face="Verdana, Arial, Helvetica, sans-serif" size="2"><b>Status:</b>
		</td>
		<td>
			&nbsp;
		</td>		
	</tr>';
	
	while ($result = mysql_fetch_row($query))
	{
		echo '
		<tr valign="top">
			<td><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
				'.$result[1].'</font>
			</td>
			<td><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
				'.$result[2].'</font>
			</td>
			<td><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
				'.$result[3].'</font>
			</td>
			<td><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
				'.$result[4].'</font>
			</td>
			<td><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
				'.$result[7].'</font>
			</td>
			<td><font face="Verdana, Arial, Helvetica, sans-serif" size="2">';
	
		if ($result[5] == 0 && $result[6] != '')
			echo 'Not activated';
		elseif ($result[5] == 0 && $result[6] == '')
			echo 'Removed';
		else
			echo 'Activated';
			
			echo '</font></td>
	
			<td><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
				<a href="usermanagement.php?editID='.$result[0].'"><img src="site_images/change_icon.gif" border="0" alt="Edit this user"></a>&nbsp;';
				
				if ($_SESSION['userID'] != $result[0])
					echo '<a href="usermanagement.php?delID='.$result[0].'"><img src="site_images/delete_icon.gif" border="0" alt="Deactivate this user (the user will not be able to activate the account again)"></a>&nbsp;<a href="usermanagement.php?obsolute_delID='.$result[0].'"><img src="site_images/obsolute_delete_icon.gif" border="0" alt="Remove this user from database (all database-posts)"></a>';
			echo '</font></td>
		</tr>
		';	
	}
	echo '</table>';
	
}
?>
