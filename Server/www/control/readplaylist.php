
<?php

$q = $_REQUEST["q"];
$basedir = "/usr/share/media_server/sys_control/playlistdir.kmf";
$dir = 0;
$playlistdata;
$pre = "/./";
$playlistdirfile = 0;
$playlistfile = 0;

$playlistdirfile = fopen($basedir, "r") or die("Unable to open playlist dir File!");
if ($playlistdirfile == FALSE) {
	/*Failed to Open File*/
	print_r("dirfail");
} else {
	$dir = fread($playlistdirfile, filesize($basedir));
	fclose($playlistdirfile);
	if ($dir != "") {
		$dir = $dir . $pre . $q;
		$playlistfile = fopen($dir, "r") or die("Unable to open playlist dir File!");
		if ($playlistdirfile == FALSE) {
			/*Failed to Open File*/
			print_r(0);
		} else {
			$playlistdata = fread($playlistfile, filesize($dir));
			fclose($playlistfile);
			print_r($playlistdata);
		}
	} else {
		/*No Directory Set*/
		print_r(1);
	}
}

?>
