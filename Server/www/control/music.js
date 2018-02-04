var directory_list;
var dir_tree = [];
var music_array = [];

function search_music(){
	var xmlhttp = 0;
	var dir = "/";
	var error_html;
	if (window.XMLHttpRequest) {
		xmlhttp = new XMLHttpRequest();
	} else {
		// code for IE6, IE5
		xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
	}
        xmlhttp.open("POST", "musicsearch.php?q=" + dir, true);
	xmlhttp.send();
	xmlhttp.onreadystatechange = function() {
		if (this.readyState == 4 && this.status == 200) {
			directory_list = this.responseText;
			if(directory_list == 0) {
				error_html = document.getElementById("musiclist");
				error_html.innerHTML = "Could Not Open Music Dir File";
			} else {
				if(directory_list == 1) {
					error_html = document.getElementById("musiclist");
					error_html.innerHTML = "No Music Directory Set";
				} else {
					dir_tree.push(dir);
					update_page();
				}
			}
		}
	}
}

function enter_dir(){
	var base = "music";
	var idname = this.id;
	var music_num = 0;
	var xmlhttp = 0;
	var dir;
	
	idname = idname.slice(base.length);
	music_num = string_to_dec(idname);

	if(music_num <= music_array.length){
		dir = dir_tree[dir_tree.length - 1] + music_array[music_num] + "/";

		if (window.XMLHttpRequest) {
			xmlhttp = new XMLHttpRequest();
		} else {
			// code for IE6, IE5
			xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
		}
	        xmlhttp.open("POST", "musicsearch.php?q=" + dir, true);
		xmlhttp.send();
		xmlhttp.onreadystatechange = function() {
			if (this.readyState == 4 && this.status == 200) {
				directory_list = this.responseText;
				dir_tree.push(dir);
				update_page();
			}
		}
	}
}

function exit_dir(){
	var xmlhttp = 0;
	var dir;
	
	if(dir_tree.length > 1){
		dir = dir_tree[dir_tree.length - 2];
	} else {
		if(dir_tree.length){
			dir = dir_tree[dir_tree.length - 1];
		} else {
			dir = "/";
		}
	}
	
	if (window.XMLHttpRequest) {
		xmlhttp = new XMLHttpRequest();
	} else {
		// code for IE6, IE5
		xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
	}
        xmlhttp.open("POST", "musicsearch.php?q=" + dir, true);
	xmlhttp.send();
	xmlhttp.onreadystatechange = function() {
		if (this.readyState == 4 && this.status == 200) {
			directory_list = this.responseText;
			if(dir_tree.length) dir_tree.pop();
			update_page();
		}
	}
}

function update_page(){
	var musicbox = 0;
	var parent = 0;
	var count = 0;
	var array_length = 0;
	var tmp_id = 0;
	var i = 0;
	var music_name = 0;
	var new_elem, new_text;
	
	parse_dir_list();

	musicbox = document.getElementById("musiclist");
	parent = musicbox.parentNode;
	parent.removeChild(musicbox);
	musicbox = document.createElement("div");
	musicbox.id = "musiclist";
	parent.appendChild(musicbox);
	musicbox = document.getElementById("musiclist");

	array_length = music_array.length;
	count = 0;
	if(dir_tree.length > 1){
		new_elem = document.createElement("div");
		tmp_id = "Prev_Dir";
		new_elem.id = tmp_id;
		new_elem.onclick = exit_dir;
		new_text = document.createTextNode("Previous Directory");
		new_elem.appendChild(new_text);
		musicbox.appendChild(new_elem);
	}
	while(count < array_length){
		new_elem = document.createElement("div");
		music_name = music_array[count];
		if(music_name.lastIndexOf(".") > 0){
			tmp_id = "music" + String(count);
			new_elem.id = tmp_id;
			new_elem.onclick = start_music;
			music_name = music_name.slice(0, (music_name.lastIndexOf(".")));
		} else {
			tmp_id = "music" + String(count);
			new_elem.id = tmp_id;
			new_elem.onclick = enter_dir;
			music_name = music_name;
		}
		new_text = document.createTextNode(music_name);
		new_elem.appendChild(new_text);
		musicbox.appendChild(new_elem);
		count = count + 1;
	}
}

function parse_dir_list(){
	var tmp_array = [];
	var tmp_name = 0;
	var list_rp = 0;
	var listlen = 0;
	var i = 0;

	listlen = directory_list.length;
	while(list_rp < listlen){
		// get to index, slice off before
		list_rp = directory_list.indexOf(" => ");
		if(list_rp < 0) break;
		directory_list = directory_list.slice(list_rp + 4);

		// new length... reset pointer
		listlen = directory_list.length;
		list_rp = 0;
		
		// copy first character of music name and increment pointer
		if((list_rp < listlen) && (directory_list.charAt(list_rp) != "\n"))
			tmp_name = directory_list.charAt(list_rp);
		list_rp = list_rp + 1;


		// get rest of name and add to tmp_name
		while((list_rp < listlen) && (directory_list.charAt(list_rp) != "\n")){
			tmp_name = tmp_name + directory_list.charAt(list_rp);
			list_rp = list_rp + 1;
		}
		// add name to the array
		if(tmp_name.length > 0) tmp_array.push(tmp_name);
	}
	
	music_array = tmp_array;
}

function start_music(){
	var location;
	var base = "music";
	var idname = this.id;
	var music_num = 0;
	var musiccall = "startaudio";

	idname = idname.slice(base.length);
	music_num = string_to_dec(idname);
	if(music_num <= music_array.length){
		// THIS WILL CALL FILE TO START OMX
		if(dir_tree.length > 1){
			musiccall = musiccall + dir_tree[dir_tree.length-1];
		}
		musiccall = musiccall + music_array[music_num];
		//this.innerText = musiccall;
		write_function(musiccall);
	}
}

function string_to_dec(name){
	var number = 0;
	var tmp_char;
	var dec_place = 1;
	var length = name.length;
	var ascii_convert = 48;
	
	while(length > 0){
		tmp_char = name.charCodeAt(length - 1);
		number = number + ((tmp_char - ascii_convert)*dec_place);
		dec_place = dec_place * 10;
		length = length - 1;
	}
	
	return number;
}
