<?php
// this src is written under the terms of the GPL-licence, see gpl.txt for futher details
	include("include/standard.inc.php");
	sstart();
	template_header();

	echo '<br><table width="100%"><tr><td width="14">&nbsp;</td><td>';// start of framespacing-table

	template_pagebox_start("The people behind DOSBox");

echo '
	<table cellspacing="0" cellpadding="0" width="100%">
	<tr>
		<td valign="top" width="180"><font face="Verdana, Arial, Helvetica, sans-serif" size="2"><b>Name:</b></font>
		</td>
		<td valign="top" width="150"><font face="Verdana, Arial, Helvetica, sans-serif" size="2"><b>Nickname:</B></font>
		</td>
		<td valign="top" width="250"><font face="Verdana, Arial, Helvetica, sans-serif" size="2"><b>Website:</B></font>
		</td>

	</tr>';
	$query = mysql_query("SELECT name, nickname, email, website FROM userdb WHERE userdb.part_of_crew =1 ORDER BY userdb.name");
	
	while ($result=mysql_fetch_row($query))
	{
		echo '
		<tr>
			<td valign="top"><font face="Verdana, Arial, Helvetica, sans-serif" size="2"><a href="mailto:'.$result[2].'">'.$result[0].'</a></font>
			</td>
			<td valign="top"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">'.$result[1].'</font>
			</td>
			<td valign="top"><font face="Verdana, Arial, Helvetica, sans-serif" size="2">'; if ($result[3] != '') echo '<a href="'.$result[3].'" target="_blank">'.$result[3].'</a>'; else echo '&nbsp;';
			echo '</font></td>

		</tr>';				
	}	
	echo '</table>';

	template_pagebox_end();

	
	echo '</td></tr></table>';	// end of framespacing-table


	template_end();
?>
