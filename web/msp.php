<?php
	// media server proxy to pass from web page to c code listening on another tcp port
	$ms_ip = array_key_exists('SERVER_ADDR',$_SERVER) ? $_SERVER['SERVER_ADDR'] : $_SERVER['LOCAL_ADDR'];
   if($ms_ip == "::1"){
      // change from "valid" localhost (ipv6 I think) to 127.0.0.1
      $ms_ip = "localhost";
   }
	$ms_port = "50000";
	
	$valid_post = 0;
	$valid_get = 0;

   //write_log(print_r($_SERVER,1));
	
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
		
		if(curl_errno($ch)){
		   //echo $ms_ip;
         echo 'Curl error: ' . curl_error($ch);
		} else {
		   print_r($output);
		}

		curl_close($ch);
	} else {
		echo "Empty Request";
	}
	
   function write_log($log_msg)
   {
      $log_folder = "C:/Windows/Temp/php_output";
      $log_filename = "php_output.txt";
      if (!file_exists($log_folder))
      {
         mkdir($log_folder, 0777, true);
      }
      $log_file_data = $log_folder."/".$log_filename;
      file_put_contents($log_file_data, $log_msg . "\n", FILE_APPEND);  
   }
?>