<?php
// this src is written under the terms of the GPL-licence, see gpl.txt for futher details
	include("include/standard.inc.php");
	sstart();
	
	if (isset($_GET['destroy']) && $_GET['destroy']==1)
	{
		SESSION_DESTROY();
		session_unset();
		Header("Location: login.php");
	}
		
	if (isset($_GET['login'],$_POST['nickname'],$_POST['password']) && $_GET['login']==1)
	{
		login($_POST['nickname'], $_POST['password']);
	}	

	template_header();
	echo '<br><table width="100%"><tr><td width="14">&nbsp;</td><td>';// start of framespacing-table		
	
	template_pagebox_start("Login", 550);			
	
	echo '<br>';
	if (isset($_GET['failed']) && $_GET['failed']==1)
		echo '<span class="bold red">The username/password you tried to use is incorrect, please try again!</span>';


	if (!isset($_SESSION['userID']))
{

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

}	else
	{ if( isset($user) ) {
		if ($user['priv']['post_news']==1)
			echo '<a href="news.php?post_news=1">Post news item</a><br>';
		if ($user['priv']['user_management']==1)
			echo '<a href="usermanagement.php?show=1">User management</a><br>';
		if ($user['priv']['status_manage']==1)
			echo '<a href="versionadmin.php?main=1">Version management</a><br>';
//		if ($user['priv']['screen_manage']==1)
//			echo '<a href="screenshots.php?main=1&page=0">Add/Remove screenshots from archive</a><br>';
		
		if ($user['priv']['status_manage']==1 || $user['priv']['post_news']==1 || $user['priv']['user_management']==1)
			echo '<br>';
		
		echo '<a href="login.php?destroy=1">Click here</a> to logout this user ('.$user['nickname'].')<br><br>';
		}
	}
	echo '</font>';

		template_pagebox_end();
/*
template_pagebox_start("Info for returning users", 550);
	echo '<font face="Verdana, Arial, Helvetica, sans-serif">';
echo '
<h3>Hackers have gained access to the user accounts on DOSBox.com.</h3>
You should consider <b>your password</b> that you used on <a href="http://www.dosbox.com">DOSBox.com</a> as <b>compromised</b>, so if you used this password at other places you should update them with a new password. Once I fix the problem I will email all active users on DOSBox.com a new password for DOSBox.com. (accounts with no posts/games on DOSBox.com are purged on a regular basis and are thus unaffected by this hack)
<br><br>
The passwords for our <a href="http://vogons.zetafleet.com/index.php?c=7">forums</a> are <b>not</b> affected, however if you used the same password for both, then you should update your forum password.
</font>';
template_pagebox_end();	
*/
		echo '</td></tr></table>';	// end of framespacing-table					
		template_end();
?>
