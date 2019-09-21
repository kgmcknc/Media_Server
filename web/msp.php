<?php
	// media server proxy to pass from web page to c code listening on another tcp port
	$ms_ip = $_SERVER['SERVER_ADDR'];
	$ms_port = "28500";
	$valid_post = 0;
	$valid_get = 0;
		
	if ($_SERVER["REQUEST_METHOD"] == "POST") {
		// collect value of input field
		$json_string = $_REQUEST['q'];
		if (empty($json_string)) {
			echo "Empty Request";
		} else {
			$json_data = json_decode($json_string);
			$valid_post = 1;
		}
	}

	if ($_SERVER["REQUEST_METHOD"] == "GET") {
		// collect value of input field
		$json_string = $_REQUEST['q'];
		if (empty($json_string)) {
			echo "Empty Request";
		} else {
			$json_data = json_decode($json_string);
			$valid_get = 1;
		}
	}

	if($valid_get | $valid_post){
		$ch = curl_init();

		if(!$ch){
			die("couldn't open curl");
		}
		
		if($valid_get == 1){
			curl_setopt($ch, CURLOPT_URL, $ms_ip.":".$ms_port."/q=".$json_string);
		}
		if($valid_post == 1){
			curl_setopt($ch, CURLOPT_URL, $ms_ip.":".$ms_port);
			curl_setopt($ch, CURLOPT_POST, 1);
			curl_setopt($ch, CURLOPT_POSTFIELDS, "/q=".$json_string);
		}

		curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);

		$output = curl_exec($ch);

		curl_close($ch);

		print_r($output);
	} else {
		echo "Empty Request";
	}
	
?>