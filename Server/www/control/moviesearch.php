
<?php

$q = $_REQUEST["q"];
$basedir = "/home/kyle/linux-main-share/MovieHD";
$dir = 0;
$dirlist;

if ($q !== "") {
	$dir = $basedir . $q;
	$dirlist = array_slice(scandir($dir), 2);
	print_r($dirlist);
}

?>
