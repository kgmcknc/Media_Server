
<?php

$q = $_REQUEST["q"];
$function;
$program = "/var/www/html/media_server/control/web_control/web_control \"";
$quotes = "\"";

if ($q !== "") {
	$function = $program . $q . $quotes;
	system($function, $retval);
	//echo $retval;
}

?>
