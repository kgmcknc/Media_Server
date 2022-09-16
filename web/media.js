
var current_user = "";
var current_user_data = "";
var current_folder = "";
var media_data_list = "";
var media_list_array = [];
var device_list_array = [];

var autoplay_type = 0;
var autoplay_amount = 0;
var autoplay_remaining = 0;
var show_device_controls = 0;
var show_web_player = 0;
var web_player_loaded = 0;

document.addEventListener('click', mouse_dropdown_hide);

java_formatting();
loadcookies();
get_device_list();
load_user_data();
load_folder_data();

setInterval(periodic_media_update, 3500);
setInterval(periodic_get_devices, 30000);

function java_formatting(){
   main = document.getElementById("main_page");
   //media = document.getElementById("media_data");
   //if(document.defaultView.innerWidth > document.defaultView.innerHeight){
      main.style.width = "95%";
      main.style.height = "95%";
      main.style.top = "20px";
      main.style.left = "20px";
      
      //media.style.width = "95%";
      //media.style.height = "95%";
      //media.style.top = "20px";
      //media.style.left = "50%";
   //} else {
      //main.style.width = "100%";
      //main.style.height = "45%";
      //main.style.top = "20px";
      //main.style.left = "20px";
      
      //media.style.width = "100%";
      //media.style.height = "50%";
      //media.style.top = "50%";
      //media.style.left = "20px";
   //}
}

function set_display(value){
   devicelist = document.getElementById("devicedropdown");
   if(devicelist.selectedIndex > 0){
      device = device_list_array[devicelist.selectedIndex-1];
      device_id = device.device_id;
      command = "/media/set_display"
      command_string = '{"/global/' + device_id + command + '":"' + value + '"}'
      command_json = JSON.parse(command_string)
      set_db_data(command_json);
   }
}

function add_new_folder(){
   new_folder_text = document.getElementById("new_folder_name_text");
   new_folder_name = new_folder_text.value;
   new_folder_text.value = "";
   create_folder(new_folder_name);
}

function create_folder(new_folder_name){
   set_db_data({"/database/add_media_folder":"new_folder_name"}, load_folder_data);
}

function remove_current_folder(){
   if(current_folder != ""){
      remove_folder(current_folder);
   }
}

function remove_folder(folder_to_delete){
   set_db_data({"/database/rem_media_folder":folder_to_delete}, load_folder_data);
}

function add_new_link_folder(){
   new_folder_text = document.getElementById("new_link_folder_name_text");
   new_folder_name = new_folder_text.value;
   new_folder_text.value = "";
   folderlist = document.getElementById("folderdropdown");
   if(folderlist.selectedIndex >= 0){
      original_folder = folderlist[folderlist.selectedIndex].value;
      create_link_folder(original_folder, new_folder_name);
   }
}

function create_link_folder(original_folder, new_folder){
   devicelist = document.getElementById("devicedropdown");
   if(devicelist.selectedIndex > 1){
      device = device_list_array[devicelist.selectedIndex-1];
      device_id = device.device_id;
      command = "/database/add_media_link"
      value = '{"src_path":"'+original_folder+'", "dst_path":"'+new_folder+'"}'
      command_string = '{"/global/' + device_id + command + '":' + value + '}'
      command_json = JSON.parse(command_string)
      set_db_data(command_json);
   }
}

function remove_current_link_folder(){
   new_folder_text = document.getElementById("new_link_folder_name_text");
   new_folder_name = new_folder_text.value;
   new_folder_text.value = "";
   folderlist = document.getElementById("folderdropdown");
   if(folderlist.selectedIndex >= 0){
      original_folder = folderlist[folderlist.selectedIndex].value;
      remove_link_folder(original_folder, new_folder_name);
   }
}

function remove_link_folder(original_folder, new_folder){
   devicelist = document.getElementById("devicedropdown");
   if(devicelist.selectedIndex > 0){
      device = device_list_array[devicelist.selectedIndex-1];
      device_id = device.device_id;
      command = "/database/rem_media_link"
      value = '{"src_path":"'+original_folder+'", "dst_path":"'+new_folder+'"}'
      command_string = '{"/global/' + device_id + command + '":' + value + '}'
      command_json = JSON.parse(command_string)
      set_db_data(command_json);
   }
}

