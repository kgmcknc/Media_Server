var directory_list;
var dir_tree = [];
var movie_array = [];
var active_clients = 0;
var status_data = 0;

function init_movies(){
    read_status();
    search_movies();
    setInterval(read_status, 20000);
}

function update_status(){
    parse_status();
    update_clients();
}

function read_status(){
    var xmlhttp = 0;
	var dir = "/";
	var error_html;
	if (window.XMLHttpRequest) {
		xmlhttp = new XMLHttpRequest();
	} else {
		// code for IE6, IE5
		xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
	}
    xmlhttp.open("POST", "read_status.php?q=" + dir, true);
	xmlhttp.send();
	xmlhttp.onreadystatechange = function() {
		if (this.readyState == 4 && this.status == 200) {
			status_data = this.responseText;
            update_status();
		}
	}
}

function parse_status(){
    var status_count = 0;
    var status_length = 0;
    var line_data = "";
    active_clients = 0;
    status_length = status_data.length;
    for(status_count=0;status_count<status_length;status_count++){
        if(status_data.charAt(status_count) != "\n"){
            line_data = line_data + status_data.charAt(status_count);
        } else {
            // finished reading line... process
            if(line_data.startsWith("c")){
                active_clients = active_clients + 1;
            } else {
                // server or other
            }
            line_data = "";
        }
    }
}

function update_clients(){
    var client_table;
    var ccount = 0;
    var new_client = 0;
    var clength = 0;
    client_table = document.getElementById("clients");
    clength = client_table.children.length;
    for(ccount=0;ccount<clength;ccount++){
        client_table.removeChild(client_table.children[clength-ccount-1]);
    }
    for(ccount=0;ccount<active_clients;ccount++){
        new_client = document.createElement("span");
        new_client.id = "client" + ccount;
        new_client.onclick = select_client;
        new_client.innerText = "Client " + ccount;
        if(selected_clients & (1 << ccount)){
            new_client.style.backgroundColor = "#00A000";
            new_client.style.opacity = 0.4;
        } else {
            new_client.style.backgroundColor = "";
            new_client.style.opacity = "";
        }
        client_table.appendChild(new_client);
    }
}

var selected_clients = 0;
function select_client(){
    var this_number = 0;
    var c_count = 0;
    var parent = 0;
    parent = this.parentNode;
    c_count = parent.childElementCount;
    while(this_number < c_count){
        if(this === parent.children[this_number]){
            break;
        } else {
            this_number = this_number + 1;
        }
    }
    if(selected_clients & (1 << this_number)){
        this.style.backgroundColor = "";
        this.style.opacity = "";
        selected_clients = selected_clients & ~(1 << this_number);
    } else {
        this.style.backgroundColor = "#00A000";
        this.style.opacity = 0.4;
        selected_clients = selected_clients | (1 << this_number);
    }
}

function search_movies(){
	var xmlhttp = 0;
	var dir = "/";
	var error_html;
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
			if(directory_list == 0) {
				error_html = document.getElementById("movielist");
				error_html.innerHTML = "Could Not Open Movie Dir File";
			} else {
				if(directory_list == 1) {
					error_html = document.getElementById("movielist");
					error_html.innerHTML = "No Movie Directory Set";
				} else {
					dir_tree.push(dir);
					update_page();
				}
			}
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
			new_elem.onclick = open_item_options;
            new_elem.classList.add("dropbtn");
			movie_name = movie_name.slice(0, (movie_name.lastIndexOf(".")));
		} else {
			tmp_id = "movie" + String(count);
			new_elem.id = tmp_id;
			new_elem.onclick = open_folder_options;
            new_elem.classList.add("dropbtn");
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
	var moviecall = "mc01";

	idname = idname.slice(base.length);
	movie_num = string_to_dec(idname);
	if(movie_num <= movie_array.length){
		// THIS WILL CALL FILE TO START OMX
		if(dir_tree.length > 1){
			moviecall = moviecall + dir_tree[dir_tree.length-1];
		}
		moviecall = moviecall + selected_clients + movie_array[movie_num];
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

function open_item_options(){
    var new_div = 0;
    cleardropdowns();
    new_elem = document.createElement("div");
    new_elem.id = "item_dropdown";
    new_elem.classList.add("dropdown-content");
    new_div = document.createElement("div");
    new_div.id = this.id;
    new_div.onclick = start_movie;
    new_text = document.createTextNode("Play");
    new_div.appendChild(new_text);
    new_elem.appendChild(new_div);
    new_div = document.createElement("div");
    new_div.id = this.id;
    new_div.onclick = add_to_list_end;
    new_text = document.createTextNode("Add To Current Playlist");
    new_div.appendChild(new_text);
    new_elem.appendChild(new_div);
    this.appendChild(new_elem);
}

function open_folder_options(){
    var new_div = 0;
    cleardropdowns();
    new_elem = document.createElement("div");
    new_elem.id = "folder_dropdown";
    new_elem.classList.add("dropdown-content");
    new_div = document.createElement("div");
    new_div.id = this.id;
    new_div.onclick = enter_dir;
    new_text = document.createTextNode("Enter Folder");
    new_div.appendChild(new_text);
    new_elem.appendChild(new_div);
    new_div = document.createElement("div");
    new_div.id = this.id;
    new_div.onclick = start_movie;
    new_text = document.createTextNode("Play Folder");
    new_div.appendChild(new_text);
    new_elem.appendChild(new_div);
    new_div = document.createElement("div");
    new_div.id = this.id;
    new_div.onclick = add_to_list_end;
    new_text = document.createTextNode("Add Folder To Current Playlist");
    new_div.appendChild(new_text);
    new_elem.appendChild(new_div);
    this.appendChild(new_elem);
}

window.onclick = function(event) {
    if(!event.target.matches('.dropbtn')){
        cleardropdowns();
    }
}

function cleardropdowns(){
    var dropdowns = document.getElementsByClassName("dropdown-content");
    var parent = 0;
    var i;
    for(i=0;i<dropdowns.length;i++){
        var openDropdown = dropdowns[i];
        parent = openDropdown.parentNode;
        parent.removeChild(openDropdown);
    }
}

function add_to_list_end(){
   alert("not implemented");
}
