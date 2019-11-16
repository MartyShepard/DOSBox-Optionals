<?php
// this src is written under the terms of the GPL-licence, see gpl.txt for futher details
	include("include/standard.inc.php");
	sstart();
//Currently assuming that users with download_management abilities can be trusted. Should be changed in the future
global $db;
if (isset($user) && ($user['priv']['download_management']==1))
{

	if (isset($_GET['removeID'],$_GET['catID']))

	{
		$removeID = mysqli_real_escape_string($db,intval(stripslashes($_GET['removeID'])));
		$catID = mysqli_real_escape_string($db,intval(stripslashes($_GET['catID'])));

		mysqli_query($db,"DELETE FROM download WHERE download.ID=$removeID");

		Header("Location: download.php?changeID=".$catID);
	}
	if (isset($_GET['adding']) &&$_GET['adding'] ==1)
	{
		$name = mysqli_real_escape_string($db,stripslashes($_POST['name']));
		$version = mysqli_real_escape_string($db,stripslashes($_POST['version']));
		$desc = mysqli_real_escape_string($db,stripslashes($_POST['description']));
		$url = mysqli_real_escape_string($db, stripslashes($_POST['url']));
		$catID = mysqli_real_escape_string($db, intval(stripslashes($_GET['catID'])));
		
		if ($name != '' AND $version != '' AND $url != '' AND $catID != '')
		{
			mysqli_query($db,"
			INSERT INTO download (name, url, description, version, catID, added)
			VALUES ('$name', '$url', '$desc', '$version', $catID, NOW())
			");
			
		    Header("Location: download.php?changeID=".(isset($_GET['catID'])?intval($_GET['catID']):"0"));
		}
		else
		      Header("Location: download.php?problem=1&changeID=".(isset($_GET['catID'])?intval($_GET['catID']):"0"));
	}
	if (isset($_GET['changing']) && $_GET['changing']==1)
	{
		$updateID = mysqli_real_escape_string($db,intval(stripslashes($_POST['updateID'])));
		$name = mysqli_real_escape_string($db,stripslashes($_POST['name']));
		$version = mysqli_real_escape_string($db,stripslashes($_POST['version']));
		$desc = mysqli_real_escape_string($db,stripslashes($_POST['description']));
		$url = mysqli_real_escape_string($db,stripslashes($_POST['url']));
		$catID = mysqli_real_escape_string($db,intval(stripslashes($_GET['catID'])));
		
		if ($name != '' AND $version != '' AND $url != '' AND $catID != '')
		{
			mysqli_query($db,"UPDATE download SET name='$name', version='$version', description='$desc', url='$url' WHERE ID=$updateID");
			
		    Header("Location: download.php?changeID=".(isset($_GET['catID'])?intval($_GET['catID']):"0"));
		}
		else
	      Header("Location: download.php?problem=1&changeID=".(isset($_GET['catID'])?intval($_GET['catID']):"0"));
	}
	if (isset($_GET['changeID']))
	{
	    $changeID=intval($_GET['changeID']);
		template_header();
		echo '<br><table width="100%"><tr><td width="14">&nbsp;</td><td>';// start of framespacing-table
                template_pagebox_start("Add/change download-category", 640);
	
			echo '<br><a href="download.php?main=1">Click here</a> to get back to the download page!<br><br>';
			download_change($changeID);
				
			echo '<br><br>
			
			<b>Add new item</b><br>
			

			<table cellspacing="0" cellpadding="0">
			<form action="download.php?adding=1&catID='.$changeID.'" method="POST" name="adding_statusID"><tr><td>
				<input type="text" name="name" maxlength="40" size="25">
			</td>

			<td>
				&nbsp;
			</td>

			<td>
				<input type="text" name="version" maxlength="20" size="5">
			</td>
			<td>
				&nbsp;
			</td>
			<td>
				<input type="text" name="description" maxlength="50" size="25">
			</td>
			<td>
				&nbsp;
			</td>
			<td>
				<input type="text" name="url" maxlength="150" size="40">
			</td>
			<td>
				&nbsp;&nbsp;<input type="submit" name="submit" value="Create item">
			</td>		
		</tr></form>
		</table><br>';
	
		template_pagebox_end();
		
		echo '</td></tr></table>';	// end of framespacing-table					
		template_end();
	}
}
/*
if (isset($_GET['changelog']) && ($_GET['changelog']==1))
{
		template_header();
	echo '
	<table width="100%" border="0" cellspacing="0" cellpadding="0">
		<tr align="left" valign="top"> 
			<td width="24">
				&nbsp;
			</td>
		
			<td> 
				<font face="Verdana, Arial, Helvetica, sans-serif" size="7"><b>Changelog</b><font size="2"><br><br>';
	
				show_changelog($_GET['showID']);
				
				echo '</font>
	
			</td>
			<td width="50">
				&nbsp;
			</td>
		</tr>
	</table><br><br>';
	
		template_end();	
}
*/
if (isset($_GET['main']) && ($_GET['main']==1))
{

		template_header();
		echo '<br><table width="100%"><tr><td width="14">&nbsp;</td><td>';// start of framespacing-table
//echo '<table><tr><td>';
echo '<span style="display: block; padding: 0 0 10px;">Click here to download <b>DOSBox 0.74-3 for your Operating System</b>, or to support us with a donation:</span>';
echo '<table width="100%"><tr><td width="50%">';
echo '<center>
<a href="https://sourceforge.net/projects/dosbox/files/latest/download"><img alt="Download DOSBox DOS Emulator" src="https://a.fsdn.com/con/app/sf-download-button" width=276 height=48></a>
</center>
';
echo '</td><td>';
//echo 'Click here to download the latest version of DOSBox for your Operating System, or to support us with a donation:';
//echo '</td></tr></table>';
echo '<center>
<form action="https://www.paypal.com/cgi-bin/webscr" method="post">
<input type="hidden" name="cmd" value="_s-xclick"> 
';
//<span class="support">Support DOSBox:&nbsp;</span>
echo '
<input type="hidden" name="hosted_button_id" value="RK4BA9ERN5AWL">
<input type="image" src="https://www.paypal.com/en_US/i/btn/x-click-but21.gif" border="0" name="submit" alt="PayPal - The safer, easier way to pay online!">
<img alt="" border="0" src="https://www.paypal.com/en_US/i/scr/pixel.gif" width="1" height="1">
</form></center>
';
echo '</td></tr></table>';
echo '<span style="display: block; padding:
               80px 0 10px;">Looking for a different version, a frontend or a translation?
Here\'s a full list of files:</span>';

		show_downloads(isset($user)?$user['priv']['download_management']:0);
				
		echo '</td></tr></table>';	// end of framespacing-table					
		template_end();
}
?>
