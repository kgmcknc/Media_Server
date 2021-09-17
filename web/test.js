function testgeturl(){
   var xmlhttp;
   var object = {"command":"get_active_devices"};
   var valuestring = JSON.stringify(object);
   //alert(valuestring);
   if (window.XMLHttpRequest) {
      xmlhttp = new XMLHttpRequest();
   } else {
      // code for IE6, IE5
      xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
   }
        xmlhttp.open("GET", "msp.php?q="+valuestring, true);
      xmlhttp.send();
      xmlhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
         //document.getElementById(value).innerHTML = this.responseText;
         alert(this.responseText);
      }
   }
  }
function testposturl(){
   var xmlhttp;
   //var object = {"command":"link_device","device_id":35402072021};
   var object = {"command":"update_config","hb_period":5};
   var valuestring = JSON.stringify(object);
   //alert(valuestring);
   if (window.XMLHttpRequest) {
      xmlhttp = new XMLHttpRequest();
   } else {
      // code for IE6, IE5
      xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
   }
        xmlhttp.open("POST", "msp.php?q="+valuestring, true);
   xmlhttp.send();
   xmlhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
         //document.getElementById(value).innerHTML = this.responseText;
         //alert(this.responseText);
      }
   }
  }
function testremurl(){
   var xmlhttp;
   var object = {"command":"rem_device_from_server","device_id":1963287791};
   var valuestring = JSON.stringify(object);
   //alert(valuestring);
   if (window.XMLHttpRequest) {
      xmlhttp = new XMLHttpRequest();
   } else {
      // code for IE6, IE5
      xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
   }
        xmlhttp.open("POST", "msp.php?q="+valuestring, true);
   xmlhttp.send();
   xmlhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
         //document.getElementById(value).innerHTML = this.responseText;
         //alert(this.responseText);
      }
   }
  }
function testplayurl(){
   var xmlhttp;
   var object = {"command":"play_movie","movie_name":"/home/pi/linux-main-share/MovieHD/The Emperors New Groove.mp4"};
   var valuestring = JSON.stringify(object);
   //alert(valuestring);
   if (window.XMLHttpRequest) {
      xmlhttp = new XMLHttpRequest();
   } else {
      // code for IE6, IE5
      xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
   }
        xmlhttp.open("POST", "msp.php?q="+valuestring, true);
   xmlhttp.send();
   xmlhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
         //document.getElementById(value).innerHTML = this.responseText;
         //alert(this.responseText);
      }
   }
  }
function testaddfolder(){
   var xmlhttp;
   var object = {"command":"add_media_folder","name":"test_folder_name","path":"/testpath/testing"};
   var valuestring = JSON.stringify(object);
   //alert(valuestring);
   if (window.XMLHttpRequest) {
      xmlhttp = new XMLHttpRequest();
   } else {
      // code for IE6, IE5
      xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
   }
        xmlhttp.open("POST", "msp.php?q="+valuestring, true);
   xmlhttp.send();
   xmlhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
         //document.getElementById(value).innerHTML = this.responseText;
         //alert(this.responseText);
      }
   }
  }