<?php
// this src is written under the terms of the GPL-licence, see gpl.txt for futher details
include("include/standard.inc.php");
global $db;
sstart();

exit();	
if (isset($user) && $user['priv']['screen_manage']==1)
{
	if ($_GET['upload_file']==1)
	{		
		if (is_uploaded_file($_FILES['userfile']['tmp_name']))
		{
			ereg( ".*\.([a-zA-z0-9]{0,5})$", $_FILES['userfile']['name'], $extension);
			
			if ($extension[1] == 'png' || $extension[1] == 'PNG')
			{	
				ResizePNG($_FILES['userfile']['tmp_name'], $settings['obsolete'].'/tmp/thumb/'.$_FILES['userfile']['name'],200, 132);
				move_uploaded_file($_FILES['userfile']['tmp_name'], $settings['obsolete'].'/tmp/big/'.$_FILES['userfile']['name']);
			}

		}
		@unlink($_FILES['userfile']['tmp_name']); 
		
	}
	if (isset($_GET['removeID']))
	{
		
		$removeID	= mysqli_escape_string($db,stripslashes($_GET['removeID']));

                
		$query = mysqli_query($db,"SELECT COUNT(ID) FROM screenshots WHERE screenshots.ID=$removeID");
		$result = mysqli_fetch_row($query);
		
		if ($result[0] == 1)
		{
			$query = mysqli_query($db,"SELECT ID FROM screenshots WHERE ID=$removeID");
			$result = mysqli_fetch_row($query);
			
			mysqli_query($db,"DELETE FROM screenshots WHERE ID=$removeID");
			unlink($settings['obsolete'].'/screenshots/thumb/'.$result[0].'.png');
			unlink($settings['obsolete'].'/screenshots/big/'.$result[0].'.png');
			Header("Location: screenshots.php?page=".$page);
		}
		else
			Header("Location: screenshots.php?not_exist=1&page=".$page);		
		
	}  	

	if ($_GET['changing']==1)
	{
		
		$page		= $_POST['page'];
		$changeID	= mysqli_escape_string($db,stripslashes($_POST['changeID']));
		$description	= mysqli_escape_string($db,stripslashes($_POST['description']));

                
		$query = mysqli_query($db,"SELECT COUNT(ID) FROM screenshots WHERE screenshots.ID=$changeID");
		$result = mysqli_fetch_row($query);
		
		if ($result[0] == 1)
		{
			mysqli_query($db,"UPDATE screenshots SET text='$description' WHERE ID=$changeID");		
			Header("Location: screenshots.php?page=".$page);
		}
		else
			Header("Location: screenshots.php?not_exist=1&page=".$page);		
		
	}       
	

	if ($_GET['adding']==1)
	{
		$filename	= mysqli_escape_string($db,stripslashes($_POST['insert']));
		$text		= mysqli_escape_string($db,stripslashes($_POST['description']));

		if ($result[0] == 0)
		{
			mysqli_query($db,"
			INSERT INTO screenshots
				(text, datetime)
			VALUES ('$text', NOW())
			");
			
                       
			$parentID = mysqli_insert_id($db);
                        
                        
			copy($settings['obsolete'].'/tmp/thumb/'.$filename, $settings['obsolete'].'/screenshots/thumb/'.$parentID.'.png');
				unlink($settings['obsolete'].'/tmp/thumb/'.$filename);
			
			copy($settings['obsolete'].'/tmp/big/'.$filename, $settings['obsolete'].'/screenshots/big/'.$parentID.'.png');
				unlink($settings['obsolete'].'/tmp/big/'.$filename);
		}
		Header("Location: screenshots.php?page=".$_POST['page']."#add");
	}


	template_header();
	echo '<br><table width="100%"><tr><td width="14">&nbsp;</td><td>';// start of framespacing-table		

	if (isset($_GET['insert']))
	{
		template_pagebox_start("Adding screenshot in database", 625);
		
		echo '
		
		<form action="screenshots.php?adding=1" method="POST" name="newscreen">
		<input type="hidden" value="'.$_GET['insert'].'" name="insert">
		
		<table cellspacing="0" cellpadding="0" width="100%">
		<tr>
			<td width="200" valign="top">
				<a href="tmp/big/'.$_GET['insert'].'" target="_blank"><img src="tmp/thumb/'.$_GET['insert'].'" border="0"></a>
		        </td>
		        <td width="10">
		        	&nbsp;
		        </td>
		        <td valign="top"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
				<br>Description/Game-title:<br>
				<input type="hidden" name="page" value="'.$_GET['page'].'">
				<input type="text" name="description" maxlength="60" size="50"><br><br>
				<input type="submit" value="Add game to database">
				
		        </td>
		</tr>
		</table>';
		template_pagebox_end(); echo '</form>';
	}
	if (isset($_GET['changeID']))
	{
		
		$changeID = mysqli_escape_string($db,stripslashes($_GET['changeID']));
		
		$query = mysqli_query($db,"SELECT ID, text FROM screenshots WHERE ID=$changeID");
		
		if (mysqli_num_rows($query))
		{
			$result = mysqli_fetch_row($query);
			template_pagebox_start("Changing screenshot-post", 625);
			
			echo '
			
			<form action="screenshots.php?changing=1" method="POST" name="changescreen">
			<input type="hidden" value="'.$changeID.'" name="changeID">
			<input type="hidden" value="'.$_GET['page'].'" name="page">
			
			<table cellspacing="0" cellpadding="0" width="100%">
			<tr>
				<td width="200" valign="top">
					<a href="screenshots/big/'.$result[0].'.png" target="_blank"><img src="screenshots/thumb/'.$result[0].'.png" border="0"></a>
			        </td>
			        <td width="10">
			        	&nbsp;
			        </td>
			        <td valign="top"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">
					<br>Description/Game-title:<br>
					<input type="text" name="description" maxlength="60" size="50" value="'.$result[1].'"><br><br>
					<input type="submit" value="Update screenshot"><br>
					<br><a href="screenshots.php?removeID='.$result[0].'&page='.$_GET['page'].'">Remove screenshot</a> (remove database-post <b>AND</b> image)
					
			        </td>
			</tr>
			</table>	        
			';
			template_pagebox_end(); echo '</form>';
		}       
		
	}	
	
	template_pagebox_start("Current screenshots", 625);
		display_screenshots_current($_GET['page']);
	template_pagebox_end();		
	
	template_pagebox_start("Screenshots waiting to be used", 625);	
		echo '<a name="add">';
		display_screenshots_add();
	template_pagebox_end();		


	if ($_GET['upload_new']=1)
	{
		template_pagebox_start("Upload new image", 625);
		echo '
			<a name="upload">
			<font face="Verdana, Arial, Helvetica, sans-serif" size="2">
			<form enctype="multipart/form-data" action="screenshots.php?page='.$_GET['page'].'&upload_file=1#upload" method="post">
			<input type="hidden" name="MAX_FILE_SIZE" value="206140"><input type="hidden" name="page" value="'.$_GET['page'].'">
			Upload this screenshot:<br>
			<input name="userfile" type="file">&nbsp;<input type="submit" value="Send file"><br><br>
			(must be in .png format)
			</form>	
		';
		
		template_pagebox_end();		
	}



	echo '</td></tr></table>';	// end of framespacing-table					
	template_end();
}


function display_screenshots_current($page)
{
	if ($handle = opendir('screenshots/thumb/'))
	{
		echo '<table>';

				$page = mysqli_escape_string($db,stripslashes($page));

				
				$count_query = mysqli_query($db,"SELECT COUNT(ID) FROM screenshots");
				$count = mysqli_fetch_row($count_query);
				
				$maxpages=floor(($count[0]-1)/6);
                                

                                
                                $query = mysqli_query($db,"SELECT ID, text FROM screenshots ORDER BY screenshots.datetime DESC LIMIT ".($page*6).",6");
                                
                                $num = mysqli_num_rows($query);
                                
			        while ($result = mysqli_fetch_row($query))
			        {
					if ($x==0)
						echo "<tr>";
					
					echo '<td><a href="screenshots.php?changeID='.$result[0].'&page='.$_GET['page'].'"><img src="screenshots/thumb/'.$result[0].'.png" border="0" alt="'.$result[1].'"></a></td>';
					
					if ($x==2)
					{
						echo "</tr>";
						$x=0;
					}
					else
						$x++;
				}
				
		echo '</table>';
		
		if ($num++ == 0)
			echo '<font face="Verdana, Arial, Helvetica, sans-serif" size="2"><i>No images waiting to be added!</i>';
		else
		{		
		echo '<table cellspacing="0" cellpadding="0" width="200" align="center">
		<tr>
			<td align="left" width="17">';
						
		if (!$page==0)
			echo '<a href="screenshots.php?page='.($page-1).'"><img src="site_images/arrow_left.gif" border="0" alt="Browse screenshots-archive"></a>';
		else
		        echo '<img src="site_images/arrow_left_nofuther.gif" border="0">';
		
		echo '</td><td align="center"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">browse screen-archive</td><td align="right" width="17">';
		
		if ($page<$maxpages)
			echo '<a href="screenshots.php?page='.($page+1).'"><img src="site_images/arrow_right.gif" border="0" alt="Browse screenshots-archive"></a>';
		else
			echo '<img src="site_images/arrow_right_nofuther.gif" border="0">';   
			
			echo '</td></tr></table>';
		}		
				
		
	}
}
function display_screenshots_add()
{
	if ($handle = opendir('tmp/thumb/'))
	{
		echo '<table>';
		while (false !== ($file = readdir($handle)))
		{
			if ($file != "." && $file != "..")
			{
					if ($x==0)
						echo "<tr>";
					
					echo '<td><a href="screenshots.php?page='.$_GET['page'].'&insert='.$file.'"><img src="tmp/thumb/'.$file.'" border="0"></td>';
					
					
					if ($x==2)
					{
						echo "</tr>";
						$x=0;
					}
					else
						$x++;
						
				$num++;
			}
		}
	}
	closedir($handle);
	echo '</table>';
	
		if ($num++ == 0)
			echo '<font face="Verdana, Arial, Helvetica, sans-serif" size="2"><i>No images waiting to be added!</i>';
}
function ResizePNG($source, $destination, $new_w, $new_h)
{ 
	header("Content-type: image/png");
	$dst_img=ImageCreate($new_w,$new_h);
	$src_img=ImageCreateFromPng($source);
	ImageCopyResized($dst_img,$src_img,0,0,0,0,$new_w,$new_h,ImageSX($src_img),ImageSY($src_img));
	ImagePng($dst_img, $destination); 
        Imagedestroy($src_img);
}
?>
