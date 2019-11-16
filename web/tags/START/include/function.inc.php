<?php
// this src is written under the terms of the GPL-licence, see gpl.txt for futher details
// Starting session
//******************************************************************************************************
function sstart()
{
	global $settings;

	session_set_cookie_params(($settings['lifetime']),$settings['cookiedir']);
	session_name("begaming_website_session");

	session_start();


	// If user-session found --> load user-vars
	//*********************************************************************************************
	if (isset($_SESSION["userID"]))
	loaduser($_SESSION["userID"]);

}
// MySQL connection
//******************************************************************************************************
function db_connect()
{
	global $db, $settings;
	$db = mysql_connect($settings['sql_url'], $settings['sql_username'], $settings['sql_password']);
	if (!$db) error_msg("Cannot connect to database server",$db);
	$result=mysql_select_db($settings['sql_db']);
	if (!$result) error_msg("Cannot connect to database mongo",$db);
}

// Logging in user
//******************************************************************************************************
function login($usr, $pwd)
{
	$username=mysql_escape_string(stripslashes($usr));
	$password=mysql_escape_string(stripslashes(md5($pwd)));

	$query = mysql_query("
	SELECT
	ID, nickname, password
	FROM
	userdb
	WHERE
	nickname = '$username' AND password = '$password' AND active = 1
	");

	$result = mysql_fetch_row($query);

	if (mysql_num_rows($query) != 0)
	{
		$_SESSION["userID"]=(int)$result[0];
		header("Location: login.php");
	}
	else
	header("Location: login.php?failed=1");

}
// Setting user-variables
//******************************************************************************************************
function loaduser($userID)
{
	unset($GLOBALS['user']);
	global $user;


	$query = mysql_query("
	SELECT
	ID,grpID,name,nickname,email
	FROM
	userdb
	WHERE
	ID = $userID
	");

	$result = mysql_fetch_row($query);
	if (mysql_num_rows($query)==1)
	{
		$user['ID']			=$result[0];
		$user['groupID']		=$result[1];
		$user['name']			=$result[2];
		$user['nickname']		=$result[3];

		$user['mail']			=$result[4];


		$query = mysql_query("SELECT * FROM usergrp WHERE ID=$result[1]");
		if (mysql_num_rows($query)==1)
		{
			$user['priv']=mysql_fetch_assoc($query);

		}
		else
		unset($GLOBALS['user']);
	}


}
// Debugging function
//******************************************************************************************************
function dbg_o()
{
	global $user;
	ob_start();
	echo "\nUSER: ";
	print_r($user);
	echo "\nGET: ";
	print_r($_GET);
	echo "\nPOST: ";
	print_r($_POST);
	echo "\nCOOKIE: ";
	print_r($_COOKIE);
	echo "\nSESSION: ";
	print_r($_SESSION);
	echo "\n";

	$temp=nl2br(htmlspecialchars(ob_get_contents()));
	ob_end_clean();
	echo $temp;
}
// Checking if $username already exists in database
//******************************************************************************************************
function check_dublicate_username($username)
{
	$query = mysql_query("SELECT COUNT(ID) FROM userdb WHERE userdb.nickname = '$username'");
	$result = mysql_fetch_row($query);

	if ($result[0] == 1)
	return 1;
	else
	return 0;
}
// Get username from $userID
//******************************************************************************************************
function get_name_from_id($userID)
{
	$query = mysql_query("SELECT nickname FROM userdb WHERE userdb.ID = $userID");
	$result = mysql_fetch_row($query);
	return $result[0];

}
function check_if_owner($newsID,$userID=NULL)
{
	$newsID = mysql_escape_string(stripslashes($newsID));
	if ($newsID != NULL)
	{
		$query = mysql_query("SELECT COUNT(*) FROM news WHERE news.ID = $newsID AND news.ownerID = $userID");
		$result = mysql_fetch_row($query);
		return $result[0];
	}
	else
	return 0;
}
function get_version_num($version = 0)
{

	$query = mysql_query("SELECT version FROM download WHERE download.catID=1 ORDER BY version DESC");

	if ($version == 0)
	{
		echo '
		<select name="version">';
		while ($result = mysql_fetch_row($query))
		echo '<option value="'.$result[0].'">DOSBox '.$result[0].'</option>';
		echo '</select>';
	}
	else
	{
		echo '<select name="version">';

		while ($result = mysql_fetch_row($query))
		{
			echo '<option value="'.$result[0].'"';

			if($version == $result[0])
			echo ' selected';

			echo '>DOSBox '.$result[0].'</option>';
		}

		echo '</select>';
	}

}
function check_game_owner($gameID, $userID)
{
	$gameID = mysql_escape_string(stripslashes($gameID));
	$userID = mysql_escape_string(stripslashes($_SESSION['userID']));

	$query = mysql_query("SELECT COUNT(*) FROM list_game WHERE list_game.ID=$gameID AND list_game.ownerID=$userID");

	$result = mysql_fetch_row($query);

	return $result[0];

}
function main_news($priv)
{

	$query = mysql_query("

	SELECT
	news.text, DATE_FORMAT(news.added, '%W, %M %D, %Y'), userdb.nickname, news.ownerID, news.ID
	FROM
	news, userdb
	WHERE
	userdb.ID=news.ownerID
	ORDER BY
	news.added DESC
	");

	while ($result = mysql_fetch_row($query))
	{
		$text = ereg_replace ("\n", "<br>", $result[0]);
		$text = parse_http($text);

		echo '<table width="730" cellspacing="0" cellpadding="1" bgcolor="#000000"><tr><td valign="top" align="left"><table cellspacing="4" cellpadding="0" width="100%" bgcolor="#355787"><tr><td>
		<table cellspacing="0" cellpadding="0" width="100%"><tr><td valign="top" align="left"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">';

		if ($priv==1 || $result[3]==$_SESSION['userID'])
		echo '<p><b>'.$result[1].'</b> - '.$result[2].'&nbsp;&nbsp;<a href="news.php?change_news=1&amp;newsID='.$result[4].'"><img src="site_images/change_icon.gif" border="0"></a>&nbsp;<a href="news.php?removing_news=1&newsID='.$result[4].'"><img src="site_images/delete_icon.gif" border="0"></a></p>';
		else
		echo '<p><b>'.$result[1].'</b> - '.$result[2].'</p>';

		echo '</font></td></tr></table></td>
		</tr></table></td></tr></table><table width="730" cellspacing="0" cellpadding="1" bgcolor="#000000"><tr><td valign="top" align="left"><table cellspacing="4" cellpadding="0" width="100%" bgcolor="#113466"><tr>
		<td><table cellspacing="0" cellpadding="0" width="100%"><tr><td valign="top" align="left"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">';





		echo $text;

		echo '</font></td></tr></table></td></tr></table></td></tr></table><br>';
	}


}
// Check if $email is a valid email adress
//******************************************************************************************************
function verify_mail($email)
{
	if(eregi("^[_a-z0-9-]+(.[_a-z0-9-]+)*@[_a-z0-9-]+(.[_a-z0-9-]+)*(.([a-z]{2,3}))+$",$email))
	return 1;
	else
	return 0;
}
// Check if $urladdr is a valid www url
//******************************************************************************************************
function verifyurl( $urladdr )
{
	$regexp = "^(https?://)?(([0-9a-z_!~*'().&=+$%-]+:)?[0-9a-z_!~*'().&=+$%-]+@)?(([0-9]{1,3}\.){3}[0-9]{1,3}|([0-9a-z_!~*'()-]+\.)*([0-9a-z][0-9a-z-]{0,61})?[0-9a-z]\.[a-z]{2,3})(:[0-9]{1,4})?((/?)|(/[0-9a-z_!~*'().;?:@&=+$,%#-]+)+/?)$";

	if (eregi( $regexp, $urladdr ))
	{
		if (!eregi( "^https?://", $urladdr ))
		$urladdr = "http://" . $urladdr;

		if (!eregi( "^https?://.+/", $urladdr ))
		$urladdr .= "/";

		if ((eregi( "/[0-9a-z~_-]+$", $urladdr)) && (!eregi( "[\?;&=+\$,#]", $urladdr))) $urladdr .= "/";
		return ($urladdr);
	}
	else
	return false;

}
// Check if the $email allready exists in database
//******************************************************************************************************
function check_mail_db($email)
{
	$mail 	= mysql_escape_string(stripslashes($email));
	$query 	= mysql_query("SELECT COUNT(*) FROM userdb WHERE userdb.email = '$mail'");
	$result = mysql_fetch_row($query);

	return $result[0];
}
// Convert mail/http/https/ftp urls to <a href> tags.
//******************************************************************************************************
function parse_http($str, $target = '_blank')
{
	$str = preg_replace ("/(https?:\/\/|ftp:\/\/|mailto:)([^>\s\"\']+)/i", "<a href='\\0' target='$target'>\\2</a>", $str);
	return $str;
}
// Displays all the usergroups in a <select> field!
//******************************************************************************************************
function get_usergroups($selected=NULL)
{
	$query=mysql_query("SELECT usergrp.ID,usergrp.groupname FROM usergrp ORDER BY usergrp.ID");
	echo '<select name="usergroup">';
	while ($result=mysql_fetch_row($query))
	{
		echo '<option value="'.$result[0].'"';

		if ($selected == $result[0])
		echo ' selected';

		echo '>'.$result[1].'</option>';
	}
	echo '</select>';
}
function status_change($changeID)
{
	$catID = mysql_escape_string(stripslashes($changeID));

	$query = mysql_query("

	SELECT
	status_items.name, status_items.percent, status_items.note, status_items.ID
	FROM
	status_items
	WHERE
	status_items.catID=$catID

	");
	echo '<table cellspacing="0" cellpadding="0">
	<tr valign="top" align="left">
	<td width="180">
	<font face="Verdana, Arial, Helvetica, sans-serif" size="2">Name:
	</td>

	<td width="5">
	&nbsp;
	</td>

	<td width="60"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">Status:
	</td>
	<td width="5">
	&nbsp;
	</td>
	<td width="100"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">Note:
	</td>
	<td>
	&nbsp;
	</td>
	<td>
	</td>
	</tr>';

	while ($result = mysql_fetch_row($query))
	{


		echo '<form action="status.php?changing=1&catID='.$changeID.'" method="POST" name="changing_status"><input type="hidden" name="updateID" value="'.$result[3].'"><tr><td>
		<input type="text" value="'.$result[0].'" name="name" maxlength="40" size="25">
		</td>

		<td>
		&nbsp;
		</td>

		<td>
		<input type="text" value="'.$result[1].'" name="status" maxlength="3" size="5">
		</td>
		<td>
		&nbsp;
		</td>

		<td>
		<input type="text" value="'.$result[2].'" name="note" maxlength="150" size="70">
		</td>
		<td>
		&nbsp;&nbsp;<input type="submit" name="submit" value="Update">
		</td>
		<td>
		&nbsp;&nbsp;<a href="status.php?removeID='.$result[3].'&catID='.$changeID.'"><img src="site_images/delete_icon.gif" border="0"></a>
		</td>
		</tr></form>
		';
	}

	echo '</table>';
}

function show_status_db($priv)
{
	$query = mysql_query("SELECT ID, name FROM status_cat");

	while($result=mysql_fetch_row($query))
	{
		if ($priv==1)
		$text = $result[1].'&nbsp;<a href="status.php?changeID='.$result[0].'" target="_top"><img src="site_images/change_icon.gif" border="0" alt="Change these item(s)"></a>';
		else
		$text = $result[1];


		template_pagebox_start($text, 890);
		echo '
		<table cellspacing="0" cellpadding="0" width="100%">';

		$item_query = mysql_query("SELECT name,percent,note FROM status_items WHERE catID=".$result[0]);
		while($item_result=mysql_fetch_row($item_query))
		{
			echo '
			<tr valign="top" align="left">
			<td width="125">
			<font face="Verdana, Arial, Helvetica, sans-serif" size="2">'.$item_result[0].'
			</font>
						</td>

			<td width="43">
			<font face="Verdana, Arial, Helvetica, sans-serif" size="2">'.$item_result[1].'%
			</font>
						</td>

			<td width="72" valign="middle">';
			if ($item_result[1] !=0 )
			echo '<img src="site_images/progress.gif" border="1" width="'.$item_result[1].'%" alt="'.$item_result[1].'% implemented" height="8">';
			else
			echo '&nbsp';
			echo '</td>

			<td width="10">
			&nbsp;
			</td>

			<td valign="middle">
			<font face="Verdana, Arial, Helvetica, sans-serif" size="2">'.$item_result[2].'
			</font>
						</td>
			</tr>';
		}
		echo '</table>';
		template_pagebox_end();
	}


}
function download_change($changeID)
{
	$catID = mysql_escape_string(stripslashes($changeID));

	$query = mysql_query("

	SELECT
	name, url, description, version, changelog, ID
	FROM
	download
	WHERE
	download.catID=$catID

	");
	echo '<table cellspacing="0" cellpadding="0">
	<tr valign="top" align="left">
	<td width="180">
	<font face="Verdana, Arial, Helvetica, sans-serif" size="2">Name:
	</td>

	<td width="5">
	&nbsp;
	</td>

	<td width="60"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">Version:
	</td>
	<td width="5">
	&nbsp;
	</td>
	<td width="60"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">Description:
	</td>
	<td width="5">
	&nbsp;
	</td>
	<td width="100"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">URL:
	</td>
	<td>
	&nbsp;
	</td>
	<td>
	</td>
	</tr>';

	while ($result = mysql_fetch_row($query))
	{


		echo '<form action="download.php?changing=1&catID='.$changeID.'" method="POST" name="changing_status"><input type="hidden" name="updateID" value="'.$result[5].'"><tr><td>
		<input type="text" value="'.$result[0].'" name="name" maxlength="40" size="25">
		</td>

		<td>
		&nbsp;
		</td>

		<td>
		<input type="text" value="'.$result[3].'" name="version" maxlength="20" size="5">
		</td>
		<td>
		&nbsp;
		</td>
		<td>
		<input type="text" value="'.$result[2].'" name="description" maxlength="50" size="25">
		</td>
		<td>
		&nbsp;
		</td>
		<td>
		<input type="text" value="'.$result[1].'" name="url" maxlength="150" size="40">
		</td>
		<td>
		&nbsp;&nbsp;<input type="submit" name="submit" value="Update">
		</td>
		<td>
		&nbsp;&nbsp;<a href="download.php?removeID='.$result[5].'&catID='.$changeID.'"><img src="site_images/delete_icon.gif" border="0"></a>
		</td>
		</tr></form>
		';
	}

	echo '</table>';
}
function show_downloads($priv)
{
	$cat_query = mysql_query("SELECT ID, name FROM download_cat");


	while($cat_result=mysql_fetch_row($cat_query))
	{

		if ($priv==1)
		$cat=$cat_result[1].'&nbsp;<a href="download.php?changeID='.$cat_result[0].'" target="_top"><img src="site_images/change_icon.gif" border="0" alt="Change these item(s)"></a>';
		else
		$cat=$cat_result[1];

		;

		template_pagebox_start($cat, 890);



		$query = mysql_query("SELECT ID, name, url, description, version, DATE_FORMAT(added, '%W, %M %D, %Y'), changelog FROM download WHERE catID=".$cat_result[0]." ORDER BY added DESC");
		if (mysql_num_rows($query) != 0)
		{
			echo '<table cellspacing="0" cellpadding="0" width="100%">';
			while($result=mysql_fetch_row($query))
			{
				echo '
				<tr valign="top" align="left">
				<td width="210">
				<font face="Verdana, Arial, Helvetica, sans-serif" size="2"><a href="'.$result[2].'" target="_blank">'.$result[1].'</a>
				</td>

				<td width="110">
				<font face="Verdana, Arial, Helvetica, sans-serif" size="2">version '.$result[4].'
				</td>
				<td>
				<font face="Verdana, Arial, Helvetica, sans-serif" size="2">'.$result[3].'
				</td>
				<td width="226">
				<font face="Verdana, Arial, Helvetica, sans-serif" size="2">'.$result[5].'
				</td>
				</tr>';
			}
			echo '</table>';
		}
		else
		echo '<font face="Verdana, Arial, Helvetica, sans-serif" size="2"><i>No items available for download in this category</i></font>';
		template_pagebox_end();

	}



}
function get_latest_version()
{
	$query = mysql_query("SELECT version FROM versions ORDER BY version DESC");
	$result = mysql_fetch_row($query);

	return $result[0];
}
function get_versions()
{
	$query = mysql_query("SELECT ID, version FROM versions ORDER BY version DESC");
	echo '<select name="version">';
	while ($result = mysql_fetch_row($query))
	{
		echo '<option value="'.$result[0].'">DOSBox '.$result[1].'</option>';
	}
	echo '</select>';
}
function change_version_compatibility_form($gameID)
{
	global $user;


	$query = mysql_query("SELECT status_games.ID, status_games.status, versions.version, versions.ID FROM versions,status_games WHERE status_games.versionID=versions.ID AND status_games.gameID=$gameID ORDER BY version DESC");
	$num = mysql_num_rows($query);

	while ($result = mysql_fetch_row($query))
	{
		echo '
		<font face="Verdana, Arial, Helvetica, sans-serif" size="2">

		<form name="versionchange" method="POST" action="comp_list.php?changeversion=1"><input type="hidden" name="letter" value="'.$_GET['letter'].'"><input type="hidden" name="gameID" value="'.$_GET['changeID'].'">
		<input type="hidden" name="statusID" value="'.$result[0].'">


		<input type="submit" value="Update this item (DOSBox v.'.$result[2].')">';



		echo '<select name="percent">';

		for ($i = 0; $i <= 100; $i++)
		{
			echo '<option value="'.$i.'"'; if ($i == $result[1]) echo ' selected'; echo '>'.$i.'% (game ';
			echo return_status($i);
			echo ')</option>';
		}

		echo '</select>&nbsp;&nbsp;';
		if ($num != 1 AND $user['priv']['compat_list_manage']==1)
		echo '<a href="comp_list.php?removeVERSION_ID='.$result[0].'&gameID='.$_GET['changeID'].'&letter='.$_GET['letter'].'"><img src="site_images/delete_icon.gif" border="0"></a>';
		echo '</form>';
	}

}
function add_version_compatibility_form($gameID)
{
	$query = mysql_query("SELECT versions.ID, versions.version FROM versions ORDER BY versions.version DESC");

	echo '<select name="versionID">
	<option>-</option>';

	while($result=mysql_fetch_row($query))
	{
		$q=mysql_query("

		SELECT
		COUNT(*)
		FROM
		status_games
		WHERE
		status_games.versionID=$result[0] AND status_games.gameID=$gameID
		");

		$a=mysql_fetch_row($q);


		if ($a[0] == 0)
		{
			echo '<option value="'.$result[0].'">version '.$result[1].'</option>';
		}
	}
	echo '</select>';
}

function choose_percentage()
{
	echo '<select name="percent">';

	for ($i = 0; $i <= 100; $i++)
	{
		echo '<option value="'.$i.'">'.$i.'% (game ';
		echo return_status($i);
		echo ')</option>';
	}

	echo '</select>';
}
function compat_list_latest()
{
	echo '<b>Latest Added:</b><br>';
	$query = mysql_query("SELECT ID, name, version, released FROM list_game ORDER BY added DESC LIMIT 10");
	while ($result = mysql_fetch_row($query))
	{
		$q = mysql_query("SELECT status_games.status FROM status_games WHERE status_games.gameID=$result[0] ORDER BY status_games.status DESC");
		$a = mysql_fetch_row($q);

		echo '<option value="'.$result[0].'"';

		if ($result[0]==$_GET['showID'])
		echo ' selected';

		echo '> (';

		echo return_status($a[0]);

		echo ') '.$result[1].''; if ($result[2] != 0) echo ' ('.$result[3].')'; echo '</option>';
	}
}
function comp_mainlist($letter)
{

	echo '<table width="730" cellspacing="0" cellpadding="1" bgcolor="#000000"><tr><td valign="top" align="left"><table cellspacing="4" cellpadding="0" width="100%" bgcolor="#355787"><tr><td>
	<table cellspacing="0" cellpadding="0" width="100%"><tr><td valign="top" align="left"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">Game directory (browsing from <b>'; if($_GET['letter']==num) echo 'numerical'; else echo $_GET['letter']; echo '</b>)</td></tr></table></td>
	</tr></table></td></tr></table><table width="730" cellspacing="0" cellpadding="1" bgcolor="#000000"><tr><td valign="top" align="left"><table cellspacing="4" cellpadding="0" width="100%" bgcolor="#113466"><tr>
	<td><table cellspacing="0" cellpadding="0" width="100%"><tr><td valign="top" align="left">';


	$letter = mysql_escape_string(stripslashes($letter));

	if ($letter == 'num')
	$query = mysql_query("
	SELECT
	list_game.ID, list_game.name, list_game.released, list_game.version
	FROM
	list_game
	WHERE
	(list_game.first_char='1' OR list_game.first_char='2' OR list_game.first_char='3' OR list_game.first_char='4' OR list_game.first_char='5' OR list_game.first_char='6' OR list_game.first_char='7' OR list_game.first_char='8' OR list_game.first_char='9' OR list_game.first_char='0')
	ORDER BY
	list_game.name");
	else
	$query = mysql_query("
	SELECT
	list_game.ID, list_game.name, list_game.released, list_game.version
	FROM
	list_game
	WHERE
	list_game.first_char='$letter'
	ORDER BY
	list_game.name");
	if (mysql_num_rows($query) == 0)
	echo '<table cellspacing="0" cellpadding="0" width="100%"><tr><td><font face="Verdana, Arial, Helvetica, sans-serif" size="2">No games with the first letter "'.$letter.'" was found in the database!</td></tr></table>';
	else
	{


		echo '<table cellspacing="0" cellpadding="0" width="720">
		<tr>
		<td width="385">
		<font face="Verdana, Arial, Helvetica, sans-serif" size="2"><b>Game:</b></font>
		</td>

		<td width="6">
		&nbsp;
		</td>

		<td width="70">
		<font face="Verdana, Arial, Helvetica, sans-serif" size="2"><b>Version:</b></font>
		</td>

		<td width="6">
		&nbsp;
		</td>

		<td width="70">
		<font face="Verdana, Arial, Helvetica, sans-serif" size="2"><b>Status:</b></font>
		</td>


		<td width="6">
		&nbsp;
		</td>


		<td width="275">
		<font face="Verdana, Arial, Helvetica, sans-serif" size="2"><b>runable&nbsp;&nbsp;-&nbsp;&nbsp;playable&nbsp;&nbsp;-&nbsp;&nbsp;supported</b></font>
		</td>
		</tr>';

		while($result=mysql_fetch_row($query))
		{

			$status_query = mysql_query("SELECT status_games.status, versions.version FROM versions, status_games WHERE status_games.gameID=".$result[0]." AND status_games.versionID=versions.ID ORDER BY status_games.status DESC");
			$status = mysql_fetch_row($status_query);
			$percent_text = return_status($status[0]);


			echo '<tr>
			<td>
			<font face="Verdana, Arial, Helvetica, sans-serif" size="2"><a href="comp_list.php?showID='.$result[0].'&amp;letter='.$_GET['letter'].'">'.$result[1].'</a>'; if ($result[2] != 0) echo ' ('.$result[2].')'; echo '</font>
			</td>

			<td>
			&nbsp;
			</td>

			<td>
			<font face="Verdana, Arial, Helvetica, sans-serif" size="2">'.$status[1].'</font>
			</td>

			<td>
			&nbsp;
			</td>

			<td>
			<font face="Verdana, Arial, Helvetica, sans-serif" size="2">'.$percent_text.'</font>
			</td>

			<td>
			&nbsp;
			</td>


			<td>';
			if ($status[0] != 0) echo '<font face="Verdana, Arial, Helvetica, sans-serif" size="2"><img src="site_images/progress.gif" border="1" width="'.$status[0].'%" height="8" alt="'.$status[0].'% ('.$percent_text.')"></font>'; else echo '&nbsp;';
			echo '</td>
			</tr>';
		}

		echo '</table>';

	}
	echo '</td></tr></table></td></tr></table></td></tr></table><br>';
}
function stri_replace($searchFor, $replaceWith, $string, $offset = 0)
{
	$lsearchFor = strtolower($searchFor);
	$lstring = strtolower($string);
	$newPos = strpos($lstring, $lsearchFor, $offset);
	if (strlen($newPos) == 0)
	return($string);
	else
	{
		$left = substr($string, 0, $newPos);
		$right = substr($string, $newPos + strlen($searchFor));
		$newStr = $left . $replaceWith . $right;
		return stri_replace($searchFor, $replaceWith, $newStr, $newPos + strlen($replaceWith));
	}
}
function search_results($keyword)
{


	$keyword = mysql_escape_string(stripslashes($keyword));

	$query = mysql_query("
	SELECT
	list_game.ID, list_game.name, list_game.released, list_game.version, list_game.first_char
	FROM
	list_game
	WHERE
	list_game.name LIKE '%$keyword%'
	ORDER BY
	list_game.name");

	$num = mysql_num_rows($query);
	if ($num == 0)
	{
		echo '<table width="730" cellspacing="0" cellpadding="1" bgcolor="#000000"><tr><td valign="top" align="left"><table cellspacing="4" cellpadding="0" width="100%" bgcolor="#355787"><tr><td>
		<table cellspacing="0" cellpadding="0" width="100%"><tr><td valign="top" align="left"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">Searching game-database (keyword "<b>'.$keyword.'</b>")</td></tr></table></td>
		</tr></table></td></tr></table><table width="730" cellspacing="0" cellpadding="1" bgcolor="#000000"><tr><td valign="top" align="left"><table cellspacing="4" cellpadding="0" width="100%" bgcolor="#113466"><tr>
		<td><table cellspacing="0" cellpadding="0" width="100%"><tr><td valign="top" align="left">';

		echo '<table cellspacing="0" cellpadding="0" width="100%"><tr><td><font face="Verdana, Arial, Helvetica, sans-serif" size="2"><i>0 hits found with the keyword "'.$keyword.'"</td></tr></table>';

	}
	else
	{

		echo '<table width="730" cellspacing="0" cellpadding="1" bgcolor="#000000"><tr><td valign="top" align="left"><table cellspacing="4" cellpadding="0" width="100%" bgcolor="#355787"><tr><td>
		<table cellspacing="0" cellpadding="0" width="100%"><tr><td valign="top" align="left"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">Searching: <b>'.$keyword.'</b> (<b>'.$num.'</b>'; if ($num ==1) echo ' result found'; else echo ' results found'; echo ')</td></tr></table></td>
		</tr></table></td></tr></table><table width="730" cellspacing="0" cellpadding="1" bgcolor="#000000"><tr><td valign="top" align="left"><table cellspacing="4" cellpadding="0" width="100%" bgcolor="#113466"><tr>
		<td><table cellspacing="0" cellpadding="0" width="100%"><tr><td valign="top" align="left">';


		echo '<table cellspacing="0" cellpadding="0" width="720">
		<tr>
		<td width="385">
		<font face="Verdana, Arial, Helvetica, sans-serif" size="2"><b>Game:</b></font>
		</td>

		<td width="6">
		&nbsp;
		</td>

		<td width="70">
		<font face="Verdana, Arial, Helvetica, sans-serif" size="2"><b>Version:</b></font>
		</td>

		<td width="6">
		&nbsp;
		</td>

		<td width="70">
		<font face="Verdana, Arial, Helvetica, sans-serif" size="2"><b>Status:</b></font>
		</td>


		<td width="6">
		&nbsp;
		</td>


		<td width="275">
		<font face="Verdana, Arial, Helvetica, sans-serif" size="2"><b>runable&nbsp;&nbsp;-&nbsp;&nbsp;playable&nbsp;&nbsp;-&nbsp;&nbsp;supported</font>
		</td>
		</tr>';

		while($result=mysql_fetch_row($query))
		{

			$status_query = mysql_query("SELECT status_games.status, versions.version FROM versions, status_games WHERE status_games.gameID=".$result[0]." AND status_games.versionID=versions.ID ORDER BY status_games.status DESC");
			$status = mysql_fetch_row($status_query);
			$percent_text = return_status($status[0]);

			$name = stri_replace($keyword, "<b><font color=\"#90DEFF\">".$keyword."</b></font color>", $result[1]);
			echo '<tr>
			<td>
			<font face="Verdana, Arial, Helvetica, sans-serif" size="2"><a href="comp_list.php?showID='.$result[0].'&letter='.$result[4].'&search='.$keyword.'">'.$name.'</a>'; if ($result[2] != 0) echo ' ('.$result[2].')'; echo '</font>
			</td>

			<td>
			&nbsp;
			</td>

			<td>
			<font face="Verdana, Arial, Helvetica, sans-serif" size="2">'.$status[1].'</font>
			</td>

			<td>
			&nbsp;
			</td>

			<td>
			<font face="Verdana, Arial, Helvetica, sans-serif" size="2">'.$percent_text.'</font>
			</td>

			<td>
			&nbsp;
			</td>


			<td>';
			if ($status[0] != 0) echo '<img src="site_images/progress.gif" border="1" width="'.$status[0].'%" height="8" alt="'.$status[0].'% ('.$percent_text.')"></font>'; else echo '&nbsp;';
			echo '</td>
			</tr>';
		}

		echo '</table>';

	}
	echo '</td></tr></table></td></tr></table></td></tr></table><br>';
}
function comp_show_ID($showID)
{
	global $user;
	$gameID = mysql_escape_string(stripslashes($gameID));

	$query = mysql_query("

	SELECT
	list_game.ID, list_game.name, list_game.publisher, list_game.released,
	userdb.nickname, userdb.ID, ownerID
	FROM
	list_game, userdb
	WHERE
	list_game.ID=$showID AND list_game.ownerID=userdb.ID
	");

	$result=mysql_fetch_row($query);

	echo '<table width="730" cellspacing="0" cellpadding="1" bgcolor="#000000"><tr><td valign="top" align="left"><table cellspacing="4" cellpadding="0" width="100%" bgcolor="#355787"><tr><td>
	<table cellspacing="0" cellpadding="0" width="100%"><tr><td valign="top" align="left"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">Game details';

	if (isset($_SESSION['userID']))
	echo '&nbsp;&nbsp;<a href="comp_list.php?changeID='.$showID.'&letter='.$_GET['letter'].'"><img src="site_images/change_icon.gif" border="0"></a>&nbsp;';
	if ($result[6] == $_SESSION['userID'] || $user['priv']['manage_comment']==1)
	echo '<a href="comp_list.php?removeID='.$showID.'&letter='.$_GET['letter'].'"><img src="site_images/delete_icon.gif" border="0"></a>';

	echo '</td></tr></table></td>
	</tr></table></td></tr></table><table width="730" cellspacing="0" cellpadding="1" bgcolor="#000000"><tr><td valign="top" align="left"><table cellspacing="4" cellpadding="0" width="100%" bgcolor="#113466"><tr>
	<td><table cellspacing="0" cellpadding="0" width="100%"><tr><td valign="top" align="left">';


	echo '<font face="Verdana, Arial, Helvetica, sans-serif" size="2">'.$result[1].' - '.$result[2]; if ($result[3] != 0) echo ' ('.$result[3].')';
	echo '<br><b>Tested By:</b> '.$result[4].'<br><br>';

	$status_query=mysql_query("
	SELECT
	status_games.versionID, status_games.status, versions.version

	FROM
	status_games, versions
	WHERE
	status_games.gameID=$result[0] AND status_games.versionID=versions.ID
	ORDER BY
	versions.version DESC
	");

	while ($status = mysql_fetch_row($status_query))
	{
		$status_text = return_status($status[1]);

		echo ;

		echo '<table cellspacing="0" cellpadding="0" width="580">
		<tr>
		<td>
		<table cellspacing="0" cellpadding="0" width="262">
		<tr>
		<td width="237">
		<font face="Verdana, Arial, Helvetica, sans-serif" size="2"><b>runable&nbsp;&nbsp;-&nbsp;&nbsp;playable&nbsp;&nbsp;-&nbsp;&nbsp;supported</font>
		</td>
		</tr>';



		echo '<tr>
		<td width="237">';
		if ($status[1] != 0) echo '<img src="site_images/progress.gif" border="1" width="'.$status[1].'%" height="8" alt="'.$status[1].'% ('.$status_text.')"></font>'; else echo '<font face="Verdana, Arial, Helvetica, sans-serif" size="2">0% supported</font>';
		echo '</td>
		</tr>';

		echo '</table>
		</td>
		<td width="20">
		&nbsp;
		</td>
		<td valign="middle" align="left" width="330">
		<font face="Verdana, Arial, Helvetica, sans-serif" size="2"><b>DOSBox version:</b> '.$status[2].'</b> ('.$status_text.')<br></font>
		</td>

		</tr>
		</table>
		<hr line color="#FFFFFF" width="525" align="left">
		';



	}

	if (isset($_SESSION['userID']) AND $_GET['post_new']!=1)
	echo '<font face="Verdana, Arial, Helvetica, sans-serif" size="2"><a href="comp_list.php?post_newMSG=1&showID='.$_GET['showID'].'&letter='.$_GET['letter'].'#post_comment">Click here</a> to post new comment</font><br>';
	echo '</td></tr></table></td></tr></table></td></tr></table>';
}
function comp_bar()
{
	echo '

	<table cellspacing="0" cellpadding="0">
	<tr>
	<td>
	<font face="Verdana, Arial, Helvetica, sans-serif" size="2"><b>First char:
		</b></font>
	</td>

	<td width="6">&nbsp;</td>

	<td>
	<font face="Verdana, Arial, Helvetica, sans-serif" size="2"><b>Latest added:
		</b></font>
	</td>

	<td width="6">&nbsp;</td>

	<td>
	<font face="Verdana, Arial, Helvetica, sans-serif" size="2"><b>Game-search:
		</b></font>
	</td>


	</tr>

	<tr>
	<td>
	<form name="sort">
	<select name="letter" onChange="submit(form.sort);" method="POST" action="comp_list.php">
	<option value="A">A ('; echo count_firstchar('A'); echo ')</option>
	<option value="B"'; if ($_GET['letter'] =='B') echo ' selected'; echo '>B ('; echo count_firstchar('B'); echo ')</option>
	<option value="C"'; if ($_GET['letter'] =='C') echo ' selected'; echo '>C ('; echo count_firstchar('C'); echo ')</option>
	<option value="D"'; if ($_GET['letter'] =='D') echo ' selected'; echo '>D ('; echo count_firstchar('D'); echo ')</option>
	<option value="E"'; if ($_GET['letter'] =='E') echo ' selected'; echo '>E ('; echo count_firstchar('E'); echo ')</option>
	<option value="F"'; if ($_GET['letter'] =='F') echo ' selected'; echo '>F ('; echo count_firstchar('F'); echo ')</option>
	<option value="G"'; if ($_GET['letter'] =='G') echo ' selected'; echo '>G ('; echo count_firstchar('G'); echo ')</option>
	<option value="H"'; if ($_GET['letter'] =='H') echo ' selected'; echo '>H ('; echo count_firstchar('H'); echo ')</option>
	<option value="I"'; if ($_GET['letter'] =='I') echo ' selected'; echo '>I ('; echo count_firstchar('I'); echo ')</option>
	<option value="J"'; if ($_GET['letter'] =='J') echo ' selected'; echo '>J ('; echo count_firstchar('J'); echo ')</option>
	<option value="K"'; if ($_GET['letter'] =='K') echo ' selected'; echo '>K ('; echo count_firstchar('K'); echo ')</option>
	<option value="L"'; if ($_GET['letter'] =='L') echo ' selected'; echo '>L ('; echo count_firstchar('L'); echo ')</option>
	<option value="M"'; if ($_GET['letter'] =='M') echo ' selected'; echo '>M ('; echo count_firstchar('M'); echo ')</option>
	<option value="N"'; if ($_GET['letter'] =='N') echo ' selected'; echo '>N ('; echo count_firstchar('N'); echo ')</option>
	<option value="O"'; if ($_GET['letter'] =='O') echo ' selected'; echo '>O ('; echo count_firstchar('O'); echo ')</option>
	<option value="P"'; if ($_GET['letter'] =='P') echo ' selected'; echo '>P ('; echo count_firstchar('P'); echo ')</option>
	<option value="Q"'; if ($_GET['letter'] =='Q') echo ' selected'; echo '>Q ('; echo count_firstchar('Q'); echo ')</option>
	<option value="R"'; if ($_GET['letter'] =='R') echo ' selected'; echo '>R ('; echo count_firstchar('R'); echo ')</option>
	<option value="S"'; if ($_GET['letter'] =='S') echo ' selected'; echo '>S ('; echo count_firstchar('S'); echo ')</option>
	<option value="T"'; if ($_GET['letter'] =='T') echo ' selected'; echo '>T ('; echo count_firstchar('T'); echo ')</option>
	<option value="U"'; if ($_GET['letter'] =='U') echo ' selected'; echo '>U ('; echo count_firstchar('U'); echo ')</option>
	<option value="V"'; if ($_GET['letter'] =='V') echo ' selected'; echo '>V ('; echo count_firstchar('V'); echo ')</option>
	<option value="W"'; if ($_GET['letter'] =='W') echo ' selected'; echo '>W ('; echo count_firstchar('W'); echo ')</option>
	<option value="X"'; if ($_GET['letter'] =='X') echo ' selected'; echo '>X ('; echo count_firstchar('X'); echo ')</option>
	<option value="Y"'; if ($_GET['letter'] =='Y') echo ' selected'; echo '>Y ('; echo count_firstchar('Y'); echo ')</option>
	<option value="Z"'; if ($_GET['letter'] =='Z') echo ' selected'; echo '>Z ('; echo count_firstchar('Z'); echo ')</option>
	<option value="num"'; if ($_GET['letter'] =='num') echo ' selected'; echo '>Numerical ('; echo count_firstchar('num'); echo ')</option>
	</select>

	</form>

	</td>

	<td width="6">&nbsp;</td>

	<td>
	<form name="latest" method="GET" action="comp_list.php">
	<input name="letter" type="hidden" value="'.$_GET['letter'].'">
	<select name="showID" onChange="submit(form.latest);">
	<option value="0">-</option>';
	compat_list_latest();

	echo '
	</select>
	</form>

	</td>

	<td width="6">&nbsp;</td>

	<td>
	<form name="game-search">
	<input name="letter" type="hidden" value="'.$_GET['letter'].'"><input type="text" name="search" size="18" value="game-name">&nbsp;<input type="submit" name="submit" value="Search">
	</form>
	</td>

	</tr>
	</table>';
	if ($_GET['post_new'] != 1 AND $_GET['posting'] !=1 AND isset($_SESSION['userID']))
	echo '<font face="Verdana, Arial, Helvetica, sans-serif" size="2"><a href="comp_list.php?post_new=1&letter='.$_GET['letter'].'">Add new game to database</a></font><br><br>';
}
function count_firstchar($letter)
{
	$letter = mysql_escape_string(stripslashes($letter));

	if ($letter == 'num')
	$query = mysql_query("SELECT COUNT(*) FROM list_game WHERE first_char='1' OR first_char='2' OR first_char='3' OR first_char='4' OR first_char='5' OR first_char='6' OR first_char='7' OR first_char='8' OR first_char='9' OR first_char='0' OR first_char='#' OR first_char='!' OR first_char='$'");
	else
	$query = mysql_query("SELECT COUNT(*) FROM list_game WHERE first_char='$letter'");

	$result = mysql_fetch_row($query);
	return $result[0];
}
function return_status($percent)
{
	if ($percent == 0 )
	return 'broken';	// game doesn't work in DOSBox
	elseif ($percent >= 64 )
	return 'supported';	// game is supported, may have some glitches and small issues
	elseif ($percent >= 29)
	return 'playable';	// game is playable but with some serius problems/errors/glitches
	elseif ($percent <= 28 )
	return 'runable';	// game starts in DOSBox but is not playable
}

function get_msg_threads($gameID, $msgID=null)
{
	global $user;
	$gameID = mysql_escape_string(stripslashes($gameID));

	if (!isset($msgID))
	{
		$query = mysql_query("
		SELECT
		list_comment.ID, list_comment.gameID, list_comment.ownerID,
		list_comment.subject, list_comment.text, list_comment.parent_id,
		DATE_FORMAT(list_comment.datetime, '%Y-%m-%d %H:%i'),
		userdb.ID, userdb.nickname
		FROM
		list_comment, userdb
		WHERE
		list_comment.gameID=$gameID
		AND list_comment.ownerID=userdb.ID

		ORDER BY datetime ASC");
	}

	while ($result = mysql_fetch_row($query))
	{

		echo '<table width="730" cellspacing="0" cellpadding="1" bgcolor="#000000">
		<tr>
		<td valign="top" align="left">
		<table cellspacing="4" cellpadding="0" width="100%" bgcolor="#355787">
		<tr>
		<td>
		<table cellspacing="0" cellpadding="0" width="100%">
		<tr>
		<td valign="top" align="left">
		<font face="Verdana, Arial, Helvetica, sans-serif" size="2">'.$result[3].' ('.$result[6].')';

		if ($user['priv']['compat_list_manage']==1)
		echo '&nbsp;<a href="comp_list.php?removeMSG_ID='.$result[0].'&letter='.$_GET['letter'].'&gameID='.$_GET['showID'].'"><img src="site_images/msgboard_remove.gif" alt="Remove this comment" border="0"></a>';





		echo '</td>
		<td valign="top" align="right" width="135">
		<font face="Verdana, Arial, Helvetica, sans-serif" size="2">'.$result[8].'
		</td>
		</tr>
		</table>

		</td>
		</tr>
		</table>
		</td>
		</tr>
		</table>
		<table width="730" cellspacing="0" cellpadding="1" bgcolor="#000000">
		<tr>
		<td valign="top" align="left">
		<table cellspacing="4" cellpadding="0" width="100%" bgcolor="#0D2E5D">
		<tr>
		<td>
		<table cellspacing="0" cellpadding="0" width="100%">
		<tr>
		<td valign="top" align="left">
		<font face="Verdana, Arial, Helvetica, sans-serif" size="2">
		';
		echo wordwrap($result[4], 105, "\n", 1);;

		echo '</td>
		</tr>
		</table>

		</td>
		</tr>
		</table>
		</td>
		</tr>
		</table>';
	}
	if (mysql_num_rows($query) != 0)
	echo '<br><br>';
}
function write_comment()
{
	echo '<table width="730" cellspacing="0" cellpadding="1" bgcolor="#000000"><tr><td valign="top" align="left"><table cellspacing="4" cellpadding="0" width="100%" bgcolor="#355787"><tr><td>
	<table cellspacing="0" cellpadding="0" width="100%"><tr><td valign="top" align="left"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">Write new comment</td></tr></table></td>
	</tr></table></td></tr></table><table width="730" cellspacing="0" cellpadding="1" bgcolor="#000000"><tr><td valign="top" align="left"><table cellspacing="4" cellpadding="0" width="100%" bgcolor="#113466"><tr>
	<td><table cellspacing="0" cellpadding="0" width="100%"><tr><td valign="top" align="left"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">';

	echo '
	<form action="comp_list.php?post_comment=1" method="POST" name="comment">
	<input type="hidden" name="gameID" value="'.$_GET['showID'].'"><input type="hidden" name="letter" value="'.$_GET['letter'].'"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">';

	if ($_GET['problem']==1)
	echo '<b>Error - this form must be filled in accurate!</b><br>';

	echo '
	<br>
	Subject: (max 60 chars)<br>
	<input type="text" name="subject" size="60" maxlength="60"><br><br>

	Comment: (max 1024 chars)<br>
	<textarea name="text" cols="50" rows="10"></textarea><br><br>
	<input type="submit" value="Post comment">
	</form>';

	echo '</td></tr></table></td></tr></table></td></tr></table><br>';
}
function show_screenshots($limit)
{
	$page = mysql_escape_string(stripslashes($_GET['page']));

	$query = mysql_query("
	SELECT
	ID, text
	FROM
	screenshots
	ORDER BY
	screenshots.datetime DESC

	LIMIT ".($page*3).",3

	");

	$count_query = mysql_query("SELECT COUNT(ID) FROM screenshots");
	$count = mysql_fetch_row($count_query);

	$maxpages=floor(($count[0]-1)/3);

	while ($result = mysql_fetch_row($query))
	{
		echo '<a href="screenshots/big/'.$result[0].'.png" target="_blank"><img src="screenshots/thumb/'.$result[0].'.png" border="0" alt="'.$result[1].'"></a><br><br>';
	}

	echo '<table cellspacing="0" cellpadding="0" width="200">
	<tr>
	<td align="left" width="17">';

	if (!$page==0)
	echo '<a href="information.php?page='.($page-1).'"><img src="site_images/arrow_left.gif" border="0" alt="Browse screenshots-archive"></a>';
	else
	echo '<img src="site_images/arrow_left_nofuther.gif" border="0">';

	echo '</td><td align="center"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">browse screen-archive</td><td align="right" width="17">';

	if ($page<($maxpages))
	echo '<a href="information.php?page='.($page+1).'"><img src="site_images/arrow_right.gif" border="0" alt="Browse screenshots-archive"></a>';
	else
	echo '<img src="site_images/arrow_right_nofuther.gif" border="0">';

	echo '</td></tr></table>';
}
function get_support_stats()
{
	$query = mysql_query("SELECT COUNT(ID) FROM list_game");
	$result = mysql_fetch_row($query);
	$text = 'Compatibility statistics (<b>'.$result[0].'</b> games in database)';

	template_pagebox_start($text);

	echo '
	<table cellspacing="0" cellpadding="0" width="100%">
	<tr>
	<td valign="top"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
	<b>Version:</b></font></td>

	<td valign="top"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
	<b>Games broken:</b></font></td>

	<td valign="top"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
	<b>Games runable:</b></font></td>

	<td valign="top"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
	<b>Games playable:</b></font></td>

	<td valign="top"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
	<b>Games supported:</b></font></td>
	</tr>

	';

	$version_query = mysql_query("SELECT ID, version FROM versions ORDER BY version DESC");
	while ($version_result = mysql_fetch_row($version_query))
	{
		$v_query = mysql_query("SELECT COUNT(ID) FROM status_games WHERE status_games.versionID=".$version_result[0]);
		$v_count = mysql_fetch_row($v_query);

		$broken_query = mysql_query("SELECT COUNT(ID) FROM status_games WHERE status_games.versionID=".$version_result[0]." AND status_games.status = 0");
		$broken_result = mysql_fetch_row($broken_query);

		$supported_query = mysql_query("SELECT COUNT(ID) FROM status_games WHERE status_games.versionID=".$version_result[0]." AND status_games.status >= 64");
		$supported_result = mysql_fetch_row($supported_query);

		$playable_query = mysql_query("SELECT COUNT(ID) FROM status_games WHERE status_games.versionID=".$version_result[0]." AND status_games.status >= 29 AND status_games.status < 64");
		$playable_result = mysql_fetch_row($playable_query);

		$runable_query = mysql_query("SELECT COUNT(ID) FROM status_games WHERE status_games.versionID=".$version_result[0]." AND status_games.status <= 28 AND status_games.status > 0");
		$runable_result = mysql_fetch_row($runable_query);

		echo '<tr>
		<td valign="top"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
		DOSBox '.$version_result[1].' ('.$v_count[0].')</font></td>

		<td valign="top"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
		'.$broken_result[0].'</font></td>

		<td valign="top"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
		'.$runable_result[0].'</font></td>

		<td valign="top"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
		'.$playable_result[0].'</font></td>

		<td valign="top"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
		'.$supported_result[0].'</font></td>
		</tr>';


	}
	echo '</table>';
	template_pagebox_end();
}
?>