function update_folder(){
   folderlist = document.getElementById("folderdropdown");
   if(folderlist.selectedIndex >= 0){
      new_folder = folderlist[folderlist.selectedIndex].value;
      if(new_folder != ""){
         set_active_folder(new_folder);
      }
   }
}

function set_active_folder(new_folder){
   current_folder = new_folder;
   load_media_data(current_folder);
}

function load_folder_data(){
   get_db_data({"/database/get_media_folders":""}, load_folder_list);
}

function load_folder_list(folders){
   folderlist = document.getElementById("folderdropdown");
   // empty current list
   while(folderlist.length > 0){
      folderlist.remove(0);
   }
   // rebuild list from stored folders
   for(x=0;x<folders.length;x++){
      new_option = document.createElement("option");
      new_option.text = folders[x];
      folderlist.add(new_option);
   }
   if(current_folder == ""){
      if(folderlist.length > 0){
         set_active_folder(folderlist[0].value);
      }
   } else {
      if(media_data_list == ""){
         set_active_folder(folderlist[0].value);
      }
      // leave selection on current folder
      for(x=0;x<folderlist.length;x++){
         if(folderlist[x].value == current_folder){
            folderlist.selectedIndex = x;
         }
      }
   }
}

function add_new_user(){
   new_user_text = document.getElementById("new_user_name_text");
   new_user_name = new_user_text.value;
   new_user_text.value = "";
   create_user(new_user_name);
}

function create_user(new_user_name){
   set_db_data({"/database/add_user":new_user_name}, load_user_data);
}

function remove_current_user(){
   if(current_user != ""){
      remove_user(current_user);
   }
}

function remove_user(user_to_delete){
   set_db_data({"/database/rem_user":user_to_delete}, load_user_data);
}

function update_user(){
   userlist = document.getElementById("userdropdown");
   if(userlist.selectedIndex >= 0){
      new_user = userlist[userlist.selectedIndex].value;
      if(new_user != ""){
         set_active_user(new_user);
      }
   }
   hide_users();
}

function update_autoplay(){
   autoplaylist = document.getElementById("autoplaydropdown");
   if(autoplaylist.selectedIndex >= 0){
      autoplay_type = autoplaylist[autoplaylist.selectedIndex].value;
      if(autoplay_type != ""){
         set_db_data({"/database/set_user_data":{"user_name":current_user,"update_field":"autoplay_type", "update_data":autoplay_type}}, reset_autoplay_index());
      }
   }
}

function update_autoplay_amount(){
   autoplay_amount = document.getElementById("autoplaynumber").value;
   set_db_data({"/database/set_user_data":{"user_name":current_user,"update_field":"autoplay_amount", "update_data":autoplay_amount}}, reset_autoplay_index());
}

function reset_autoplay_index(){
   autoplay_remaining = autoplay_amount;
}

function check_autoplay(){
   var play_next = 0;
   if(autoplay_type == "autoplay_forever"){
      play_next = 1;
   }
   if(autoplay_type == "autoplay_fixed_number"){
      if(autoplay_remaining > 0){
         play_next = 1;
         autoplay_remaining = autoplay_remaining - 1;
      }
   }
   if(autoplay_type == "autoplay_end_of_folder"){
      end_of_folder = check_media_end_of_folder();
      if(end_of_folder){
         play_next = 0;
      } else {
         play_next = 1;
      }
   }
   web_player_loaded = play_next;
   if(play_next){
      episode = get_next_episode();
      if(episode != -1){
         document.getElementById("movie_text").value = episode;
         show_list_element(episode);
         load_movie_data();
         setlastplayed();
      }
   }
}

function index_media_folder(){
   set_db_data({"/database/index_media_folder":current_folder}, update_media_list);
}

function set_media_text(){
   media = this.id;
   box = document.getElementById("movie_text");
   box.value = media;
}

function set_active_user(new_user){
   get_db_data({"/database/get_user_data":new_user}, set_current_user);
}

function get_device_list(){
   get_db_data({"/database/get_db_devices":""}, load_device_list);
}

function periodic_get_devices(){
   if(!document.hidden){
      get_device_list();
   }
}

