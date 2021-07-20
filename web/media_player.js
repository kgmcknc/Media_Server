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
}

function next_episode(){
   var object = document.getElementById("episode_num");
   global_episode = global_episode + 1;
   object.value = global_episode;
   playnaruto();
}

function previous_episode(){
   var object = document.getElementById("episode_num");
   global_episode = global_episode - 1;
   object.value = global_episode;
   playnaruto();
}