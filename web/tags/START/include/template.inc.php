<?php
// this src is written under the terms of the GPL-licence, see gpl.txt for futher details
function template_header()
{
	echo '
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<HTML>
<HEAD><TITLE>DOSBox, a x86 emulator with DOS</TITLE>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1"> 
</HEAD>	
		<body bgcolor="#000000" text="#FFFFFF" link="#FFFFFF" vlink="#FFFFFF" alink="#FFFFFF" leftmargin="0" topmargin="0" marginwidth="0" marginheight="0" background="site_images/background.gif">
			
			<table width="100%" border="0" cellspacing="0" cellpadding="0">
				<tr align="left" valign="top"> 
					<td>
						<img src="site_images/upper_logo.png" width="499" height="102" alt="logo upper">
					</td>
				
					<td width="100%">
						<img src="site_images/upper_back.png" width="100%" height="102" alt="back upper">
					</td>
				</tr>
			</table>

<table border="0" cellspacing="0" cellpadding="0">
	<tr align="left" valign="top"> 
		<td width="520"><img src="site_images/menu.gif" usemap="#dosbox" border="0" alt="menu"></td>
		<td valign="middle" align="right" width="158">
			<font face="Verdana, Arial, Helvetica, sans-serif" size="2">Latest version: <b>'; echo get_latest_version(); echo '</b></font>
		</td>
	</tr>
</table><br>
			
			
	';
}
function template_end()
{
	echo '	
		<table align="left" width="100%" border="0" cellspacing="0" cellpadding="0">
			<tr>
				<td valign="top" width="14">
 					&nbsp;
				</td>
				<td valign="top">';
					/*
					&nbsp;&nbsp;&nbsp;
					<a target="_blank" href="http://t.extreme-dm.com/?login=harekiet">
					<img name=im src="http://t1.extreme-dm.com/i.gif" height=38
					border=0 width=41 alt=""></a><script language="javascript"><!--
					an=navigator.appName;d=document;function
					pr(){d.write("<img src=\"http://t0.extreme-dm.com",
					"/0.gif?tag=harekiet&j=y&srw="+srw+"&srb="+srb+"&",
					"rs="+r+"&l="+escape(parent.document.referrer)+"\" height=1 ",
					"width=1>");}srb="na";srw="na";//-->
					</script><script language="javascript1.2"><!--
					s=screen;srw=s.width;an!="Netscape"?
					srb=s.colorDepth:srb=s.pixelDepth;//-->
					</script><script language="javascript"><!--
					r=41;d.images?r=d.im.width:z=0;pr();//-->
					</script><noscript><img height=1 width=1 alt="" 
					src="http://t0.extreme-dm.com/0.gif?tag=harekiet&amp;j=n"></noscript>	
					&nbsp;&nbsp;&nbsp;&nbsp;<a href="http://www.sourceforge.net" target="_blank"><img src="http://sourceforge.net/sflogo.php?group_id=52551&amp;type=1" width="88" height="31" border="0" alt="SourceForge.net Logo"></a><br>
					<img src="site_images/copyright.gif" alt="Copyright 2003 DOSBox">
					*/
				echo '</td>
			</tr>
		</table><br><br><br><br><br><br>
			<map name="dosbox">
			  <area shape="rect" coords="14,4,54,32" href="news.php?show_news=1"   alt="The latest news regarding DOSBox">
			  <area shape="rect" coords="59,5,96,37" href="crew.php"   alt="The hard working guys behind DOSBox">
			  <area shape="rect" coords="102,5,185,54" href="information.php?page=0" alt="Information about DOSBox">
			  <area shape="rect" coords="192,5,237,51" href="status.php?show_status=1" alt="The current status of DOSBox">
			  <area shape="rect" coords="242,4,273,27" href="faq.php" alt="Frequently Asked Questions">
			  <area shape="rect" coords="282,4,359,47" href="download.php?main=1" alt="You may download DOSBox here">
			  <area shape="rect" coords="370,4,416,41" href="comp_list.php?letter=a" alt="DOSBox compatibility-list">
			  <area shape="rect" coords="427,4,473,35" href="http://vogons.zetafleet.com/forumdisplay.php?s=&amp;forumid=30" alt="DOSBox forum" target="_blank">
			  <area shape="rect" coords="482,4,523,45" href="links.php" alt="DOSBox recomended links">
			  <area shape="rect" coords="530,3,576,25" href="login.php" alt="Login to your account">
			</map>
		</body>
		</html>



	';
}
function template_pagebox_start($text, $width=730)
{
	echo '<table width="'.$width.'" cellspacing="0" cellpadding="1" bgcolor="#000000"><tr><td valign="top" align="left"><table cellspacing="4" cellpadding="0" width="100%" bgcolor="#355787"><tr><td>
	<table cellspacing="0" cellpadding="0" width="100%"><tr><td valign="top" align="left"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">'.$text.'</font></td></tr></table></td>
	</tr></table></td></tr></table><table width="'.$width.'" cellspacing="0" cellpadding="1" bgcolor="#000000"><tr><td valign="top" align="left"><table cellspacing="4" cellpadding="0" width="100%" bgcolor="#113466"><tr>
	<td><table cellspacing="0" cellpadding="0" width="100%"><tr><td valign="top" align="left">';	
}
function template_pagebox_end()
{
	echo '</td></tr></table></td></tr></table></td></tr></table><br>';	
}
?>
