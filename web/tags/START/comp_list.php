<?php
// this src is written under the terms of the GPL-licence, see gpl.txt for futher details
include("include/standard.inc.php");
sstart();


if ($_GET['changeversion']==1)
{
	if (isset($_SESSION['userID']))
	mysql_query("UPDATE status_games SET status=".$_POST['percent']." WHERE status_games.ID=".$_POST['statusID']);

	Header("Location: comp_list.php?changeID=".$_POST['gameID']."&letter=".$_POST['letter']);
}
if (isset($_GET['removeMSG_ID']))
{
	if ($user['priv']['compat_list_manage']==1)
	{
		$msgID =mysql_escape_string(stripslashes($_GET['removeMSG_ID']));
		mysql_query("DELETE FROM list_comment WHERE ID=$msgID");
	}
	Header("Location: comp_list.php?showID=".$_GET['gameID']."&letter=".$_GET['letter']);
}
if (isset($_GET['removeVERSION_ID']))
{
	$versionID	=mysql_escape_string(stripslashes($_GET['removeVERSION_ID']));
	$gameID		=$_GET['gameID'];
	$letter		=$_GET['letter'];
	$userID		=$_SESSION['userID'];

	if ($user['priv']['compat_list_manage']==1)
	mysql_query("DELETE FROM status_games WHERE status_games.ID=$versionID");

	Header("Location: comp_list.php?changeID=".$gameID."&letter=".$_GET['letter']);
}
if ($_GET['addVersion']==1)
{
	$gameID		=mysql_escape_string(stripslashes($_POST['gameID']));
	$versionID	=mysql_escape_string(stripslashes($_POST['versionID']));
	$status		=mysql_escape_string(stripslashes($_POST['percent']));
	$userID		=$_SESSION['userID'];

	if (isset($_SESSION['userID']) AND $versionID != '-')
	{
		mysql_query("

		INSERT INTO
		status_games (gameID, versionID, status)
		VALUES ($gameID, $versionID, $status)
		");

		#mysql_query("UPDATE list_game SET ownerID=$userID WHERE ID=$gameID");
	}
	Header("Location: comp_list.php?changeID=".$gameID."&letter=".$_POST['letter']);
}
if ($_GET['post_comment']==1)
{
	$gameID		=mysql_escape_string(stripslashes($_POST['gameID']));
	$subject	=mysql_escape_string(stripslashes($_POST['subject']));
	$text		=mysql_escape_string(stripslashes($_POST['text']));
	$userID		=$_SESSION['userID'];

	if (isset($_SESSION['userID']))
	{
		if ($_POST['subject'] == '' || $_POST['text'] == '' || strlen($_POST['subject'])>60 || strlen($_POST['text'])>1024)
		Header("Location: comp_list.php?problem=1&post_newMSG=1&showID=".$gameID."&letter=".$_POST['letter']."#post_comment");
		else
		{
			mysql_query("

			INSERT INTO
			list_comment (gameID, ownerID, subject, text, datetime)
			VALUES
			($gameID, $userID, '$subject', '$text', NOW())
			");

			Header("Location: comp_list.php?showID=".$gameID."&letter=".$_POST['letter']);
		}
	}

}
if ($_GET['changing']==1)
{

	$changeID	= $_POST['ID'];
	$letter		= $_POST['letter'];
	$name 		= mysql_escape_string(stripslashes($_POST['name']));
	$publisher 	= mysql_escape_string(stripslashes($_POST['publisher']));
	$released 	= mysql_escape_string(stripslashes($_POST['year']));
	$moby 		= mysql_escape_string(stripslashes($_POST['moby']));
	$userID		= $_SESSION['userID'];
	$first_char 	= $name{0};


	if (check_game_owner($changeID, $_SESSION['userID'])==1 || $user['priv']['compat_list_manage']==1)
	{
		if ($name == '' || $publisher == '' || !is_numeric($released))
		Header("Location: comp_list.php?letter=".$letter."&problem=1&changeID=".$changeID);
		else
		{

			if ($moby != '' AND verifyurl($moby)==false)
			Header("Location: comp_list.php?letter=".$letter."&problem=1&changeID=".$changeID);

			mysql_query("UPDATE list_game SET name='$name', publisher='$publisher', first_char='$first_char', released=$released, moby_url='$moby' WHERE ID=$changeID");
			Header("Location: comp_list.php?changeID=".$changeID."&letter=".$letter);
		}
	}
}

if ($_GET['removeID'])
{
	if (check_game_owner($_GET['removeID'], $_SESSION['userID'])==1 || $user['priv']['compat_list_manage']==1)
	{
		$gameID = mysql_escape_string(stripslashes($_GET['removeID']));

		mysql_query("DELETE FROM list_game WHERE ID=$gameID");
		mysql_query("DELETE FROM status_games WHERE gameID=$gameID");
		mysql_query("DELETE FROM list_comment WHERE gameID=$gameID");

		if (is_numeric($_GET['letter']))
		Header("Location: comp_list.php?letter=num");
		else
		Header("Location: comp_list.php?letter=".$_GET['letter']);
	}
}
if ($_SESSION['userID'])
{
	if ($_GET['posting_new']==1)
	{
		$name 		= mysql_escape_string(stripslashes($_POST['name']));
		$publisher 	= mysql_escape_string(stripslashes($_POST['publisher']));
		$percent 	= mysql_escape_string(stripslashes($_POST['percent']));
		$released 	= mysql_escape_string(stripslashes($_POST['year']));
		$comment	= mysql_escape_string(stripslashes($_POST['note']));
		$version 	= mysql_escape_string(stripslashes($_POST['version']));
		$moby 		= mysql_escape_string(stripslashes($_POST['moby']));
		$userID		= $_SESSION['userID'];
		$first_char 	= $name{0};

		if ($name == '' || $publisher == '' || !is_numeric($released) || strlen($comment)>1024)
		Header("Location: comp_list.php?post_new=1&error=1&letter=".$_POST['letter']);
		else
		{
			if (verifyurl($moby)==0 AND $moby != '')
			Header("Location: comp_list.php?post_new=1&error=1&letter=".$_POST['letter']);

			mysql_query("
			INSERT INTO list_game
			(ownerID, added, name, publisher, version, first_char, released, moby_url)
			VALUES($userID, NOW(), '$name', '$publisher', '$version', '$first_char', $released, '$moby_url')");

			$AUTO_INCREMENT_ID = mysql_insert_id();

			mysql_query("INSERT INTO status_games (gameID,versionID,status) VALUES($AUTO_INCREMENT_ID, $version, $percent)");

			if ($comment != '')
			{
				$subject = 'Note:';

				mysql_query("

				INSERT INTO list_comment
				(gameID, ownerID, subject, text, datetime)
				VALUES ($AUTO_INCREMENT_ID, $userID, '$subject', '$comment', NOW())
				");
			}

			Header("Location: comp_list.php?showID=".$AUTO_INCREMENT_ID."&letter=".$_POST['letter']);
		}

	}


}

template_header();

echo '
<table width="100%">
<tr>
<td width="9">
&nbsp;
</td>
<td>';

comp_bar();


if ($_GET['showID'] AND $_GET['post_new'] != 1)
{
	if (!isset($_SESSION['userID']))
	echo '<font face="Verdana, Arial, Helvetica, sans-serif" size="2">You must login before posting comments/adding games, <a href="login.php">click here</a> to get to the login page!</font><br><br>';

	comp_show_ID($_GET['showID']);
}
echo '<br><br>';
if (isset($_GET['showID']) AND $_GET['post_new'] != 1)
{
	get_msg_threads($_GET['showID']);

	if ($_GET['post_newMSG']==1)
	{
		echo '<br>';
		echo '<a name="post_comment">';
		write_comment();

		echo '<br>';
	}


}

if ($_GET['changeID'])
{
	$gameID = mysql_escape_string(stripslashes($_GET['changeID']));
	$query = mysql_query("SELECT ID, ownerID, name, publisher, version, released, moby_url FROM list_game WHERE ID = $gameID");

	$result = mysql_fetch_row($query);


	echo '<table width="730" cellspacing="0" cellpadding="1" bgcolor="#000000"><tr><td valign="top" align="left"><table cellspacing="4" cellpadding="0" width="100%" bgcolor="#355787"><tr><td>
	<table cellspacing="0" cellpadding="0" width="100%"><tr><td valign="top" align="left"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">Updating game-post</td></tr></table></td>
	</tr></table></td></tr></table><table width="730" cellspacing="0" cellpadding="1" bgcolor="#000000"><tr><td valign="top" align="left"><table cellspacing="4" cellpadding="0" width="100%" bgcolor="#113466"><tr>
	<td><table cellspacing="0" cellpadding="0" width="100%"><tr><td valign="top" align="left"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">';

	if (check_game_owner($_GET['changeID'], $_SESSION['userID'])==1 || $user['priv']['compat_list_manage']==1)
	{
		if ($_GET['error']==1)
		echo '<b>Error - this form must be filled in accurate!</b>';

		echo '<form action="comp_list.php?changing=1" method="POST">
		<input type="hidden" name="ID" value="'.$result[0].'">
		<input type="hidden" name="ownerID" value='.$result[1].'>
		<input type="hidden" name="letter" value="'.$_GET['letter'].'">
		<table cellspacing="0" cellpadding="0" width="100%">
		<tr>
		<td valign="top"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
		Game title:
		</td>
		<td width="10">
		&nbsp;
		</td>
		<td valign="top"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
		Publisher/Developer:
		</td>

		</tr>
		<tr>
		<td valign="top"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
		<input type="text" name="name" size="38" maxlength="35" value="'.$result[2].'">
		</td>
		<td width="10">
		&nbsp;
		</td>
		<td valign="top"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
		<input type="text" name="publisher" size="38" maxlength="20" value="'.$result[3].'">
		</td>

		</tr>
		<tr>
		<td colspan="3">
		&nbsp;
		</td>
		</tr>
		<tr>
		<td valign="top"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
		Year released:
		</td>
		<td width="10">
		&nbsp;
		</td>
		<td valign="top"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
		<!--Moby-games link: -->
		</td>

		</tr>
		<tr>
		<td valign="top"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
		<input type="text" name="year" size="38" maxlength="4" value="'.$result[5].'">
		</td>
		<td width="10">
		&nbsp;
		</td>
		<td valign="top"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
		<input type="hidden" name="moby" size="38" maxlength="65" value="'.$result[6].'">
		</td>

		</tr>
		<tr>
		<td colspan="3">
		&nbsp;
		</td>
		</tr>
		</table>
		<input type="submit" name="submit" value="Update post">
		<br><br><hr line width="650" color="#FFFFFF">
		</form><font face="Verdana, Arial, Helvetica, sans-serif" size="2">';
	}
	else
	{
		echo '<br>You are logged in as a "normal" user, and are not allowed to edit these settings, however.. you may still add/edit DOSBox version-support for this game.<br><br>click on the buttons to apply the new settings!<br><br>
		<table cellspacing="0" cellpadding="0" width="100%">
		<tr>
		<td valign="top"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
		Game title:
		</td>
		<td width="10">
		&nbsp;
		</td>
		<td valign="top"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
		Publisher/Developer:
		</td>

		</tr>
		<tr>
		<td valign="top"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
		<input type="text" name="name" size="38" maxlength="35" value="'.$result[2].'" disabled>
		</td>
		<td width="10">
		&nbsp;
		</td>
		<td valign="top"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
		<input type="text" name="publisher" size="38" maxlength="20" value="'.$result[3].'" disabled>
		</td>

		</tr>
		<tr>
		<td colspan="3">
		&nbsp;
		</td>
		</tr>
		<tr>
		<td valign="top"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
		Year released:
		</td>
		<td width="10">
		&nbsp;
		</td>
		<td valign="top"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
		<!--Moby-games link: -->
		</td>

		</tr>
		<tr>
		<td valign="top"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
		<input type="text" name="year" size="38" maxlength="4" value="'.$result[5].'" disabled>
		</td>
		<td width="10">
		&nbsp;
		</td>
		<td valign="top"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
		<input type="hidden" name="moby" size="38" maxlength="65" value="'.$result[6].'">
		</td>

		</tr>
		<tr>
		<td colspan="3">
		&nbsp;
		</td>
		</tr>
		</table>
		<hr line width="650" color="#FFFFFF">
		<font face="Verdana, Arial, Helvetica, sans-serif" size="2">';
	}



	change_version_compatibility_form($result[0]);

	echo '<hr line width="650" color="#FFFFFF"><br><form name="addversion" method="POST" action="comp_list.php?addVersion=1"><input type="hidden" name="gameID" value="'.$_GET['changeID'].'">';

	add_version_compatibility_form($result[0]);
	choose_percentage();
	echo '&nbsp;<input type="hidden" name="letter" value="'.$_GET['letter'].'"><input type="submit" value="Add version-support"><br><br>
	<br><a href="comp_list.php?showID='.$_GET['changeID'].'&letter='.$_GET['letter'].'">Click here to get back!<br></form>';


	echo '</td></tr></table></td></tr></table></td></tr></table><br>';
}

if ($_GET['post_new']==1 AND isset($_SESSION['userID']))
{
	echo '<table width="730" cellspacing="0" cellpadding="1" bgcolor="#000000"><tr><td valign="top" align="left"><table cellspacing="4" cellpadding="0" width="100%" bgcolor="#355787"><tr><td>
	<table cellspacing="0" cellpadding="0" width="100%"><tr><td valign="top" align="left"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">Adding new game</td></tr></table></td>
	</tr></table></td></tr></table><table width="730" cellspacing="0" cellpadding="1" bgcolor="#000000"><tr><td valign="top" align="left"><table cellspacing="4" cellpadding="0" width="100%" bgcolor="#113466"><tr>
	<td><table cellspacing="0" cellpadding="0" width="100%"><tr><td valign="top" align="left"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">';

	if ($_GET['error']==1)
	echo '<b>Error - this form must be filled in accurate!</b>';

        echo 'If the game is <b>already present</b> in the database.
	select that game and press the little <b>square</b> next to its name to add a new
	version or another comment';
	echo '<form action="comp_list.php?posting_new=1" method="POST">
	<input type="hidden" name="letter" value="'.$_GET['letter'].'">
	<table cellspacing="0" cellpadding="0" width="100%">
	<tr>
	<td valign="top"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
	Game title:
	</td>
	<td width="10">
	&nbsp;
	</td>
	<td valign="top"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
	Publisher/Developer:
	</td>

	</tr>
	<tr>
	<td valign="top"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
	<input type="text" name="name" size="38" maxlength="35">
	</td>
	<td width="10">
	&nbsp;
	</td>
	<td valign="top"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
	<input type="text" name="publisher" size="38" maxlength="20">
	</td>

	</tr>
	<tr>
	<td colspan="3">
	&nbsp;
	</td>
	</tr>
	<tr>
	<td valign="top"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
	Year released:
	</td>
	<td width="10">
	&nbsp;
	</td>
	<td valign="top"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
	<!--Moby-games link: -->
	</td>

	</tr>
	<tr>
	<td valign="top"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
	<input type="text" name="year" size="38" maxlength="4">
	</td>
	<td width="10">
	&nbsp;
	</td>
	<td valign="top"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
	<input type="hidden" name="moby" size="38" maxlength="20">
	</td>

	</tr>
	<tr>
	<td colspan="3">
	&nbsp;
	</td>
	</tr>
	<tr>
	<td valign="top"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
	Running DOSBox version:
	</td>
	<td width="10">
	&nbsp;
	</td>
	<td valign="top"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
	Compatibility: (specify DOSBox support in percent, 0-100%)
	</td>

	</tr>
	<tr>
	<td valign="top"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">';
	get_versions();
	echo '</td>
	<td width="10">
	&nbsp;
	</td>
	<td valign="top"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">';
	choose_percentage();

	echo '</td>

	</tr>



	</table>
	<br>
	<font face="Verdana, Arial, Helvetica, sans-serif" size="2">


	Comment: (eg. what problems occurs when playing it in DOSBox)<br>
	<textarea name="note" cols="40" rows="6"></textarea>

	<br><br>
	<input type="submit" name="submit" value="Add game to database">
	</form>';


	echo '</td></tr></table></td></tr></table></td></tr></table><br>';
}



if (isset($_GET['search']))
search_results($_GET['search']);

if (isset($_GET['letter']))
comp_mainlist($_GET['letter']);



echo '<br>';
get_support_stats();




echo '</td></tr></table><br><br>';

template_end();

?>
