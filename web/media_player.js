
function playmovie(){
    var object = document.getElementById("movie_text");
    var valuestring = "media_player.php?media_file=" + object.value;
    var player_box = document.getElementById("media_box");
    var player_source = document.getElementById("mediaplayer");
    player_box.pause();
    player_source.src = valuestring;
    player_box.load();
}

var global_episode = 1;

function playnaruto(){
   var episode_num = String(global_episode);
   if(global_episode < 10){
      episode_num = "00" + String(global_episode);
   } else {
      if(global_episode < 100){
         episode_num = "0" + String(global_episode);
      }
   }
   var episode = "TV Shows/Naruto Shippuden/Naruto_" + episode_num + ".mp4";
   var valuestring = "media_player.php?media_file=" + episode;
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

function set_episode(){
   var object = document.getElementById("episode_num");
   global_episode = parseInt(object.value);
   setCookie("last_episode", global_episode, 365);
}

function next_episode(){
   var object = document.getElementById("episode_num");
   global_episode = global_episode + 1;
   object.value = global_episode;
   setCookie("last_episode", global_episode, 365);
   playnaruto();
}

function previous_episode(){
   var object = document.getElementById("episode_num");
   global_episode = global_episode - 1;
   object.value = global_episode;
   setCookie("last_episode", global_episode, 365);
   playnaruto();
}

function loadcookies(){
   var ep_num = getCookie("last_episode");
   if(ep_num != ""){
      var object = document.getElementById("episode_num");
      global_episode = parseInt(ep_num);
      object.value = parseInt(ep_num);
   } else {
      global_episode = 1;
      setCookie("last_episode", global_episode, 365);
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