function load_device_list(devices){
   old_id = ""
   if(device_list_array.length > 1){
      old_device = device_list_array[devicelist.selectedIndex]
      old_id = old_device.device_id
   }
   devicelist = document.getElementById("devicedropdown");
   device_list_array = devices
   // empty current list
   while(devicelist.length > 0){
      devicelist.remove(0);
   }

   new_option = document.createElement("option");
   new_option.text = "Web Player";
   devicelist.add(new_option);

   // rebuild list from stored devices
   for(x=0;x<devices.length;x++){
      new_option = document.createElement("option");
      new_option.text = devices[x].name;
      devicelist.add(new_option);
      if(old_id != ""){
         if(devices[x].device_id == old_id){
            devicelist.selectedIndex = x;
         }
      }
   }
   if(devicelist.selectedIndex > 0){
      show_device_controls = 1;
   } else {
      show_device_controls = 0;
   }
   update_visibility();
}

function update_device_name(){
   new_device_name_text = document.getElementById("new_device_name_text");
   new_device_name = new_device_name_text.value;
   new_device_name_text.value = "";
   command = {"/database/update_config":{"name":new_device_name}}
   global_set_db_data(command);
}

function load_user_data(){
   get_db_data({"/database/get_users":""}, load_user_list);
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
         get_db_data({"/database/get_user_data":userlist[0].value}, set_current_user);
      }
   } else {
      if(current_user_data == ""){
         get_db_data({"/database/get_user_data":userlist[0].value}, set_current_user);
      }
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

      if(current_user_data.last_played != undefined){
         document.getElementById("movie_text").value = current_user_data.last_played;
         show_list_element(current_user_data.last_played);
      }

      autoplaylist = document.getElementById("autoplaydropdown");
      autoplayinput = document.getElementById("autoplaynumber");
      if((new_user_data.autoplay_type != undefined) && (new_user_data.autoplay_amount != undefined)){
         autoplay_type = new_user_data.autoplay_type;
         autoplay_amount = new_user_data.autoplay_amount;
         
         for(x=0;x<autoplaylist.length;x++){
            if(autoplaylist[x].value == autoplay_type){
               autoplaylist.selectedIndex = x;
               break;
            }
         }
         autoplayinput.value = autoplay_amount;
      } else {
         set_db_data({"/database/set_user_data":{"user_name":current_user,"update_field":"autoplay_type", "update_data":autoplaylist.value}});
         set_db_data({"/database/set_user_data":{"user_name":current_user,"update_field":"autoplay_amount", "update_data":autoplayinput.value}});
      }
   }
}

function show_list_element(element_to_show){
   media_list = document.getElementById("media_list");
   list_element = document.getElementById(element_to_show);
   if(list_element != null){
      // make sure all parent list elements are visible
      next_parent = list_element.parentElement;
      while(next_parent != media_list){
         element = next_parent;
         next_parent = element.parentElement;
         class_string = element.classList.toString();
         if(class_string.indexOf("nested") >= 0){
            //span element is in sibling
            element.classList.add("active");
            for(var x=0;x<next_parent.childElementCount;x++){
               class_string = next_parent.children[x].classList.toString();
               if(class_string.indexOf("caret") >= 0){
                  next_parent.children[x].classList.add("caret-down");
                  break;
               }
            }
         }
         //next_parent = element.parentElement;
      }
      list_element.scrollIntoView(true);
   }
}

function setlastplayed(){
   last_item = document.getElementById("movie_text").value;
   set_db_data({"/database/set_user_data":{"user_name":current_user,"update_field":"last_played", "update_data":last_item}});
}

function loadcookies(){
   current_user = getCookie("current_user");
   current_folder = getCookie("current_folder");
}

function get_db_data(request_data, callback){
   var xmlhttp;
   if(typeof(request_data) == "string"){
      var req_object = {};
      req_object[request_data] = "";
   } else {
      if(typeof(request_data) == "object"){
         var req_object = request_data;
      } else {
         return
      }
   }
   var valuestring = JSON.stringify(req_object);
   
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
         if(callback){
            try {
               response_json = JSON.parse(this.responseText);
            } catch (e) {
               return
            }
            if(response_json != "DB_ERR"){
               callback(response_json);
            }
         }
      }
   }
}

