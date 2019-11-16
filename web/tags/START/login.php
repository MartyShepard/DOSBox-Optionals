<?php
// this src is written under the terms of the GPL-licence, see gpl.txt for futher details
	include("include/standard.inc.php");
	sstart();
	
	if ($_GET['destroy']==1)
	{
		SESSION_DESTROY();
		Header("Location: login.php");
	}
		
	if ($_GET['login']==1)
	{
		login($_POST['nickname'], $_POST['password']);
	}	

	template_header();
	echo '<br><table width="100%"><tr><td width="14">&nbsp;</td><td>';// start of framespacing-table		
	
	template_pagebox_start("Login", 550);			
	
	echo '<font face="Verdana, Arial, Helvetica, sans-serif" size="2"><br>';
	if ($_GET['failed']==1)
		echo '<b class=boldred>The username/password you tried to use is incorrect, please try again!</b>';


	if (!isset($_SESSION['userID']))
		echo '	<form action="login.php?login=1" method="POST">
			Nickname:<br>
			<input name="nickname" type="text"><br><br>
			Password:<br>
			<input name="password" type="password"><br><br>

			<input type="submit" name="submit" value="Login">
			</form>
			<a href="register_account.php?main_form=1" target="_top">Register new account</a>
			
			<br><br>
			
			';					
	else
	{
		if ($user['priv']['post_news']==1)
			echo '<a href="news.php?post_news=1">Post news item</a><br>';
		if ($user['priv']['user_management']==1)
			echo '<a href="usermanagement.php?show=1">User management</a><br>';
		if ($user['priv']['status_manage']==1)
			echo '<a href="versionadmin.php?main=1">Version management</a><br>';
		if ($user['priv']['screen_manage']==1)
			echo '<a href="screenshots.php?main=1&page=0">Add/Remove screenshots from archive</a><br>';
		
		if ($user['priv']['status_manage']==1 || $user['priv']['post_news']==1 || $user['priv']['user_management']==1)
			echo '<br>';
		
		echo '<a href="login.php?destroy=1">Click here</a> to logout this user ('.$user['nickname'].')<br><br>';
	}

		template_pagebox_end();	
		echo '</td></tr></table>';	// end of framespacing-table					
		template_end();
?>
