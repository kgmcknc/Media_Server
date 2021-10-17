
var current_user = "";
var current_user_data = "";
var current_folder = "";
var media_data_list = "";
var media_list_array = [];

var autoplay_type = 0;
var autoplay_amount = 0;
var autoplay_remaining = 0;

java_formatting();
loadcookies();
load_user_data();
load_folder_data();

setInterval(second_pulse_update, 1000);

function java_formatting(){
   main = document.getElementById("main_page");
   media = document.getElementById("media_data");
   if(document.defaultView.innerWidth > document.defaultView.innerHeight){
      main.style.width = "50%";
      main.style.height = "100%";
      main.style.top = "20px";
      main.style.left = "20px";
      
      media.style.width = "50%";
      media.style.height = "100%";
      media.style.top = "20px";
      media.style.left = "50%";
   } else {
      main.style.width = "100%";
      main.style.height = "45%";
      main.style.top = "20px";
      main.style.left = "20px";
      
      media.style.width = "100%";
      media.style.height = "50%";
      media.style.top = "50%";
      media.style.left = "20px";
   }
}

function add_new_folder(){
   new_folder_text = document.getElementById("new_folder_name_text");
   new_folder_name = new_folder_text.value;
   new_folder_text.value = "";
   create_folder(new_folder_name);
}

function create_folder(new_folder_name){
   set_db_data({"command":"add_media_folder","path":new_folder_name}, load_folder_data);
}

function remove_current_folder(){
   if(current_folder != ""){
      remove_folder(current_folder);
   }
}

function remove_folder(folder_to_delete){
   set_db_data({"command":"rem_media_folder","path":folder_to_delete}, load_folder_data);
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
   get_db_data({"command":"get_media_folders"}, load_folder_list);
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

function update_autoplay(){
   autoplaylist = document.getElementById("autoplaydropdown");
   if(autoplaylist.selectedIndex >= 0){
      autoplay_type = autoplaylist[autoplaylist.selectedIndex].value;
      if(autoplay_type != ""){
         set_db_data({"command":"set_user_data","user_name":current_user,"update_field":"autoplay_type", "update_data":autoplay_type}, reset_autoplay_index());
      }
   }
}

function update_autoplay_amount(){
   autoplay_amount = document.getElementById("autoplaynumber").value;
   set_db_data({"command":"set_user_data","user_name":current_user,"update_field":"autoplay_amount", "update_data":autoplay_amount}, reset_autoplay_index());
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
   set_db_data({"command":"index_media_folder","path":current_folder}, update_media_list);
}

function set_media_text(){
   media = this.id;
   box = document.getElementById("movie_text");
   box.value = media;
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
      if(current_user_data == ""){
         get_db_data({"command":"get_user_data","user_name":userlist[0].value}, set_current_user);
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
         set_db_data({"command":"set_user_data","user_name":current_user,"update_field":"autoplay_type", "update_data":autoplaylist.value});
         set_db_data({"command":"set_user_data","user_name":current_user,"update_field":"autoplay_amount", "update_data":autoplayinput.value});
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
   set_db_data({"command":"set_user_data","user_name":current_user,"update_field":"last_played", "update_data":last_item});
}

function loadcookies(){
   current_user = getCookie("current_user");
   current_folder = getCookie("current_folder");
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

function add_list_event(item){
   item.addEventListener("click", function() {
   this.parentElement.querySelector(".nested").classList.toggle("active");
   this.classList.toggle("caret-down");
   });
}

function load_media_data(folder){
   get_db_data({"command":"get_media_data", "path":folder}, update_media_list);
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
   load_movie_data();
   reset_autoplay_index();
   setlastplayed();
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

function second_pulse_update(){
   // check and update position in currently playing / last played (maybe every 10 seconds)

}