function global_get_db_data(request_data, callback){
   devicelist = document.getElementById("devicedropdown");
   if(devicelist.selectedIndex > 0){
      device = device_list_array[devicelist.selectedIndex-1];
      device_id = device.device_id;
      var xmlhttp;
      if(typeof(request_data) == "string"){
         var req_object = {};
         req_object[request_data] = "";
      } else {
         if(typeof(request_data) == "object"){
            var req_object = request_data;
         } else {
            return
         }
      }
      var new_request = JSON.stringify(req_object);
      var valuestring = new_request.slice(0,2) + "/global/" + device_id + new_request.slice(2)
      
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
            if(callback){
               try {
                  response_json = JSON.parse(this.responseText);
               } catch (e) {
                  return
               }
               if(response_json != "DB_ERR"){
                  callback(response_json);
               }
            }
         }
      }
   } else {
      get_db_data(request_data, callback);
   }
}

function set_db_data(request_data, callback){
   var xmlhttp;
   if(typeof(request_data) == "string"){
      var req_object = {};
      req_object[request_data] = "";
   } else {
      if(typeof(request_data) == "object"){
         var req_object = request_data;
      } else {
         return
      }
   }
   var valuestring = JSON.stringify(req_object);
   
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
            try {
               response_json = JSON.parse(this.responseText);
            } catch (e) {
               return
            }
            if(response_json != "DB_ERR"){
               callback(response_json);
            }
         }
      }
   }
}

