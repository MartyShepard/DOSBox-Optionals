<?php
// this src is written under the terms of the GPL-licence, see gpl.txt for futher details
	include("include/standard.inc.php");
	sstart();

if ($user['priv']['download_management']==1)
{

	if ($_GET['removeID'])
	{
		$removeID = mysql_escape_string(stripslashes($_GET['removeID']));
		$catID = mysql_escape_string(stripslashes($_GET['catID']));

		mysql_query("DELETE FROM download WHERE download.ID=$removeID");

		Header("Location: download.php?changeID=".$catID);
	}
	if ($_GET['adding']==1)
	{
		$name = mysql_escape_string(stripslashes($_POST['name']));
		$version = mysql_escape_string(stripslashes($_POST['version']));
		$desc = mysql_escape_string(stripslashes($_POST['description']));
		$url = mysql_escape_string(stripslashes($_POST['url']));
		$catID = mysql_escape_string(stripslashes($_GET['catID']));
		
		if ($name != '' AND $version != '' AND $url != '' AND $catID != '')
		{
			mysql_query("
			INSERT INTO download (name, url, description, version, catID, added)
			VALUES ('$name', '$url', '$desc', '$version', $catID, NOW())
			");
			
			Header("Location: download.php?changeID=".$_GET['catID']);
		}
		else
			Header("Location: download.php?problem=1&changeID=".$_GET['catID']);
	}
	if ($_GET['changing']==1)
	{
		$updateID = mysql_escape_string(stripslashes($_POST['updateID']));
		$name = mysql_escape_string(stripslashes($_POST['name']));
		$version = mysql_escape_string(stripslashes($_POST['version']));
		$desc = mysql_escape_string(stripslashes($_POST['description']));
		$url = mysql_escape_string(stripslashes($_POST['url']));
		$catID = mysql_escape_string(stripslashes($_GET['catID']));
		
		if ($name != '' AND $version != '' AND $url != '' AND $catID != '')
		{
			mysql_query("UPDATE download SET name='$name', version='$version', description='$desc', url='$url' WHERE ID=$updateID");
			
			Header("Location: download.php?changeID=".$_GET['catID']);
		}
		else
			Header("Location: download.php?problem=1&changeID=".$_GET['catID']);
	}
	if ($_GET['changeID'])
	{
		template_header();
		echo '<br><table width="100%"><tr><td width="14">&nbsp;</td><td>';// start of framespacing-table
                template_pagebox_start("Add/change download-category", 890);
	
			echo '<br><font face="Verdana, Arial, Helvetica, sans-serif" size="2"><a href="download.php?main=1">Click here</a> to get back to the download page!<br><br>';
			download_change($_GET['changeID']);
				
			echo '<br><br>
			
			<b>Add new item</b><font size="2"><br>
			

			<table cellspacing="0" cellpadding="0">
			<form action="download.php?adding=1&catID='.$_GET['changeID'].'" method="POST" name="adding_statusID"><tr><td>
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

if ($_GET['changelog']==1)
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
if ($_GET['main']==1)
{

		template_header();
		echo '<br><table width="100%"><tr><td width="14">&nbsp;</td><td>';// start of framespacing-table

		show_downloads($user['priv']['download_management']);
				
		echo '</td></tr></table>';	// end of framespacing-table					
		template_end();
}
?>
