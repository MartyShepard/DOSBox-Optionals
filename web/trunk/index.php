<?php 
// this src is written under the terms of the GPL-licence, see gpl.txt for futher details
        include("include/standard.inc.php");
	        sstart();
        template_header();
	        echo '<br>
		<table width="100%">
		        <tr>
			                <td width="14">&nbsp;</td>
					                <td>';// start of framespacing-table

echo '
<form action="https://www.paypal.com/cgi-bin/webscr" method="post">
<input type="hidden" name="cmd" value="_s-xclick">
<span class="c1"><strong>Note:</strong> this website uses cookies for the user account-system!</span>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
<input type="hidden" name="hosted_button_id" value="RK4BA9ERN5AWL">
<input type="image" src="https://www.paypal.com/en_US/i/btn/x-click-but21.gif" border="0" name="submit" alt="PayPal - The safer, easier way to pay online!">
<img alt="" border="0" src="https://www.paypal.com/en_US/i/scr/pixel.gif" width="1" height="1">
</form><br>
';

													        if (isset($user) && ($user['priv']['post_news']==1))
															                echo '<a href="news.php?post_news=1">Create news-item</a><br><br>';
													        main_news(isset($user)?$user['priv']['news_management']:0);
		        echo '</td></tr></table>';      // end of framespacing-table
template_end();
?>
