<?php
// this src is written under the terms of the GPL-licence, see gpl.txt for futher details
	include("include/standard.inc.php");
	sstart();
	template_header();
	echo '<br><table width="100%"><tr><td width="14">&nbsp;</td><td>';// start of framespacing-table

template_pagebox_start("DOSBox related links", 740);
			echo '<ul>
			<li>
			<a href="https://www.sourceforge.net/projects/dosbox" target="_blank">Sourceforge</a> - DOSBox project page,  submit bugs and patches here. 
			</li>
			<li>
			<a href="https://www.vogons.org" target="_blank">VOGONS</a> - Official DOSBox Forum, look here for solutions for your problems.
			</li>
			<li>
			<a href="http://pain.scene.org/service_dosbox.php" target="_blank">pain.scene.org</a> - Unofficial DOSBox Demo Compatibility List.
			</li>							    
			<li>
			<a href="https://www.pixelships.com/adg/index.html" target="_blank">Ancient DOS Games</a> - Great game reviews made by Kris Asick using DOSBox.
			</li>
			</ul>';
template_pagebox_end();
	template_pagebox_start("Where to get DOS games",740);
			echo '<ul>
			<li>
			<a href="https://www.classicdosgames.com">Classic DOS Games</a> - has free downloads of over 300 games.
			</li>
			<li>
			<a href="https://www.gog.com/?pp=b3f0c7f6bb763af1be91d9e74eabfeb199dc1f1f">GOG.com</a> - DRM-free PC classics.
			</li>
			</ul>';


template_pagebox_end();

template_pagebox_start("General Emulation links", 740);
			echo '<ul>
			<li>
			<a href="http://www.emu-france.com/" target="_blank">Emu-France</a> - a good french source for your daily emulation news.
			</li>
			<li>
			<a href="https://www.aep-emu.de/" target="_blank">AEP Emulation Page</a> - German emulation news. Online since 1 april 1998.
			</li>
			</ul>';
template_pagebox_end();

template_pagebox_start("PC/DOS/SB Emulation links", 740);			
			echo '<ul>
			<li>
			<a href="https://sourceforge.net/projects/vdmsound/" target="_blank">VDMSound</a> - Great tool by Vlad R. that gives you Soundblaster emulation under Windows NT/2K/XP.
			</li>
		        <li>
			<a href="https://bochs.sourceforge.net/" target="_blank">Bochs IA-32 Emulator</a> - Bochs emulates a full x86 based pc. Unlike DOSBox that tries to mainly emulate dos programs.
			</li>
                        <li>
			<a href="http://www.dosemu.org/" target="_blank">DOSEMU</a> - DOS Emulator for Linux, great tool to run just about any dos application under linux.
			</li>
			</ul>';
template_pagebox_end();
	
	echo '</td></tr></table>';	// end of framespacing-table
	
	template_end();
?>
