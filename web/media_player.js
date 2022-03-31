
var global_episode = 1;

function set_episode(){
   var object = document.getElementById("episode_num");
   global_episode = parseInt(object.value);
   setCookie("last_episode", global_episode, 365);
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
