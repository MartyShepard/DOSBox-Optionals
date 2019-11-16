<?php
// this src is written under the terms of the GPL-licence, see gpl.txt for futher details
	include("include/standard.inc.php");
	$page = 'page_register.gif';
	sstart();

	if ($_GET['adding']==1)
	{
		$nickname 	= mysql_escape_string(stripslashes($_POST['nickname']));
		$password_plain	= mysql_escape_string(stripslashes($_POST['password1']));
		$password1 	= mysql_escape_string(stripslashes(md5($_POST['password1'])));
		$password2 	= mysql_escape_string(stripslashes(md5($_POST['password2'])));
		$name 		= mysql_escape_string(stripslashes($_POST['name']));
		$email 		= mysql_escape_string(stripslashes($_POST['email']));
		$website 	= mysql_escape_string(stripslashes($_POST['website']));
		$temp_passwd	= md5(rand(0, 1999999999999999));
	
		if ($nickname == '' || $name == '' || $password2 == '' || $password1 == '' || $email == '')
			$problem = 1;
		
		if ($password1 != $password2)
			$passwd = 1;
		
		if (verify_mail($email) == 0)
			$error_mail = 1;
		
		if (check_mail_db($email))
			$email_exists = $email;
			
			if (check_dublicate_username($nickname) == 1)
				$occupied = $nickname;
		
		if ($problem == 1 || $occupied != '' || $passwd == 1 || $error_mail == 1 || $email_exists != '')
			Header("Location: register_account.php?main_form=1&email_exists=".$email_exists."&error_mail=".$error_mail."&passwd=".$passwd."&problem=".$problem."&occupied=".$occupied."");
		else
		{
			mysql_query("INSERT INTO userdb (name,nickname,password,email,website, added, chg_passwd, active) VALUES ('$name', '$nickname', '$password1', '$email', '$website', NOW(), '', 1)");
			

			/* 	
				$message = '
				
				
				Hi and welcome to BeGaming, just click on this link to activate your account:
				http://skrubben.mine.nu/BeGaming/activate.php?activate=1&username='.$nickname.'&temp_password='.$temp_passwd.'"
				
				Your nickname is: '.$nickname.'
				Your password is: '.$password_plain.'
				Regards BeGaming crew!
			
				';			
				$from = 'admin@skrubben.mine.nu';
				$subject = 'BeGaming activation-mail!';
				$mail = new mail_class;
  				$mail->new_mail($from, $email, $subject);
				$mail->add_part($message, "text/plain", "iso-8859-1", "7bit");
				$mail->sendmail();	
				send_activate_email($temp_passwd, $nickname, 1, $email, $_POST['password1']); */
			
			Header("Location: register_account.php?success=".$nickname."");
		}
	}
	
	template_header();
	echo '<br><table width="100%"><tr><td width="14">&nbsp;</td><td>';// start of framespacing-table		
	
	template_pagebox_start("Register account", 550);			
	
	echo '<font face="Verdana, Arial, Helvetica, sans-serif" size="2"><br>';


	
	if ($_GET['problem']==1 || $_GET['occupied'] || $_GET['passwd']==1 || $_GET['error_mail'] == 1)
		echo '<b>Registration failed!</b><br>';
	
	if ($_GET['problem']==1)
		echo '<li>You must fill in all the boxes that is marked with a <b class="boldred">(*)</b></li>';	
	if ($_GET['error_mail'] == 1)
		echo '<li>You must type a valid email adress (you will get the activation-code by email)</li>';			
	if ($_GET['passwd']==1)
		echo '<li>The "password" and "Retype password" is not desame, please try again!</li>';	
	if ($_GET['occupied'])
		echo '<li>The nickname "<b class="boldred">'.$_GET['occupied'].'</b>" does already exist, please choose another.</li>';
	if ($_GET['email_exists'])
		echo '<li>The email "<b class="boldred">'.$_GET['email_exists'].'</b>" is already in use by another member, please choose another.</li>';
	if ($_GET['success'])
		echo 'The user <b class="boldblue">'.$_GET['success'].'</b> has succesfully been created!<br><a href="login.php?mainform=1" target="_top">Click here</a> to login!<br><br>Regards, DOSBox crew!<br><br>';


	if ($_GET['main_form']==1)
	{
		echo '
			<form action="register_account.php?adding=1" method="POST">
				<b class=boldblue>Fill in the boxes below:</b> (the boxes that isn\'t marked are optional)<br><br>
				<table cellspacing="0" cellpadding="0">
				<tr valign="top"> 
					<td>
						<font face="Verdana, Arial, Helvetica, sans-serif" size="2">Nickname:</b> <b class="boldred">*</b>
					</td>
					<td width="15">
						&nbsp;
					</td>
					<td>
						<font face="Verdana, Arial, Helvetica, sans-serif" size="2">Website:</b>
					</td>
				</tr>
				<tr valign="top"> 
					<td>
						<input name="nickname" type="text" size="25"  maxlength="25" value="'.$result[2].'">
					</td>
					<td width="5">
						&nbsp;
					</td>
					<td>
						<input name="website" type="text" size="45" maxlength="60" value="'.$result[4].'">
						
					</td>
				</tr>
				<tr valign="top"> 
					<td>
						<font face="Verdana, Arial, Helvetica, sans-serif" size="2">Name:</b> <b class="boldred">*</b>
					</td>
					<td width="15">
						&nbsp;
					</td>
					<td>
						<font face="Verdana, Arial, Helvetica, sans-serif" size="2">Email:</b> <b class="boldred">*</b>
					</td>
				</tr>
				<tr valign="top"> 
					<td>
						<input name="name" type="text" size="25" maxlength="60" value="'.$result[1].'">
					</td>
					<td width="5">
						&nbsp;
					</td>
					<td>
						<input name="email" type="text" size="45" maxlength="50" value="'.$result[3].'">
					</td>
				</tr>
				
				<tr valign="top"> 
					<td>
						<font face="Verdana, Arial, Helvetica, sans-serif" size="2">Password:</b> <b class="boldred">*</b>
					</td>
					<td width="15">
						&nbsp;
					</td>
					<td>
						<font face="Verdana, Arial, Helvetica, sans-serif" size="2">Retype password:</b> <b class="boldred">*</b>
					</td>
				</tr>
				<tr valign="top"> 
					<td>
						<input name="password1" type="password" size="25" maxlength="40">
					</td>
					<td width="5">
						&nbsp;
					</td>
					<td>
						<input name="password2" type="password" size="25" maxlength="40">
					</td>
				</tr>				
				</table>			

			<br><br>
			
			<input type="submit" name="submit" value="Register me">
			</form>';
	}

		template_pagebox_end();	
		echo '</td></tr></table>';	// end of framespacing-table					
		template_end();
?>
