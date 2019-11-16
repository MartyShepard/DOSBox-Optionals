<?php
// this src is written under the terms of the GPL-licence, see gpl.txt for futher details
include("include/standard.inc.php");
sstart();
global $db;

if (isset($_GET['changeversion']) && $_GET['changeversion']==1)
{
	if (isset($_SESSION['userID'],$_POST['statusID'],$_POST['percent']))
	mysqli_query($db,"UPDATE status_games SET status=".intval($_POST['percent'])." WHERE status_games.ID=".intval($_POST['statusID']));

	Header("Location: comp_list.php?changeID=".intval($_POST['gameID'])."&letter=".letter_check());
}
if (isset($_GET['removeMSG_ID']))
{
	if (isset($user) && $user['priv']['compat_list_manage']==1)
	{
		$msgID =mysqli_real_escape_string($db,intval(stripslashes($_GET['removeMSG_ID'])));
		mysqli_query($db,"DELETE FROM list_comment WHERE ID=$msgID");
	}
	Header("Location: comp_list.php?showID=".intval($_GET['gameID'])."&letter=".letter_check());
}
if (isset($_GET['showID1']) && $_GET['showID1'] != 0)
	header("Location: comp_list.php?showID=".intval($_GET['showID1'])."&letter=".letter_check());
