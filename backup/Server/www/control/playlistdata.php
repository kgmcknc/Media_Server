
<?php

$q = $_REQUEST["q"];
$basedir = "/usr/share/media_server/sys_control/playlistdir.kmf";
$dir = 0;
$fp;
$fdata;
$playlistdirfile = 0;

$playlistdirfile = fopen($basedir, "r") or die("Unable to open playlist dir File!");
if ($playlistdirfile == FALSE) {
	/*Failed to Open File*/
	print_r(0);
} else {
	$dir = fread($playlistdirfile, filesize($basedir));
	fclose($playlistdirfile);
	if ($dir != "") {
		$dir = $dir . $q;
		$fp = fopen($dir, "r");
		if($fp) {
			while(($fdata = fgets($fp, 4096)) !== false){
				print_r($fdata);
			}
			if(!feof($fp)){
				/*Couldn't Open File*/
				print_r(3);
			}
			fclose($fp);
		} else {
			/*Couldn't Open File*/
			print_r(2);
		}
		print_r($dirlist);
	} else {
		/*No Directory Set*/
		print_r(1);
	}
}

?>
