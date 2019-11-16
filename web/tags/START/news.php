<?php
// this src is written under the terms of the GPL-licence, see gpl.txt for futher details
	include("include/standard.inc.php");
	sstart();




if (check_if_owner($_GET['newsID'],$_SESSION['userID']) || $user['priv']['news_management']==1)
{
	if ($_GET['changing_news']==1)
	{
		$text 		= mysql_escape_string(stripslashes($_POST['text']));
		$updateID	= mysql_escape_string(stripslashes($_POST['updateID']));
		
		mysql_query("UPDATE news SET text='$text' WHERE ID = $updateID");
		Header("Location: news.php?show_news=1");
	}
	if ($_GET['removing_news']==1)
	{
		$newsID=mysql_escape_string(stripslashes($_GET['newsID']));
		mysql_query("DELETE FROM news WHERE news.ID=$newsID");
		Header("Location: news.php?show_news=1");
	}
	if ($_GET['change_news']==1)
	{
		template_header();
		echo '<br><table width="100%"><tr><td width="14">&nbsp;</td><td>';// start of framespacing-table		
		
		template_pagebox_start("Changing news-item");			
		
		echo '<font face="Verdana, Arial, Helvetica, sans-serif" size="2">';
		

		$query = mysql_query("
				SELECT
					text
				FROM
					news
				WHERE
					news.ID = ".mysql_escape_string(stripslashes($_GET['newsID']))
				);

		$result = mysql_fetch_row($query);

		echo '
			<form action="news.php?changing_news=1&newsID='.$_GET['newsID'].'" method="POST"><input name="updateID" type="hidden" value="'.$_GET['newsID'].'">

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

if ($user['priv']['post_news']==1)
{
	if ($_GET['posting_news']==1)
	{
		$text = mysql_escape_string(stripslashes($_POST['text']));

		mysql_query("INSERT INTO news (text, ownerID, added) VALUES ('$text', ".$_SESSION['userID'].", NOW())");	
		Header("Location: news.php?success=1");
	}
	if ($_GET['post_news']==1)
	{
		template_header();
		echo '<br><table width="100%"><tr><td width="14">&nbsp;</td><td>';// start of framespacing-table		
		
		template_pagebox_start("Posting news-item");			
		
		echo '<font face="Verdana, Arial, Helvetica, sans-serif" size="2"><br>
		
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


if ($_GET['show_news']==1)
{
	template_header();
	echo '<br><table width="100%"><tr><td width="14">&nbsp;</td><td>';// start of framespacing-table		

	echo '<font face="Verdana, Arial, Helvetica, sans-serif" size="2"><b>Note:</b> this website uses cookies for the user account-system!<br><br>';
	
	if ($user['priv']['post_news']==1)
		echo '<a href="news.php?post_news=1">Create news-item</a></font><br><br>';

	main_news($user['priv']['news_management']);
		
	echo '</td></tr></table>';	// end of framespacing-table					
	template_end();
}
if ($_GET['success']==1)
{	
	template_header();
	echo '<br><table width="100%"><tr><td width="14">&nbsp;</td><td>';// start of framespacing-table	
	template_pagebox_start("Posting news-item");			
		
		echo '<font face="Verdana, Arial, Helvetica, sans-serif" size="2"><br>
				';
			echo '<b>News-item was posted successfully!</b><br><a href="news.php?post_news=1">Click here</a> to post one more news-item or <a href="news.php?show_news=1">click here</a> to show the main news-page again!</font><br><br>';
			
	template_pagebox_end();
	echo '</td></tr></table>';	// end of framespacing-table	
	template_end();
}
?>
