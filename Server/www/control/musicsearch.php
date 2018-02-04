
<?php

$q = $_REQUEST["q"];
$basedir = "/usr/share/media_server/sys_control/musicctrl/musicdir.kmf";
$dir = 0;
$dirlist;
$musicdirfile = 0;

if ($q !== "") {
	$musicdirfile = fopen($basedir, "r") or die("Unable to open musicdir File!");
	if ($musicdirfile == FALSE) {
		/*Failed to Open File*/
		print_r(0);
	} else {
		$dir = fread($musicdirfile, filesize($basedir));
		fclose($musicdirfile);
		if ($dir != "") {
			$dir = $dir . $q;
			$dirlist = array_slice(scandir($dir), 2);
			print_r($dirlist);
		} else {
			/*No Directory Set*/
			print_r(1);
		}
	}
}

?>
