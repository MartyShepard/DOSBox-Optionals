<?php
// this src is written under the terms of the GPL-licence, see gpl.txt for futher details
	include("include/standard.inc.php");
	sstart();
	template_header();

echo '
<br><table width="100%" border="0" cellspacing="0" cellpadding="0">
	<tr align="left" valign="top"> 
		<td width="14">
			&nbsp;
		</td>
	
		<td width="540">';
	
	template_pagebox_start("Information", 540);		
	
			echo '<br>
			DOSBox is a DOS-emulator that uses the SDL-library which makes DOSBox very easy to port to different platforms. 
		        DOSBox has already been ported to many different platforms, such as Windows, BeOS, Linux, MacOS X...<br><br>
			DOSBox also emulates CPU:286/386 realmode/protected mode, Directory FileSystem/XMS/EMS, Tandy/Hercules/CGA/EGA/VGA/VESA graphics, 
			a SoundBlaster/Gravis Ultra Sound card for excellent sound compatibility with older games...<br><br>
			You can "re-live" the good old days with the help of DOSBox, it can run plenty of the old classics that don\'t run on your new computer!<br><br>
			
			DOSBox is totally free of charge and Open Source.<br><br>
			Check our <a href="download.php?main=1" target="_top">Downloads</a> page for the most recent DOSBox version.<br><br>';
        template_pagebox_end();
        echo '<br>';
	
		echo '</td>
		<td width="30">
			&nbsp;
		</td>
		<td>';
			show_screenshots(3);
			
		
		echo '</td>
	</tr>
</table>';

template_end();
?>
