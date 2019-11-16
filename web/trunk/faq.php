<?php
// this src is written under the terms of the GPL-licence, see gpl.txt for futher details
	include("include/standard.inc.php");
	sstart();
	template_header();

	echo '<br><table width="100%"><tr><td width="14">&nbsp;</td><td>';// start of framespacing-table

	template_pagebox_start("DOSBox FAQ (Frequently Asked Questions)");
?>	
<br>

				
					<b>What is DOSBox?</b>
					<ul>
						It is a PC emulator with builtin DOS for running DOS Games primarily. For more information, visit the <a href="information.php">Information</a> page.
					</ul>
					
					<b>Game X doesn't Run?</b>
					<ul>
						Shit happens, wait for another version. (<a href="http://vogons.zetafleet.com/viewtopic.php?t=4020">or look here</a>)
					</ul>

					<b>DOSBox crashes out with some EXIT:CPU:Opcode blah Unhandled message.</b><br>
					<ul>
						The CPU emulation encountered an instruction it doesn't emulate yet or and illegal instruction. <br>This can mean the game uses some advanced CPU functions not emulated yet or something goes wrong and non-code gets executed.<br>
						All in all nothing you can do anything about but wait for a newer version.
					</ul>

					<b>The game doesn't detect the soundbaster.</b>
					<ul>
						Does the game have some setup/soundset/install or similar tool to setup your sound? Use that. <br>
						The default settings for the soundblaster are, port 220, irq 7, dma 1.<br>
						Some games may also require you to slow down cpu emulation to detect the sound blaster correctly.
					</ul>

					<b>Will DOSBox be open source?</b>
					<ul>
						It is, check our <a href="https://www.sourceforge.net/projects/dosbox">sourceforge site</a>.
					</ul>
					
					<b>Does Duke Nukem 3D Run?</b>
					<ul>
						Yes, but remember that you are emulating a <b>full</b> PC when selecting graphics and other options.<br>
						It might run not as fast as you had hoped.
					</ul>
					
					<b>Where might i ask questions regarding DOSBox ?</b>
					<ul>
						You may ask your questions in our IRC channel, connect to the <b>irc.freenode.net</b> IRC server and join the channel <b>#DOSBox</b>, <b>but please</b> check our <b>Forum first</b> your question may already be answered there.
					</ul>					
						
			</font>

<?php
	template_pagebox_end();

	
	echo '</td></tr></table>';	// end of framespacing-table
	
	
	template_end();
?>
