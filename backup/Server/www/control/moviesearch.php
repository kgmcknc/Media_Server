
<?php

$q = $_REQUEST["q"];
$basedir = "/usr/share/media_server/sys_control/moviectrl/moviedir.kmf";
$dir = 0;
$dirlist;
$moviedirfile = 0;

if ($q !== "") {
	$moviedirfile = fopen($basedir, "r") or die("Unable to open moviedir File!");
	if ($moviedirfile == FALSE) {
		/*Failed to Open File*/
		print_r(0);
	} else {
		$dir = fread($moviedirfile, filesize($basedir));
		fclose($moviedirfile);
		if ($dir != "") {
			$dir = $dir . $q;
			$dirlist = scandir($dir);
			print_r($dirlist);
		} else {
			/*No Directory Set*/
			print_r(1);
		}
	}
}

?>
