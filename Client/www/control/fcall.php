
<?php

$q = $_REQUEST["q"];
$function;
$program = "/var/www/html/control/websysproc/websys \"";
$quotes = "\"";

if ($q !== "") {
	$function = $program . $q . $quotes;
	system($function, $retval);
	//echo $retval;
}

?>
