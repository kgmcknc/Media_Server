
var current_user = "";
var current_user_data = "";

function get_init_data(){
   loadcookies();
   load_user_data();
}

function add_new_user(){
   new_user_text = document.getElementById("new_user_name_text");
   new_user_name = new_user_text.value;
   new_user_text.value = "";
   create_user(new_user_name);
}

function create_user(new_user_name){
   set_db_data({"command":"add_user","user_name":new_user_name}, load_user_data);
}

function remove_current_user(){
   if(current_user != ""){
      remove_user(current_user);
   }
}

function remove_user(user_to_delete){
   set_db_data({"command":"rem_user","user_name":user_to_delete}, load_user_data);
}

function update_user(){
   userlist = document.getElementById("userdropdown");
   if(userlist.selectedIndex >= 0){
      new_user = userlist[userlist.selectedIndex].value;
      if(new_user != ""){
         set_active_user(new_user);
      }
   }
}

function set_active_user(new_user){
   get_db_data({"command":"get_user_data","user_name":new_user}, set_current_user);
}

function load_user_data(){
   get_db_data({"command":"get_users"}, load_user_list);
}

function load_user_list(users){
   userlist = document.getElementById("userdropdown");
   // empty current list
   while(userlist.length > 0){
      userlist.remove(0);
   }
   // rebuild list from stored users
   for(x=0;x<users.length;x++){
      new_option = document.createElement("option");
      new_option.text = users[x];
      userlist.add(new_option);
   }
   if(current_user == ""){
      if(userlist.length > 0){
         get_db_data({"command":"get_user_data","user_name":userlist[0].value}, set_current_user);
      }
   } else {
      // leave selection on current user
      for(x=0;x<userlist.length;x++){
         if(userlist[x].value == current_user){
            userlist.selectedIndex = x;
         }
      }
   }
}

function set_current_user(new_user_data){
   // load last media item from that user
   if(new_user_data){
      current_user = new_user_data.user_name;
      current_user_data = new_user_data;
      setCookie("current_user", current_user, 365);
   }
}

function setlastplayed(){
   set_db_data({"command":"set_user_data","user_name":current_user,"update_field":"last_played", "update_data":"test_last.mp4"});
}

function loadcookies(){
   current_user = getCookie("current_user");
}

function get_db_data(request_data, callback){
   var xmlhttp;
   var object = request_data;//{"command":"get_user_data","user_name":"test_user"};
   var valuestring = JSON.stringify(object);
   
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
         if(callback){
            response_json = JSON.parse(this.responseText);
            if(response_json.result != "DB_ERR"){
               callback(response_json.result);
            }
         }
      }
   }
}

function set_db_data(request_data, callback){
   var xmlhttp;
   var object = request_data;//{"command":"get_user_data","user_name":"test_user"};
   var valuestring = JSON.stringify(object);
   
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
         if(callback){
            response_json = JSON.parse(this.responseText);
            if(response_json.result != "DB_ERR"){
               callback(response_json.result);
            }
         }
      }
   }
}

function setCookie(cname, cvalue, exdays) {
   const d = new Date();
   d.setTime(d.getTime() + (exdays*24*60*60*1000));
   let expires = "expires="+ d.toUTCString();
   document.cookie = cname + "=" + cvalue + ";" + expires + ";path=/";
}

function getCookie(cname) {
   let name = cname + "=";
   let decodedCookie = decodeURIComponent(document.cookie);
   let ca = decodedCookie.split(';');
      for(let i = 0; i <ca.length; i++) {
         let c = ca[i];
         while (c.charAt(0) == ' ') {
            c = c.substring(1);
         }
         if (c.indexOf(name) == 0) {
         return c.substring(name.length, c.length);
      }
   }
   return "";
}

function checkCookie(cname) {
   let cval = getCookie(cname);
   if (cval != "") {
      return cval
   } else {
      return ""
   }
}
