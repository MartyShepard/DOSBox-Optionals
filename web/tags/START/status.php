<?php
// this src is written under the terms of the GPL-licence, see gpl.txt for futher details
	include("include/standard.inc.php");
	sstart();
if ($user['priv']['status_manage']==1)
{
	if ($_GET['changing']==1)
	{

		
		if (is_numeric($_POST['status']) AND $_POST['status'] < 101 AND $_POST['name'] != '' AND $_POST['note'] != '')
		{
			
			$updateID	= mysql_escape_string(stripslashes($_POST['updateID']));
			$name 		= mysql_escape_string(stripslashes($_POST['name']));
			$status 	= mysql_escape_string(stripslashes($_POST['status']));
			$note 		= mysql_escape_string(stripslashes($_POST['note']));
			
			mysql_query("UPDATE status_items SET name='$name', percent=$status, note='$note' WHERE ID=$updateID");
			
			Header("Location: status.php?changeID=".$_GET['catID']);
		}
		else
			Header("Location: status.php?problem=1&changeID=".$_GET['catID']);

	}

	if ($_GET['adding'])
	{
		if (is_numeric($_POST['status']) AND $_POST['status'] < 101 AND $_POST['name'] != '' AND $_POST['note'] != '')
		{
			$catID		= mysql_escape_string(stripslashes($_GET['catID']));
			$name 		= mysql_escape_string(stripslashes($_POST['name']));
			$status 	= mysql_escape_string(stripslashes($_POST['status']));
			$note 		= mysql_escape_string(stripslashes($_POST['note']));
			
			mysql_query("
					INSERT INTO 
						status_items (catID, name, percent, note)
					VALUES ($catID, '$name', $status, '$note')
				");
		
			Header("Location: status.php?changeID=".$_GET['catID']);
		}
		else
			Header("Location: status.php?problem=1&changeID=".$_GET['catID']);		
	}
	if ($_GET['removeID'])
	{
		$removeID = mysql_escape_string(stripslashes($_GET['removeID']));
		$catID = mysql_escape_string(stripslashes($_GET['catID']));

		mysql_query("DELETE FROM status_items WHERE status_items.ID=$removeID");

		Header("Location: status.php?changeID=".$catID);

	}
	if ($_GET['changeID'])
	{
		template_header();
		echo '<br><table width="100%"><tr><td width="14">&nbsp;</td><td>';// start of framespacing-table		
		
		template_pagebox_start("Changing DOSBox development status", 890);			
		
		echo '<font face="Verdana, Arial, Helvetica, sans-serif" size="2"><br>';
		
		
		if ($_GET['problem']==1)
			echo '<b>Something went wrong, please try again!</b><br>';
		
		echo '<a href="status.php?show_status=1">Click here</a> to get back to the status page!<br><br><br>';
		status_change($_GET['changeID']);
				
			echo '<br><br></font>
			
			<font face="Verdana, Arial, Helvetica, sans-serif" size="2"><b>Add new item</b><font size="2"><br>
			

			<table cellspacing="0" cellpadding="0">
			<form action="status.php?adding=1&catID='.$_GET['changeID'].'" method="POST" name="adding_statusID"><tr><td>
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
				&nbsp;&nbsp;<input type="submit" name="submit" value="Create item">
			</td>		
		</tr></form>
		</table>


			
			<br></font>';
			
			
		template_pagebox_end();	
		echo '</td></tr></table>';	// end of framespacing-table					
		template_end();

	}
}
if ($_GET['show_status']==1)
{	
	template_header();
	
	echo '<br><table width="100%"><tr><td width="14">&nbsp;</td><td>';// start of framespacing-table
		echo '<font face="Verdana, Arial, Helvetica, sans-serif" size="2">
		This shows a basic overview of the different systems emulated in DOSBox.<br><br>';
				
	show_status_db($user['priv']['status_manage']);
	
		
	echo '<br>';	
		
	echo '</td></tr></table>';	// end of framespacing-table					
	template_end();

}
?>
