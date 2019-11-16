<?php

// this src is written under the terms of the GPL-licence, see gpl.txt for futher details
	include("include/standard.inc.php");
	$page = 'page_register.gif';
	global $db;
	sstart();

	if (isset($_POST['nickname'],$_POST['password1'],$_POST['password2']) && $_GET['adding']==1)
	{
		$nickname 	= mysqli_real_escape_string( $db, htmlspecialchars($_POST['nickname']));
		$password_plain	= mysqli_real_escape_string( $db, stripslashes($_POST['password1']));
		$password1 	= mysqli_real_escape_string( $db, stripslashes(scramble($_POST['password1'])));
		$password2 	= mysqli_real_escape_string( $db, stripslashes(scramble($_POST['password2'])));
		$name 		= mysqli_real_escape_string( $db, htmlspecialchars($_POST['name']));
		$email 		= mysqli_real_escape_string( $db, htmlspecialchars($_POST['email']));
		$website 	= mysqli_real_escape_string( $db, htmlspecialchars($_POST['website']));
		$temp_passwd	= md5(rand());

		$problem = 0;	
		if ($nickname == '' || $name == '' || $password2 == '' || $password_plain == '' || $email == '')
			$problem = 1;
		
		$passwd = 0;
		if ($password1 != $password2)
			$passwd = 1;
		
		$error_mail = 0;
		if (verify_mail($email) == 0)
			$error_mail = 1;
		
		$email_exists = '';
		$occupied = '';
		if (check_mail_db($email))
			$email_exists = $email;
			
			if (check_dublicate_username($nickname) == 1)
				$occupied = $nickname;
		
		if ($problem == 1 || $occupied != '' || $passwd == 1 || $error_mail == 1 || $email_exists != '')
			Header("Location: register_account.php?main_form=1&email_exists=".$email_exists."&error_mail=".$error_mail."&passwd=".$passwd."&problem=".$problem."&occupied=".$occupied."");
		else
		{
			mysqli_query( $db, "INSERT INTO userdb (name,nickname,password,email,website, added, chg_passwd, active) VALUES ('$name', '$nickname', '$password1', '$email', '$website', NOW(), '', 1)");
			

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
	
	echo '<br>';


	$e = false;
	if ((isset($_GET['problem']) && $_GET['problem']==1) || 
	     isset($_GET['occupied']) || 
	     (isset($_GET['passwd']) && $_GET['passwd']==1) || 
	     (isset($_GET['error_mail']) && $_GET['error_mail'] == 1)) {
		$e = true;
		echo '<b>Registration failed!</b><br><ul>';
	}
	if (isset($_GET['problem']) && $_GET['problem']==1)
		echo '<li>You must fill in all the boxes that are marked with a (<span class="bold red">*</span>)</li>';	
	if (isset($_GET['error_mail']) && $_GET['error_mail'] == 1)
		echo '<li>You must type a valid email adress (you will get the activation-code by email)</li>';			
	if (isset($_GET['passwd']) && $_GET['passwd']==1)
		echo '<li>The "password" and "Retype password" are not the same, please try again!</li>';	
	if (!empty($_GET['occupied']))
		echo '<li>The nickname "<b class="bold red">'.htmlspecialchars($_GET['occupied']).'</b>" does already exist, please choose another.</li>';
	if (!empty($_GET['email_exists']))
		echo '<li>The email "<b class="bold red">'.htmlspecialchars($_GET['email_exists']).'</b>" is already in use by another member, please choose another.</li>';

	if ($e)
		echo '</ul>';
	if (isset($_GET['success']) && $_GET['success'])
		echo 'The user <b class="bold blue">'.htmlspecialchars($_GET['success']).'</b> has succesfully been created!<br><a href="login.php?mainform=1" target="_top">Click here</a> to login!<br><br>Regards, DOSBox crew!<br><br>';


	if (isset($_GET['main_form']) && $_GET['main_form']==1)
	{
		echo '
			<form action="register_account.php?adding=1" method="POST">
				<span class="bold blue">Fill in the boxes below:</span> (the box that isn\'t marked is optional)<br><br>
				<span class="bold blue">Please note: </span>Registering will <span class="bold blue">only</span> allow you to make comments or <br>add compatibility reports about games.<span class="bold blue">
			   Nothing more!</span><br><br><br>
				<table cellspacing="0" cellpadding="0">
				<tr valign="top"> 
					<td>
						<b>Nickname:</b> <span class="bold red">*</span>
					</td>
					<td width="15">
						&nbsp;
					</td>
				<td>
					<b>Website:</b>
					</td>
				</tr>
				<tr valign="top"> 
					<td>
						<input name="nickname" type="text" size="25"  maxlength="25">
					</td>
					<td width="5">
						&nbsp;
					</td>
					<td>
						<input name="website" type="text" size="45" maxlength="60">
						
					</td>
				</tr>
				<tr valign="top"> 
					<td>
						<b>Name:</b> <span class="bold red">*</span>
					</td>
					<td width="15">
						&nbsp;
					</td>
					<td>
						<b>Email:</b> <span class="bold red">*</span>
					</td>
				</tr>
				<tr valign="top"> 
					<td>
						<input name="name" type="text" size="25" maxlength="60">
					</td>
					<td width="5">
						&nbsp;
					</td>
					<td>
						<input name="email" type="text" size="45" maxlength="50">
					</td>
				</tr>
				
				<tr valign="top"> 
					<td>
						<b>Password:</b> <span class="bold red">*</span>
					</td>
					<td width="15">
						&nbsp;
					</td>
					<td>
						<b>Retype password:</b> <span class="bold red">*</span>
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
