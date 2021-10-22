<?php

   //while(ob_end_clean());
   //ob_implicit_flush(true);
   ignore_user_abort(true);
   ob_start();
   $enable_output_log = 0;

   $my_media_basename = $_REQUEST["media_file"];//filter to have a trust filename

   $file = $my_media_basename;
   
   if(!file_exists($file)){
     print_r("File Doesn't Exist");
     shutdown();
   }

   register_shutdown_function('shutdown');
   set_exception_handler("my_error_handler");
   //write_log(print_r($_SERVER,1));

   write_log("");
   write_log("starting player php");
   $fp     = fopen($file, 'rb');
   if($fp == FALSE){
      print_r("File Open Error");
      shutdown();
   }
   $size   = filesize($file); // File size
   $length = $size;           // Content length
   $start  = 0;               // Start byte
   $end    = $size - 1;       // End byte
   
   if (isset($_SERVER['HTTP_RANGE'])) {
      $c_start = $start;
      $c_end   = $end;
      list(, $range) = explode('=', $_SERVER['HTTP_RANGE'], 2);
      if (strpos($range, ',') !== false) {
         header('HTTP/1.1 416 Requested Range Not Satisfiable');
         header("Content-Range: bytes $start-$end/$size");
         write_log("invalid range");
         shutdown();
      }
      if ($range == '-') {
         $c_start = $size - substr($range, 1);
         write_log("range was -");
      }else{
         $range  = explode('-', $range);
         $c_start = $range[0];
         $c_end   = (isset($range[1]) && is_numeric($range[1])) ? $range[1] : $end;
      }
      if($c_end > $end){
         $c_end = $end;
      }
      if ($c_start > $c_end || $c_start > $size - 1 || $c_end >= $size) {
         header('HTTP/1.1 416 Requested Range Not Satisfiable');
         header("Content-Range: bytes $start-$end/$size");
         write_log("invalid range 2");
         shutdown();
      }
      $start  = $c_start;
      $end    = $c_end;
      $length = $end - $start + 1;
      fseek($fp, $start);
      header('HTTP/1.1 206 Partial Content');
   }else {
      header('HTTP/1.1 200 OK');
   //   write_log("Probably should exit?!");
      shutdown();
      exit;
   }
   header('Content-type: video/mp4');
   header("Last-Modified: ".date("F d Y H:i:s.", filemtime($file)));
   
   header("Accept-Ranges: bytes");
   header("Server: ".$_SERVER["SERVER_SOFTWARE"]);
   header("Date: ".gmdate('D, d M Y H:i:s T'));
   
   header("Content-Length: ".$length);
   header("Content-Range: bytes $start-$end/$size");
   
   // set how big of chunks to send back to server
   write_log("starting to send file");
   $buffer = 1024*8;
   //set_time_limit(0);
   try{
      while(1) {
         if(feof($fp) || (($p = ftell($fp)) >= $end)){
            write_log("file done");
            break;
         }
         if(connection_aborted() || (connection_status() != CONNECTION_NORMAL)){
            write_log("aborted");
            break;
         }
         if ($p + $buffer > $end) {
            $buffer = $end - $p + 1;
         }
         echo fread($fp, $buffer);
         $buffer_fullness = ob_get_length();
         $num_buffers = ob_get_level();
         ob_flush();
         flush();
      }
      ob_flush();
      flush();
      write_log("finished file");
      fclose($fp);
   } catch(Exception $e) {
      ob_flush();
      flush();
      write_log("Caught Exception");
      fclose($fp);
   }
   // //ob_end_clean();
   // write_log("finished player php");

   function write_log($log_msg)
   {
      if($enable_output_log > 0){
         $log_folder = "C:/Windows/Temp/php_output";
         $log_filename = "php_output.txt";
         if (!file_exists($log_folder))
         {
            mkdir($log_folder, 0777, true);
         }
         $log_file_data = $log_folder."/".$log_filename;
         file_put_contents($log_file_data, $log_msg . "\n", FILE_APPEND);
      }
   }

   function shutdown()
   {
      //$a = error_get_last();
      //fastcgi_finish_request();

      //if ($a == null) {echo "No errors";}
      //else {print_r($a);}
      //ob_flush();
      //flush();
      //ob_end_clean();
      if(connection_aborted() || (connection_status() != CONNECTION_NORMAL)){
         write_log("aborted");
      } else {
         ob_flush();
         flush();
         while(ob_get_level() > 0){
            ob_end_flush();
         }
      }
      write_log("Shutdown Function");
      exit;
   }

   function my_error_handler()
   {
      //$a = error_get_last();
      //fastcgi_finish_request();

      //if ($a == null) {echo "No errors";}
      //else {print_r($a);}
      //ob_flush();
      //flush();
      //ob_end_clean();
      if(connection_aborted() || (connection_status() != CONNECTION_NORMAL)){
         write_log("aborted");
      } else {
         ob_flush();
         flush();
         while(ob_get_level() > 0){
            ob_end_flush();
         }
      }
      write_log("Error Function");
      exit;
   }

   function sig_handler($signo)
{

     switch ($signo) {
         case SIGTERM:
             // handle shutdown tasks
             write_log("Shutdown task");
             exit;
             break;
         case SIGHUP:
             // handle restart tasks
             write_log("Restart task");
             break;
         case SIGUSR1:
            write_log("SigUsr1");
             echo "Caught SIGUSR1...\n";
             break;
         default:
            write_log("other sig");
             // handle all other signals

     }

}

?>