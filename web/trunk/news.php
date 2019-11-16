<?php
// this src is written under the terms of the GPL-licence, see gpl.txt for futher details
	include("include/standard.inc.php");
	sstart();
	global $db;



if ((isset($_GET['newsID'],$_SESSION['userID']) && check_if_owner($_GET['newsID'],$_SESSION['userID'])) || (isset($user) && $user['priv']['news_management']==1))
{
	if (isset($_GET['changing_news']) && ( $_GET['changing_news']==1))
	{
		$text 		= mysqli_real_escape_string($db,stripslashes($_POST['text']));
		$updateID	= mysqli_real_escape_string($db,stripslashes($_POST['updateID']));
		
		mysqli_query($db,"UPDATE news SET text='$text' WHERE ID = $updateID");
		Header("Location: news.php?show_news=1");
	}
	if (isset($_GET['removing_news']) && ($_GET['removing_news']==1))
	{
		$newsID=mysqli_real_escape_string($db,stripslashes($_GET['newsID']));
		mysqli_query($db,"DELETE FROM news WHERE news.ID=$newsID");
		Header("Location: news.php?show_news=1");
	}
	if (isset($_GET['change_news']) && ($_GET['change_news']==1))
	{
		template_header();
		echo '<br><table width="100%"><tr><td width="14">&nbsp;</td><td>';// start of framespacing-table		
		
		template_pagebox_start("Changing news-item");			
		
		

		$query = mysqli_query($db,"
				SELECT
					text
				FROM
					news
				WHERE
					news.ID = ".mysqli_real_escape_string($db, stripslashes($_GET['newsID']))
				);

		$result = mysqli_fetch_row($query);

		echo '
			<form action="news.php?changing_news=1&newsID='.intval($_GET['newsID']).'" method="POST"><input name="updateID" type="hidden" value="'.intval($_GET['newsID']).'">

			News text:<br>
			<textarea name="text" cols="60" rows="12">'.$result[0].'</textarea>
			
			<br><br>
			<input type="submit" name="submit" value="Send changes to database">
			</form><br>
		
				</font>';
				
		template_pagebox_end();	
		echo '</td></tr></table>';	// end of framespacing-table					
		template_end();
	}
}

if (isset($user) && $user['priv']['post_news']==1)
{
	if (isset($_GET['posting_news'])  && ($_GET['posting_news']==1))
	{
		$text = mysqli_real_escape_string($db,stripslashes($_POST['text']));

		mysqli_query($db,"INSERT INTO news (text, ownerID, added) VALUES ('$text', ".$_SESSION['userID'].", NOW())");	
		Header("Location: news.php?success=1");
	}
	if (isset($_GET['post_news']) && ($_GET['post_news']==1))
	{
		template_header();
		echo '<br><table width="100%"><tr><td width="14">&nbsp;</td><td>';// start of framespacing-table		
		
		template_pagebox_start("Posting news-item");			
		
		echo '<br>
		
			<form action="news.php?posting_news=1" method="POST">
			News text:<br>
			<textarea name="text" cols="60" rows="12"></textarea>
			
			<br><br>
			<input type="submit" name="submit" value="Send post to database">
			</form>';					
				
		template_pagebox_end();	
		echo '</td></tr></table>';	// end of framespacing-table					
		template_end();
	}
}


if (isset($_GET['show_news']) && $_GET['show_news']==1)
{
	template_header();
	echo '<br>
<table width="100%">
	<tr>
		<td width="14">&nbsp;</td>
		<td>';// start of framespacing-table		
echo '
<form action="https://www.paypal.com/cgi-bin/webscr" method="post">
<input type="hidden" name="cmd" value="_s-xclick">
<span class="c1"><strong>Note:</strong> this website uses cookies for the user account-system!</span>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
<input type="hidden" name="hosted_button_id" value="RK4BA9ERN5AWL">
<input type="image" src="https://www.paypal.com/en_US/i/btn/x-click-but21.gif" border="0" name="submit" alt="PayPal - The safer, easier way to pay online!">
<img alt="" border="0" src="https://www.paypal.com/en_US/i/scr/pixel.gif" width="1" height="1">
</form><br>';
	
	if (isset($user) && ($user['priv']['post_news']==1))
		echo '<a href="news.php?post_news=1">Create news-item</a><br><br>';

	main_news(isset($user)?$user['priv']['news_management']:0);
		
	echo '</td></tr></table>';	// end of framespacing-table					
	template_end();
}
if (isset($_GET['success']) && $_GET['success']==1)
{	
	template_header();
	echo '<br><table width="100%"><tr><td width="14">&nbsp;</td><td>';// start of framespacing-table	
	template_pagebox_start("Posting news-item");			
		
		echo '<br>
				';
			echo '<b>News-item was posted successfully!</b><br><a href="news.php?post_news=1">Click here</a> to post one more news-item or <a href="news.php?show_news=1">click here</a> to show the main news-page again!<br><br>';
			
	template_pagebox_end();
	echo '</td></tr></table>';	// end of framespacing-table	
	template_end();
}
?>
