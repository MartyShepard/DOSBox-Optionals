<?php
// this src is written under the terms of the GPL-licence, see gpl.txt for futher details
	include("include/standard.inc.php");
	sstart();
	template_header();
	global $db;
	echo '<br><table width="100%"><tr><td width="14">&nbsp;</td><td valign="top">';// start of framespacing-table

	template_pagebox_start("The people behind DOSBox");

echo '
	<table cellspacing="0" cellpadding="0" width="100%">
	<tr>
		<td valign="top" width="180"><b>Name:</b>
		</td>
		<td valign="top" width="150"><b>Nickname:</B>
		</td>
		<td valign="top" width="250"><b>Website:</B>
		</td>

	</tr>';
	$query = mysqli_query($db,"SELECT name, nickname, email, website FROM userdb WHERE userdb.part_of_crew =1 ORDER BY userdb.name");
	
	while ($result=mysqli_fetch_row($query))
	{
		echo '
		<tr>
			<td valign="top">'.$result[0].'
			</td>
			<td valign="top">'.$result[1].'
			</td>
			<td valign="top">'; if ($result[3] != '') echo '<a href="'.$result[3].'" target="_blank">'.$result[3].'</a>'; else echo '&nbsp;';
			echo '</td>

		</tr>';				
	}	
	echo '</table>';

	template_pagebox_end();

	
echo '</td></tr><tr><td>&nbsp;</td><td>
      If you want to communicate with us feel free to use the following: 
<IMG SRC="site_images/nothing.png" width="200" height="20"alt="Nothing"><br>
<em>If you have questions on how to use DOSBox or how to run a specific game; please use the <a href="https://www.vogons.org/viewforum.php?f=53">forum</a> instead</em><br>
';
echo '</td></tr></table>';	// end of framespacing-table


	template_end();
?>