if (isset($_GET['removeVERSION_ID'],$_GET['gameID'],$_SESSION['userID']))
{
	$versionID	=mysqli_real_escape_string($db,intval(stripslashes($_GET['removeVERSION_ID'])));
	$gameID		=intval($_GET['gameID']);
	$letter		=letter_check();
	$userID		=$_SESSION['userID'];

	if (isset($user) && $user['priv']['compat_list_manage']==1)
	mysqli_query($db,"DELETE FROM status_games WHERE status_games.ID=$versionID");

	Header("Location: comp_list.php?changeID=".$gameID."&letter=".$letter);
}
if (isset($_GET['addVersion'],$_POST['gameID'],$_POST['versionID'],$_POST['percent'],$_SESSION['userID']) && $_GET['addVersion']==1)
{
	$gameID		=mysqli_real_escape_string($db, intval(stripslashes($_POST['gameID'])));
	$versionID	=mysqli_real_escape_string($db, intval(stripslashes($_POST['versionID'])));
	$status		=mysqli_real_escape_string($db, intval(stripslashes($_POST['percent'])));
	$userID		=$_SESSION['userID'];

	if (isset($_SESSION['userID']) AND $versionID != '-')
	{
		mysqli_query($db,"

		INSERT INTO
		status_games (gameID, versionID, status)
		VALUES ($gameID, $versionID, $status)
		");

		#mysql_query("UPDATE list_game SET ownerID=$userID WHERE ID=$gameID");
	}
	Header("Location: comp_list.php?changeID=".$gameID."&letter=".letter_check());
}
if (isset($_GET['post_comment'],$_POST['gameID'],$_SESSION['userID']) && $_GET['post_comment']==1)
{
	$gameID		=mysqli_real_escape_string($db,intval(stripslashes($_POST['gameID'])));
	$subject	=mysqli_real_escape_string($db,stripslashes(htmlspecialchars($_POST['subject'])));
	$text		=mysqli_real_escape_string($db,stripslashes(htmlspecialchars($_POST['text'])));
	$userID		=$_SESSION['userID'];

	if (isset($_SESSION['userID']))
	{
		if ($_POST['subject'] == '' || $_POST['text'] == '' || strlen($_POST['subject'])>60 || strlen($_POST['text'])>1024)
		Header("Location: comp_list.php?problem=1&post_newMSG=1&showID=".$gameID."&letter=".letter_check()."#post_comment");
		else
		{
			mysqli_query($db,"

			INSERT INTO
			list_comment (gameID, ownerID, subject, text, datetime)
			VALUES
			($gameID, $userID, '$subject', '$text', NOW())
			");

			Header("Location: comp_list.php?showID=".$gameID."&letter=".letter_check());
		}
	}

}
if (isset($_GET['changing'],$_POST['ID'],$_POST['name'],$_POST['publisher'],$_POST['year'],$_SESSION['userID']) && $_GET['changing']==1)
{

	$changeID	= intval($_POST['ID']);
	$letter		= letter_check();
	$name 		= mysqli_real_escape_string($db, stripslashes(htmlspecialchars($_POST['name'])));
	$publisher 	= mysqli_real_escape_string($db, stripslashes(htmlspecialchars($_POST['publisher'])));
	$released 	= mysqli_real_escape_string($db, stripslashes(htmlspecialchars($_POST['year'])));
	$moby 		= mysqli_real_escape_string($db, stripslashes(htmlspecialchars($_POST['moby'])));
	$userID		= $_SESSION['userID'];
	$first_char 	= $name{0};


	if (check_game_owner($changeID, $_SESSION['userID'])==1 || (isset($user) && $user['priv']['compat_list_manage']==1))
	{
		if ($name == '' || $publisher == '' || !is_numeric($released))
		Header("Location: comp_list.php?letter=".$letter."&problem=1&changeID=".$changeID);
		else
		{

//			if ($moby != '' AND verifyurl($moby)==false)
//			Header("Location: comp_list.php?letter=".$letter."&problem=1&changeID=".$changeID);
			//don't support the moby url
			$moby = '';
			mysqli_query($db,"UPDATE list_game SET name='$name', publisher='$publisher', first_char='$first_char', released=$released, moby_url='$moby' WHERE ID=$changeID");
			Header("Location: comp_list.php?changeID=".$changeID."&letter=".$letter);
		}
	}
}

if (isset($_GET['removeID'],$_SESSION['userID']) && $_GET['removeID'])
{
	if (check_game_owner($_GET['removeID'], $_SESSION['userID'])==1 || (isset($user) && $user['priv']['compat_list_manage']==1))
	{
		$gameID = mysqli_real_escape_string($db,intval(stripslashes($_GET['removeID'])));

		mysqli_query($db,"DELETE FROM list_game WHERE ID=$gameID");
		mysqli_query($db,"DELETE FROM status_games WHERE gameID=$gameID");
		mysqli_query($db, "DELETE FROM list_comment WHERE gameID=$gameID");

		if ( isset( $_GET['letter'] ) && is_numeric( $_GET['letter'] ) )
		Header("Location: comp_list.php?letter=num");
		else
		Header("Location: comp_list.php?letter=".letter_check());
	}
}
if (isset($_SESSION['userID']) && $_SESSION['userID'])
{
	if (isset($_GET['posting_new']) && ($_GET['posting_new']==1) && isset($_POST['name'],$_POST['publisher']))
	{
		$name 		= mysqli_real_escape_string($db,stripslashes(htmlspecialchars($_POST['name'])));
		$publisher 	= mysqli_real_escape_string($db,stripslashes(htmlspecialchars($_POST['publisher'])));
		$percent 	= mysqli_real_escape_string($db,stripslashes(htmlspecialchars($_POST['percent'])));
		$released 	= mysqli_real_escape_string($db,stripslashes(htmlspecialchars($_POST['year'])));
		$comment	= mysqli_real_escape_string($db,stripslashes(htmlspecialchars($_POST['note'])));
		$version 	= mysqli_real_escape_string($db,stripslashes(htmlspecialchars($_POST['version'])));
		$moby 		= mysqli_real_escape_string($db,stripslashes(htmlspecialchars($_POST['moby'])));
		$userID		= $_SESSION['userID'];
		$first_char 	= ($name =='')?'A':$name{0};
		$letter = letter_check();
		$query = mysqli_query($db,"Select name from list_game where name='$name'");
		$num = mysqli_num_rows($query);

		if ($num > 0 || $name == '' || $publisher == '' || !is_numeric($released) || strlen($comment)>1024)
			Header("Location: comp_list.php?post_new=1&error=1&letter=".$letter);
		else
		{
//			if (verifyurl($moby)==0 AND $moby != '')
//			Header("Location: comp_list.php?post_new=1&error=1&letter=".$letter);

			$moby_url='';

			mysqli_query($db,"
			INSERT INTO list_game
			(ownerID, added, name, publisher, version, first_char, released, moby_url)
			VALUES($userID, NOW(), '$name', '$publisher', '$version', '$first_char', $released, '$moby_url')");

			$AUTO_INCREMENT_ID = mysqli_insert_id($db);

			mysqli_query($db,"INSERT INTO status_games (gameID,versionID,status) VALUES($AUTO_INCREMENT_ID, $version, $percent)");

			if ($comment != '')
			{
				$subject = 'Note:';

				mysqli_query($db,"

				INSERT INTO list_comment
				(gameID, ownerID, subject, text, datetime)
				VALUES ($AUTO_INCREMENT_ID, $userID, '$subject', '$comment', NOW())
				");
			}

			Header("Location: comp_list.php?showID=".$AUTO_INCREMENT_ID."&letter=".$letter);
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


if (isset($_GET['showID']) && intval($_GET['showID']) > 0 AND (!isset($_GET['post_new']) || $_GET['post_new'] != 1))
{
	if (!isset($_SESSION['userID']))
		echo 'You must login before posting comments/adding games, <a href="https://www.dosbox.com/login.php">click here</a> to get to the login page!<br><br>';

	comp_show_ID(intval($_GET['showID']));
}
echo '<br/>';
if (isset($_GET['showID'])  && intval($_GET['showID']) > 0 AND (!isset($_GET['post_new']) || $_GET['post_new'] != 1))
{
	get_msg_threads(intval($_GET['showID']));


	if (isset($_GET['post_newMSG']) && $_GET['post_newMSG']==1)
	{
		echo '<br>';
		echo '<a name="post_comment">';
		write_comment();

		echo '<br>';
	}

}

if (isset($_GET['changeID']) && $_GET['changeID'])
{
	$gameID = mysqli_real_escape_string($db,intval(stripslashes($_GET['changeID'])));
	$query = mysqli_query($db,"SELECT ID, ownerID, name, publisher, version, released, moby_url FROM list_game WHERE ID = $gameID");

	$result = mysqli_fetch_row($query);


	echo '<table width="638" cellspacing="0" cellpadding="1" bgcolor="#000000"><tr><td valign="top" align="left"><table cellspacing="4" cellpadding="0" width="100%" bgcolor="#355787"><tr><td>
	<table cellspacing="0" cellpadding="0" width="100%"><tr><td valign="top" align="left">Updating game-post</td></tr></table></td>
	</tr></table></td></tr></table><table width="638" cellspacing="0" cellpadding="1" bgcolor="#000000"><tr><td valign="top" align="left"><table cellspacing="4" cellpadding="0" width="100%" bgcolor="#113466"><tr>
	<td><table cellspacing="0" cellpadding="0" width="100%"><tr><td valign="top" align="left">';

	if ((isset($_SESSION['userID']) && check_game_owner($_GET['changeID'], $_SESSION['userID'])==1) || (isset($user) && $user['priv']['compat_list_manage']==1))
	{
		if (isset($_GET['error'])  &&  $_GET['error']==1)
		echo '<b>Error - this form must be filled in accurate!</b>';

		echo '<form action="comp_list.php?changing=1" method="POST">
		<input type="hidden" name="ID" value="'.$result[0].'">
		<input type="hidden" name="ownerID" value='.$result[1].'>
		<input type="hidden" name="letter" value="'.letter_check().'">
		<table cellspacing="0" cellpadding="0" width="100%">
		<tr>
		<td valign="top">
		Game title:
		</td>
		<td width="10">
		&nbsp;
		</td>
		<td valign="top">
		Publisher/Developer:
		</td>

		</tr>
		<tr>
		<td valign="top">
		<input type="text" name="name" size="38" maxlength="35" value="'.$result[2].'">
		</td>
		<td width="10">
		&nbsp;
		</td>
		<td valign="top">
		<input type="text" name="publisher" size="38" maxlength="20" value="'.$result[3].'">
		</td>

		</tr>
		<tr>
		<td colspan="3">
		&nbsp;
		</td>
		</tr>
		<tr>
		<td valign="top">
		Year released:
		</td>
		<td width="10">
		&nbsp;
		</td>
		<td valign="top">
		<!--Moby-games link: -->
		</td>

		</tr>
		<tr>
		<td valign="top">
		<input type="text" name="year" size="38" maxlength="4" value="'.$result[5].'">
		</td>
		<td width="10">
		&nbsp;
		</td>
		<td valign="top">
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
		<br><br><hr line color="#FFFFFF">
		</form>';
	}
	else
	{
		echo '<br>You are logged in as a "normal" user, and are not allowed to edit these settings, however you may still add/edit DOSBox version-support for this game.<br><br>Click on the buttons to apply the new settings!<br><br>
		<table cellspacing="0" cellpadding="0" width="100%">
		<tr>
		<td valign="top">
		Game title:
		</td>
		<td width="10">
		&nbsp;
		</td>
		<td valign="top">
		Publisher/Developer:
		</td>

		</tr>
		<tr>
		<td valign="top">
		<input type="text" name="name" size="38" maxlength="35" value="'.$result[2].'" disabled>
		</td>
		<td width="10">
		&nbsp;
		</td>
		<td valign="top">
		<input type="text" name="publisher" size="38" maxlength="20" value="'.$result[3].'" disabled>
		</td>

		</tr>
		<tr>
		<td colspan="3">
		&nbsp;
		</td>
		</tr>
		<tr>
		<td valign="top">
		Year released:
		</td>
		<td width="10">
		&nbsp;
		</td>
		<td valign="top">
		<!--Moby-games link: -->
		</td>

		</tr>
		<tr>
		<td valign="top">
		<input type="text" name="year" size="38" maxlength="4" value="'.$result[5].'" disabled>
		</td>
		<td width="10">
		&nbsp;
		</td>
		<td valign="top">
		<input type="hidden" name="moby" size="38" maxlength="65" value="'.$result[6].'">
		</td>

		</tr>
		<tr>
		<td colspan="3">
		&nbsp;
		</td>
		</tr>
		</table>
		<hr line color="#FFFFFF">';
	}



	change_version_compatibility_form($result[0]);

	echo '<hr line color="#FFFFFF"><br><form name="addversion" method="POST" action="comp_list.php?addVersion=1"><input type="hidden" name="gameID" value="'.intval($_GET['changeID']).'">';

	add_version_compatibility_form($result[0]);
	choose_percentage();
	echo '&nbsp;<input type="hidden" name="letter" value="'.letter_check().'"><input type="submit" value="Add version-support"><br><br>
	<br><a href="comp_list.php?showID='.intval($_GET['changeID']).'&letter='.letter_check().'">Click here to get back!<br></form>';


	echo '</td></tr></table></td></tr></table></td></tr></table><br>';
}

if (isset($_GET['post_new']) AND $_GET['post_new']==1 AND isset($_SESSION['userID']))
{
	echo '<table width="638" cellspacing="0" cellpadding="1" bgcolor="#000000"><tr><td valign="top" align="left"><table cellspacing="4" cellpadding="0" width="100%" bgcolor="#355787"><tr><td>
	<table cellspacing="0" cellpadding="0" width="100%"><tr><td valign="top" align="left">Adding new game</td></tr></table></td>
	</tr></table></td></tr></table><table width="638" cellspacing="0" cellpadding="1" bgcolor="#000000"><tr><td valign="top" align="left"><table cellspacing="4" cellpadding="0" width="100%" bgcolor="#113466"><tr>
	<td><table cellspacing="0" cellpadding="0" width="100%"><tr><td valign="top" align="left">';

	if (isset($_GET['error']) && $_GET['error']==1)
	echo '<b>Error - this form must be filled in correctly!</b>';

        echo 'If the game is <b>already present</b> in the database.
	Select that game and press the little <b>square</b> next to its name to add a new
	version or another comment.<br><br>';
	echo '<form action="comp_list.php?posting_new=1" method="POST">
	<input type="hidden" name="letter" value="'.letter_check().'">
	<table cellspacing="0" cellpadding="0" width="100%">
	<tr>
	<td valign="top">
	Game title:
	</td>
	<td width="10">
	&nbsp;
	</td>
	<td valign="top">
	Publisher/Developer:
	</td>

	</tr>
	<tr>
	<td valign="top">
	<input type="text" name="name" size="38" maxlength="35">
	</td>
	<td width="10">
	&nbsp;
	</td>
	<td valign="top">
	<input type="text" name="publisher" size="38" maxlength="20">
	</td>

	</tr>
	<tr>
	<td colspan="3">
	&nbsp;
	</td>
	</tr>
	<tr>
	<td valign="top">
	Year released:
	</td>
	<td width="10">
	&nbsp;
	</td>
	<td valign="top">
	<!--Moby-games link: -->
	</td>

	</tr>
	<tr>
	<td valign="top">
	<input type="text" name="year" size="38" maxlength="4">
	</td>
	<td width="10">
	&nbsp;
	</td>
	<td valign="top">
	<input type="hidden" name="moby" size="38" maxlength="20">
	</td>

	</tr>
	<tr>
	<td colspan="3">
	&nbsp;
	</td>
	</tr>
	<tr>
	<td valign="top">
	DOSBox version:
	</td>
	<td width="10">
	&nbsp;
	</td>
	<td valign="top">
	Compatibility: (specify DOSBox support: 0-100%)
	</td>

	</tr>
	<tr>
	<td valign="top">';
	get_versions();
	echo '</td>
	<td width="10">
	&nbsp;
	</td>
	<td valign="top">';
	choose_percentage();

	echo '</td>

	</tr>



	</table>
	<br>

	Comment: (e.g. what problems occur when playing it in DOSBox)<br>
	<textarea name="note" cols="40" rows="6"></textarea>

	<br><br>
	<input type="submit" name="submit" value="Add game to database">
	</form>';


	echo '</td></tr></table></td></tr></table></td></tr></table><br>';
}



if (isset($_GET['search']))
search_results($_GET['search']);

if (isset($_GET['letter'])){
	if(/*$_GET['letter'] == 'a' && */ !isset($_GET['showID'])){
		echo <<<EOT
<div style="width: 630px;" class="caption">
 <p style="padding:6px;margin:0;"><a href="/">DOSBox</a> does NOT host these games. This list is a compatibility list. If you are looking for games, you can visit <a href="http://www.classicdosgames.com" target="blank">www.classicdosgames.com</a> or <a href="https://www.gog.com/?pp=b3f0c7f6bb763af1be91d9e74eabfeb199dc1f1f" class="goglink" target="_blank"><span class="gog1">GOG</span><span class="gog2">.COM</span></a>.</p>
</div>
<br/>
EOT;
	}
	comp_mainlist(letter_check());
}


echo '<br>';
get_support_stats();




echo '</td></tr></table><br><br>';

template_end();

?>
