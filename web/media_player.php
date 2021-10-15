<?php

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

   function shutdown()
   {
      $a = error_get_last();

      if ($a == null) {echo "No errors";}
      else {print_r($a);}
   }

   $my_media_basename = $_REQUEST["media_file"];//filter to have a trust filename

   $file = "D:/" . $my_media_basename;
    
   if(!file_exists($file)){
     print_r("File Doesn't Exist");
     return;
   }

   register_shutdown_function('shutdown');
   //write_log(print_r($_SERVER,1));

   $fp     = fopen($file, 'rb');
   if($fp == FALSE){
      print_r("File Open Error");
      return;
   }
   $size   = filesize($file); // File size
   $length = $size;           // Content length
   $start  = 0;               // Start byte
   $end    = $size - 1;       // End byte
   header('Content-type: video/mp4');
   header("Accept-Ranges: 0-$length");
   header("Accept-Ranges: bytes");
   if (isset($_SERVER['HTTP_RANGE'])) {
      $c_start = $start;
      $c_end   = $end;
      list(, $range) = explode('=', $_SERVER['HTTP_RANGE'], 2);
      if (strpos($range, ',') !== false) {
         header('HTTP/1.1 416 Requested Range Not Satisfiable');
         header("Content-Range: bytes $start-$end/$size");
         fclose($fp);
         ob_clean();
         exit();
      }
      if ($range == '-') {
         $c_start = $size - substr($range, 1);
      }else{
         $range  = explode('-', $range);
         $c_start = $range[0];
         $c_end   = (isset($range[1]) && is_numeric($range[1])) ? $range[1] : $size;
      }
      $c_end = ($c_end > $end) ? $end : $c_end;
      if ($c_start > $c_end || $c_start > $size - 1 || $c_end >= $size) {
         header('HTTP/1.1 416 Requested Range Not Satisfiable');
         header("Content-Range: bytes $start-$end/$size");
         fclose($fp);
         ob_clean();
         exit();
      }
      $start  = $c_start;
      $end    = $c_end;
      $length = $end - $start + 1;
      fseek($fp, $start);
      header('HTTP/1.1 206 Partial Content');
   }
   header("Content-Range: bytes $start-$end/$size");
   header("Content-Length: ".$length);
   $buffer = 256 * 8;
   while(!feof($fp) && ($p = ftell($fp)) <= $end) {
      if ($p + $buffer > $end) {
         $buffer = $end - $p + 1;
      }
      set_time_limit(0);
      echo fread($fp, $buffer);
      ob_flush();
   }
   fclose($fp);
   ob_clean();
   exit();
?>