function global_set_db_data(request_data, callback){
   devicelist = document.getElementById("devicedropdown");
   if(devicelist.selectedIndex >= 0){
      device = device_list_array[devicelist.selectedIndex-1];
      device_id = device.device_id;
      var xmlhttp;
      if(typeof(request_data) == "string"){
         var req_object = {};
         req_object[request_data] = "";
      } else {
         if(typeof(request_data) == "object"){
            var req_object = request_data;
         } else {
            return
         }
      }
      var new_request = JSON.stringify(req_object);
      var valuestring = new_request.slice(0,2) + "/global/" + device_id + new_request.slice(2)
      
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
               try {
                  response_json = JSON.parse(this.responseText);
               } catch (e) {
                  return
               }
               if(response_json != "DB_ERR"){
                  callback(response_json);
               }
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

function add_list_event(item){
   item.addEventListener("click", function() {
   this.parentElement.querySelector(".nested").classList.toggle("active");
   this.classList.toggle("caret-down");
   });
}

function load_media_data(folder){
   get_db_data({"/database/get_media_data":folder}, update_media_list);
}

function update_media_list(folder_data){
   media_data_list = folder_data.data;
   media_list = document.getElementById("media_list");
   media_list.name = current_folder;
   while(media_list.childElementCount > 0){
      media_list.removeChild(media_list.children[0]);
   }
   media_list_array = [];
   iterate_media_list(media_list, media_data_list);
}

function iterate_media_list(media_list, media_data){
   for(const data in media_data){
      if(media_data[data].name != undefined){
         // item in current list
         new_element = document.createElement('li');
         new_element.innerText = media_data[data].name;
         new_element.id = media_data[data].path;
         media_list_array.push(media_data[data].path);
         new_element.addEventListener("click", set_media_text);
         media_list.append(new_element);
      } else {
         // folder item
         new_element = document.createElement('li');
         new_element.name = "folder";
         new_span = document.createElement('span');
         new_span.className = "caret";
         new_span.innerText = data;
         add_list_event(new_span);
         new_element.append(new_span);
         //new_element.id = data;
         media_list.append(new_element);
         saved_list = document.createElement('ul');
         saved_list.name = "subfolder"
         saved_list.className = "nested";
         //saved_list.innerText = data;
         //saved_list.id = data;
         new_element.append(saved_list);
         iterate_media_list(saved_list, media_data[data]);
      }
   }
}

function playmovie(){
   devicelist = document.getElementById("devicedropdown");
   show_web_player = devicelist.selectedIndex == 0;
   if(show_web_player){
      web_player_loaded = 1;
      load_movie_data();
      reset_autoplay_index();
      setlastplayed();
   } else {
      var object = document.getElementById("movie_text");
      command = {"/media/start":{"base_path":current_folder,"file_path":object.value}}
   
      global_set_db_data(command)
   }
   update_visibility();
}

// function startmedia(){
//    var object = document.getElementById("movie_text");
//    command = {"/media/start":{"base_path":current_folder,"file_path":object.value}}

//    global_set_db_data(command)
// }

function stopmedia(){
   devicelist = document.getElementById("devicedropdown");
   show_web_player = devicelist.selectedIndex == 0;
   if(show_web_player){
      web_player_loaded = 0;
      show_web_player = 0;
      web_player_stop();
      closeFullscreen();
   } else {
      device_stop();
   }
   update_visibility();
}
function web_player_stop(){
   var player_box = document.getElementById("media_box");
   player_box.pause();
   player_box.currentTime = 0;
}
function device_stop(){
   global_set_db_data({"/media/stop":""})
}

function playmedia(){
   devicelist = document.getElementById("devicedropdown");
   show_web_player = devicelist.selectedIndex == 0;
   if(show_web_player){
      web_player_play();
   } else {
      device_play();
   }
   update_visibility();
}
function web_player_play(){
   var player_box = document.getElementById("media_box");
   player_box.play();
}
function device_play(){
   global_set_db_data({"/media/play":""})
}

function pausemedia(){
   devicelist = document.getElementById("devicedropdown");
   show_web_player = devicelist.selectedIndex == 0;
   if(show_web_player){
      web_player_pause();
   } else {
      device_pause();
   }
   update_visibility();
}
function web_player_pause(){
   var player_box = document.getElementById("media_box");
   player_box.pause();
}
function device_pause(){
   global_set_db_data({"/media/pause":""})
}

function toggle_fullscreen(){
   global_set_db_data({"/media/toggle_fullscreen":""})
}

function load_movie_data(){
   var object = document.getElementById("movie_text");
   var valuestring = "media_player.php?media_file=" + object.value;
   var player_box = document.getElementById("media_box");
   var player_source = document.getElementById("mediaplayer");
   player_box.pause();
   if(player_source.src != valuestring){
     player_box.currentTime = 0;
     player_source.src = valuestring;
     player_box.load();
   }
   player_box.play();
}

function next_episode(){
   episode = get_next_episode();
   if(episode != -1){
      document.getElementById("movie_text").value = episode;
      show_list_element(episode);
      load_movie_data();
      reset_autoplay_index();
      setlastplayed();
   }
}

function get_next_episode(){
   media_list = document.getElementById("media_list");
   current_item = document.getElementById("movie_text").value;
   list_element = document.getElementById(current_item);
   error = 0;
   
   // if we are already at end of current folder, move forward until we have a next sibling
   while(list_element.nextSibling == null){
      // need to go up 2 levels to find next item we can try
      // search until we find next item or run out of items (get to top)
      if(list_element.parentElement == media_list){
         error = 1;
         break;
      } else {
         // go up two levels (ul/li) to next folder
         if(list_element.parentElement.name == "subfolder"){
            list_element = list_element.parentElement;
         } else {
            error = 1;
            break
         }
         if(list_element.parentElement.name == "folder"){
            list_element = list_element.parentElement;
         } else {
            error = 1;
            break;
         }
      }
   }

   if(error == 0){
      // move to next item
      list_element = list_element.nextSibling;
      // move into any subfolders as deep as needed
      while(list_element.name == "folder"){
         if(list_element.childElementCount > 0){
            // move into folder
            list_element = list_element.children[0];
         } else {
            // should be child for sub folder
            error = 1;
            break;
         }
         // should find subfolder
         while(list_element.name != "subfolder"){
            if(list_element.nextSibling == null){
               // ran out of siblings before finding subfolder
               error = 1;
               break;
            } else {
               // move to next sibling
               list_element = list_element.nextSibling;
            }
         }
         // found subfolder. Set item as first child
         if(list_element.childElementCount > 0){
            list_element = list_element.children[0];
         } else {
            // somehow didn't have item in subfolder
            error = 1;
            break;
         }
      }
   }
   if(error == 0){
      return list_element.id;
   } else {
      return -1;
   }
}

function check_media_end_of_folder(){
   current_item = document.getElementById("movie_text").value;
   list_element = document.getElementById(current_item);
   if(list_element.nextSibling == null){
      // end of folder
      return 1;
   } else {
      // not end of folder
      return 0;
   }
}

function previous_episode(){
   episode = get_previous_episode();
   if(episode != -1){
      document.getElementById("movie_text").value = episode;
      show_list_element(episode);
      load_movie_data();
      reset_autoplay_index();
      setlastplayed();
   }
}

function get_previous_episode(){
   media_list = document.getElementById("media_list");
   current_item = document.getElementById("movie_text").value;
   list_element = document.getElementById(current_item);
   error = 0;
   
   // if we are already at beginning of current folder, move back until we have a previous sibling
   while(list_element.previousSibling == null){
      // need to go up 2 levels to find next item we can try
      // search until we find next item or run out of items (get to top)
      if(list_element.parentElement == media_list){
         error = 1;
         break;
      } else {
         // go up two levels (ul/li) to next folder
         if(list_element.parentElement.name == "subfolder"){
            list_element = list_element.parentElement;
         } else {
            error = 1;
            break
         }
         if(list_element.parentElement.name == "folder"){
            list_element = list_element.parentElement;
         } else {
            error = 1;
            break;
         }
      }
   }

   if(error == 0){
      // move to previous item
      list_element = list_element.previousSibling;
      // move into any subfolders as deep as needed
      while(list_element.name == "folder"){
         if(list_element.childElementCount > 0){
            // move into folder
            list_element = list_element.children[0];
         } else {
            // should be child for sub folder
            error = 1;
            break;
         }
         // should find subfolder
         while(list_element.name != "subfolder"){
            if(list_element.nextSibling == null){
               // ran out of siblings before finding subfolder
               error = 1;
               break;
            } else {
               // move to next sibling
               list_element = list_element.nextSibling;
            }
         }
         // found subfolder. Set item as last child
         if(list_element.childElementCount > 0){
            list_element = list_element.children[list_element.childElementCount-1];
         } else {
            // somehow didn't have item in subfolder
            error = 1;
            break;
         }
      }
   }
   if(error == 0){
      return list_element.id
   } else {
      return -1;
   }
}

var media_active = 0
var media_duration = 0
var media_time = 0
var media_position = 0
var media_range_slider = document.getElementById("media_range");
var media_time_display = document.getElementById("media_time_div")

function periodic_media_update(){
   // check and update position in currently playing / last played (maybe every 10 seconds)
   if(!document.hidden){
      global_get_db_data("/media/get_status",get_media_status)
   }
   //if(media_active){
      //get_db_data("/media/get_status",get_media_time)
   //} else {
      //get_db_data("/media/is_active",get_media_active)
   //}
}

function get_media_status(media_status){
   media_active = media_status.active
   if(media_active){
      media_duration = media_status.total_time
      media_time = media_status.current_time
      media_position = media_status.current_position
   }
   update_media_time()
}

function ffmedia(){
   time_change = 20000
   if((media_time + time_change) < media_duration){
      global_set_db_data({"/media/set_time":media_time+time_change},update_media_time)
   }
}

function rwmedia(){
   time_change = 20000
   if(media_time > time_change){
      new_time = media_time - time_change
   } else {
      new_time = 0
   }
   global_set_db_data({"/media/set_time":new_time},update_media_time)
}

function mediasliderchange(){
   new_position = media_range_slider.value/100
   global_set_db_data({"/media/set_position":new_position},update_media_time)
}

function get_media_active(media_active_status){
   media_active = media_active_status
}

function get_media_time(media_time_status){
   media_duration = media_time_status.total_time
   media_time = media_time_status.current_time
   media_position = media_time_status.current_position
}

function update_media_time(){
   if(media_active){
      slider_value = (media_position*100)
      media_range_slider.value = slider_value.toFixed(0)
      total_seconds = (media_duration/1000)%60
      total_minutes = (media_duration/60000)%60
      total_hours = (media_duration/3600000)%60
      media_seconds = (media_time/1000)%60
      media_minutes = (media_time/60000)%60
      media_hours = (media_time/3600000)%60
      total_seconds = total_seconds.toFixed(0)
      total_minutes = total_minutes.toFixed(0)
      total_hours = total_hours.toFixed(0)
      media_seconds = media_seconds.toFixed(0)
      media_minutes = media_minutes.toFixed(0)
      media_hours = media_hours.toFixed(0)
      media_time_display.innerHTML = media_hours+":"+media_minutes+":"+media_seconds+" of "+total_hours+":"+total_minutes+":"+total_seconds
   } else {
      media_time_display.innerHTML = "0:0:0 of 0:0:0"
      media_range_slider.value = 1
   }
}

// Make the DIV element draggable:
//dragElement(document.getElementById("video_div"));

function dragElement(elmnt) {
  var pos1 = 0, pos2 = 0, pos3 = 0, pos4 = 0;
  if (document.getElementById(elmnt.id + "header")) {
    // if present, the header is where you move the DIV from:
    document.getElementById(elmnt.id + "header").onmousedown = dragMouseDown;
  } else {
    // otherwise, move the DIV from anywhere inside the DIV:
    elmnt.onmousedown = dragMouseDown;
  }

  function dragMouseDown(e) {
    e = e || window.event;
    e.preventDefault();
    // get the mouse cursor position at startup:
    pos3 = e.clientX;
    pos4 = e.clientY;
    document.onmouseup = closeDragElement;
    // call a function whenever the cursor moves:
    document.onmousemove = elementDrag;
  }

  function elementDrag(e) {
    e = e || window.event;
    e.preventDefault();
    // calculate the new cursor position:
    pos1 = pos3 - e.clientX;
    pos2 = pos4 - e.clientY;
    pos3 = e.clientX;
    pos4 = e.clientY;
    // set the element's new position:
    elmnt.style.top = (elmnt.offsetTop - pos2) + "px";
    elmnt.style.left = (elmnt.offsetLeft - pos1) + "px";
  }

  function closeDragElement() {
    // stop moving when mouse button is released:
    document.onmouseup = null;
    document.onmousemove = null;
  }
}
var settings_visible = 0;
var settings_clicked = 0;
function settings_button(){
   hide_users();
   hide_devices();
   hide_sleep();
   if(settings_visible){
      hide_settings();
   } else {
      show_settings();
   }
   settings_clicked = 1;
}
function show_settings(){
   settings_div = document.getElementById("settings_div");
   settings_div.style.visibility = "visible";
   settings_visible = 1;
}
function hide_settings(){
   settings_div = document.getElementById("settings_div");
   settings_div.style.visibility = "";
   settings_visible = 0;
}
var users_visible = 0;
var users_clicked = 0;
function users_button(){
   hide_settings();
   hide_devices();
   hide_sleep();
   if(users_visible){
      hide_users();
   } else {
      show_users();
   }
   users_clicked = 1;
}
function show_users(){
   username_div = document.getElementById("username_div");
   username_div.style.visibility = "visible";
   users_visible = 1;
}
function hide_users(){
   username_div = document.getElementById("username_div");
   username_div.style.visibility = "";
   users_visible = 0;
}
var devices_visible = 0;
var devices_clicked = 0;
function devices_button(){
   hide_settings();
   hide_users();
   hide_sleep();
   if(devices_visible){
      hide_devices();
   } else {
      show_devices();
   }
   devices_clicked = 1;
}
function show_devices(){
   devices_div = document.getElementById("devices_div");
   devices_div.style.visibility = "visible";
   devices_visible = 1;
}
function hide_devices(){
   devices_div = document.getElementById("devices_div");
   devices_div.style.visibility = "";
   devices_visible = 0;
}

var sleep_visible = 0;
var sleep_clicked = 0;
function sleep_button(){
   hide_settings();
   hide_users();
   hide_devices();
   if(sleep_visible){
      hide_sleep();
   } else {
      show_sleep();
   }
   sleep_clicked = 1;
}
function show_sleep(){
   sleep_div = document.getElementById("sleep_div");
   sleep_div.style.visibility = "visible";
   sleep_visible = 1;
}
function hide_sleep(){
   sleep_div = document.getElementById("sleep_div");
   sleep_div.style.visibility = "";
   sleep_visible = 0;
}

function mouse_dropdown_hide(event){
   settings_div = document.getElementById("settings_div");
   username_div = document.getElementById("username_div");
   devices_div = document.getElementById("devices_div");
   sleep_div = document.getElementById("sleep_div");

   if(settings_div.contains(event.target)){
      settings_clicked = 1;
   }
   if(username_div.contains(event.target)){
      users_clicked = 1;
   }
   if(devices_div.contains(event.target)){
      devices_clicked = 1;
   }
   if(sleep_div.contains(event.target)){
      sleep_clicked = 1;
   }
   clear_visible();
}

function clear_visible(){
   if(settings_clicked == 0){
      if(settings_visible){
         settings_button(0);
      }
   }
   if(users_clicked == 0){
      if(users_visible){
         users_button(0);
      }
   }
   if(devices_clicked == 0){
      if(devices_visible){
         devices_button(0);
      }
   }
   settings_clicked = 0;
   users_clicked = 0;
   devices_clicked = 0;
}

function update_active_device(){
   devicelist = document.getElementById("devicedropdown");
   if(devicelist.selectedIndex > 0){
      show_device_controls = 1;
      if(web_player_loaded){
         web_player_pause();
      }
      show_web_player = 0;
   } else {
      show_device_controls = 0;
      if(web_player_loaded){
         show_web_player = 1;
         playmedia();
      }
   }
   update_visibility();
   hide_devices();
}

function update_visibility(){
   video_player = document.getElementById("video_div");
   media_control = document.getElementById("media_data");
   if(show_web_player == 0){
      video_player.style.display = "none";
      media_control.style.height = "65%";
   } else {
      video_player.style.display = "inline";
      media_control.style.height = "45%";
   }
   controls = document.getElementById("device_control");
   if(show_device_controls == 0){
      controls.style.display = "none";
   } else {
      controls.style.display = "inline";
   }
}

let sleep_timer_interval;
let sleep_timer_hours;
let sleep_timer_minutes;
let sleep_timer_active = 0;

function sleep_timer_button(){
   if(sleep_timer_active){
      stop_sleep_timer();
   } else {
      start_sleep_timer();
   }
}

function start_sleep_timer(){
   button = document.getElementById("sleep_timer_button");
   sleep_timer_interval = setInterval(sleep_minute_interrupt,60000);
   sleep_timer_active = 1;
   button.value = "Stop Sleep Timer";
   sleep_timer_hours = document.getElementById("hr_timer").value;
   sleep_timer_minutes = document.getElementById("min_timer").value;
   sleep_status = document.getElementById("sleep_status");
   sleep_status.innerText = sleep_timer_hours+" hr, "+sleep_timer_minutes+" mins";
   // we should cache each users sleep timer settings
}

function stop_sleep_timer(){
   button = document.getElementById("sleep_timer_button");
   clearInterval(sleep_timer_interval);
   sleep_timer_interval = null;
   sleep_timer_active = 0;
   button.value = "Start Sleep Timer";
   sleep_status = document.getElementById("sleep_status");
   sleep_timer_hours = 0;
   sleep_timer_minutes = 0;
   sleep_status.innerText = sleep_timer_hours+" hr, "+sleep_timer_minutes+" mins";
}

function sleep_minute_interrupt(){
   if(sleep_timer_minutes > 0){
      sleep_timer_minutes = sleep_timer_minutes - 1;
   }
   if(sleep_timer_minutes == 0){
      if(sleep_timer_hours == 0){
         sleep_timer_done();
      } else {
         sleep_timer_hours = sleep_timer_hours - 1;
         sleep_timer_minutes = 59;
      }
   }
   sleep_status = document.getElementById("sleep_status");
   sleep_status.innerText = sleep_timer_hours+" hr, "+sleep_timer_minutes+" mins";
}

function sleep_timer_done(){
   stop_sleep_timer();
   stopmedia();
   sleep_display = document.getElementById("screen_sleep_check");
   if(sleep_display.value == "on"){
      devicelist = document.getElementById("devicedropdown");
      if(devicelist.selectedIndex > 0){
         set_display('off');
      }
   }
}

function closeFullscreen() {
   if (document.exitFullscreen) {
     document.exitFullscreen();
   } else if (document.webkitExitFullscreen) { /* Safari */
     document.webkitExitFullscreen();
   } else if (document.msExitFullscreen) { /* IE11 */
     document.msExitFullscreen();
   }
 }