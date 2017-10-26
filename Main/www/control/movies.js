var directory_list;
var dir_tree = [];
var movie_array = [];

function search_movies(){
	var xmlhttp = 0;
	var dir = "/";
	if (window.XMLHttpRequest) {
		xmlhttp = new XMLHttpRequest();
	} else {
		// code for IE6, IE5
		xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
	}
        xmlhttp.open("POST", "moviesearch.php?q=" + dir, true);
	xmlhttp.send();
	xmlhttp.onreadystatechange = function() {
		if (this.readyState == 4 && this.status == 200) {
			directory_list = this.responseText;
			dir_tree.push(dir);
			update_page();
		}
	}
}

function enter_dir(){
	var base = "movie";
	var idname = this.id;
	var movie_num = 0;
	var xmlhttp = 0;
	var dir;
	
	idname = idname.slice(base.length);
	movie_num = string_to_dec(idname);

	if(movie_num <= movie_array.length){
		dir = dir_tree[dir_tree.length - 1] + movie_array[movie_num] + "/";

		if (window.XMLHttpRequest) {
			xmlhttp = new XMLHttpRequest();
		} else {
			// code for IE6, IE5
			xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
		}
	        xmlhttp.open("POST", "moviesearch.php?q=" + dir, true);
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
        xmlhttp.open("POST", "moviesearch.php?q=" + dir, true);
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
	var moviebox = 0;
	var parent = 0;
	var count = 0;
	var array_length = 0;
	var tmp_id = 0;
	var i = 0;
	var movie_name = 0;
	var new_elem, new_text;
	
	parse_dir_list();

	moviebox = document.getElementById("movielist");
	parent = moviebox.parentNode;
	parent.removeChild(moviebox);
	moviebox = document.createElement("div");
	moviebox.id = "movielist";
	parent.appendChild(moviebox);
	moviebox = document.getElementById("movielist");

	array_length = movie_array.length;
	count = 0;
	if(dir_tree.length > 1){
		new_elem = document.createElement("div");
		tmp_id = "Prev_Dir";
		new_elem.id = tmp_id;
		new_elem.onclick = exit_dir;
		new_text = document.createTextNode("Previous Directory");
		new_elem.appendChild(new_text);
		moviebox.appendChild(new_elem);
	}
	while(count < array_length){
		new_elem = document.createElement("div");
		movie_name = movie_array[count];
		if(movie_name.lastIndexOf(".") > 0){ // FIX THIS CHECK! FAILS ON FOLDER NAME WITH DOT
			tmp_id = "movie" + String(count);
			new_elem.id = tmp_id;
			new_elem.onclick = start_movie;
			movie_name = movie_name.slice(0, (movie_name.lastIndexOf(".")));
		} else {
			tmp_id = "movie" + String(count);
			new_elem.id = tmp_id;
			new_elem.onclick = enter_dir;
			movie_name = movie_name;
		}
		new_text = document.createTextNode(movie_name);
		new_elem.appendChild(new_text);
		moviebox.appendChild(new_elem);
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
		
		// copy first character of movie name and increment pointer
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
	
	movie_array = tmp_array;
}

function start_movie(){
	var location;
	var base = "movie";
	var idname = this.id;
	var movie_num = 0;
	var moviecall = "startvideo";

	idname = idname.slice(base.length);
	movie_num = string_to_dec(idname);
	if(movie_num <= movie_array.length){
		// THIS WILL CALL FILE TO START OMX
		if(dir_tree.length > 1){
			moviecall = moviecall + dir_tree[dir_tree.length-1];
		}
		moviecall = moviecall + movie_array[movie_num];
		//this.innerText = moviecall;
		write_function(moviecall);
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
