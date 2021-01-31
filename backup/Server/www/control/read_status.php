
<?php

$q = $_REQUEST["q"];
$status_file = "/usr/share/media_server/sys_control/sys_status.kmf";
$status = 0;
$status_data = 0;

if ($q !== "") {
	$status = fopen($status_file, "r") or die("Unable to open status File!");
	if ($status == FALSE) {
		/*Failed to Open File*/
		print_r(0);
	} else {
		$status_data = fread($status, filesize($status_file));
		fclose($status);
        print_r($status_data);
	}
}

?>
