<?php
// this src is written under the terms of the GPL-licence, see gpl.txt for futher details
	include("include/standard.inc.php");
	global $db;
	sstart();
if (isset($user) && $user['priv']['status_manage']==1)
{
	if (isset($_POST['updateID'],$_POST['name'],$_POST['status']) && $_GET['changing']==1)
	{

		
		if (is_numeric($_POST['status']) AND $_POST['status'] < 101 AND $_POST['name'] != '' AND $_POST['note'] != '')
		{
			
			$updateID	= mysqli_real_escape_string( $db, stripslashes($_POST['updateID']));
			$name 		= mysqli_real_escape_string( $db, stripslashes($_POST['name']));
			$status 	= mysqli_real_escape_string( $db, stripslashes($_POST['status']));
			$note 		= mysqli_real_escape_string( $db, stripslashes($_POST['note']));
			
			mysqli_query( $db, "UPDATE status_items SET name='$name', percent=$status, note='$note' WHERE ID=$updateID");
			
			Header("Location: status.php?changeID=".intval($_GET['catID']));
		}
		else
			Header("Location: status.php?problem=1&changeID=".intval($_GET['catID']));

	}

	if (isset($_GET['catID'],$_POST['name'],$_POST['status']) && $_GET['adding'])
	{
		if (is_numeric($_POST['status']) AND $_POST['status'] < 101 AND $_POST['name'] != '' AND $_POST['note'] != '')
		{
			$catID		= mysqli_real_escape_string( $db, stripslashes($_GET['catID']));
			$name 		= mysqli_real_escape_string( $db, stripslashes($_POST['name']));
			$status 	= mysqli_real_escape_string( $db, stripslashes($_POST['status']));
			$note 		= mysqli_real_escape_string( $db, stripslashes($_POST['note']));
			
			mysqli_query($db,"
					INSERT INTO 
						status_items (catID, name, percent, note)
					VALUES ($catID, '$name', $status, '$note')
				");
		
			Header("Location: status.php?changeID=".intval($_GET['catID']));
		}
		else
			Header("Location: status.php?problem=1&changeID=".intval($_GET['catID']));		
	}
	if (isset($_GET['removeID']))
	{
		$removeID = mysqli_escape_string( $db, stripslashes($_GET['removeID']));
		$catID = mysqli_escape_string( $db, stripslashes($_GET['catID']));

		mysqli_query( $db, "DELETE FROM status_items WHERE status_items.ID=$removeID");

		Header("Location: status.php?changeID=".$catID);

	}
	if (isset($_GET['changeID']) &&is_numeric($_GET['changeID']))
	{
		template_header();
		echo '<br><table width="100%"><tr><td width="14">&nbsp;</td><td>';// start of framespacing-table		
		
		template_pagebox_start("Changing DOSBox development status", 690);			
		
		echo '<br>';
		
		
		if ($_GET['problem']==1)
			echo '<b>Something went wrong, please try again!</b><br>';
		
		echo '<a href="status.php?show_status=1">Click here</a> to get back to the status page!<br><br><br>';
		status_change(intval($_GET['changeID']));
				
			echo '<br><br>
			
			<strong>Add new item</strong><br>
			

			<table cellspacing="0" cellpadding="0">
			<form action="status.php?adding=1&catID='.intval($_GET['changeID']).'" method="POST" name="adding_statusID"><tr><td>
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
if (isset($_GET['show_status']) && $_GET['show_status']==1)
{	
	template_header();
	
	echo '<br><table width="100%"><tr><td width="14">&nbsp;</td><td>';// start of framespacing-table
		echo 'This shows a basic overview of the different systems emulated in DOSBox.<br><br>';
				
	show_status_db(isset($user)?$user['priv']['status_manage']:0);
	
		
	echo '<br>';	
		
	echo '</td></tr></table>';	// end of framespacing-table					
	template_end();

}
?>
