var directory_list;
var playlist_list;
var dir_tree = [];
var media_array = [];
var playlist_array = ["New Playlist"];
var playlist_data = [];
var active_clients = 0;
var client_name = [];
var status_data = 0;
var top_toggle = 0;
var media_type = 0;
var selected_playlist = 0;
var current_playlist = [];
var playing_playlist = [];
var new_playlist = [];
var playlist_dropdown_open = 0;

var webpage = {
    mobile_display: 0,
    state_change: 0,
    screen_width: 0,
    screen_height: 0,
    dynamic_widths: [150, 400],
    dynamic_percents: [0.2, 0.45],
    dynamic_fonts: ["", "400%"],
    side_nav_max_width: function() {
		return this.dynamic_widths[this.mobile_display];
	},
    side_nav_percent: function() {
		return this.dynamic_percents[this.mobile_display];
	},
    side_nav_width: function() {
        if((this.screen_width*this.side_nav_percent()) > this.side_nav_max_width()){
            return this.side_nav_max_width();
        } else {
            return this.screen_width*this.side_nav_percent();
        }
    }
};

function resize(init){
    var side = document.getElementById("side_nav");
    var main = document.getElementById("media_main");
    var width = 0;
    webpage.screen_width = window.innerWidth || document.documentElement.clientWidth || document.body.clientWidth;
    webpage.screen_height = window.innerHeight || document.documentElement.clientHeight || document.body.clientHeight;
    if(webpage.screen_width < 1000){
		if(webpage.mobile_display == 0) webpage.state_change = 1;
        webpage.mobile_display = 1;
    } else {
		if(webpage.mobile_display == 1) webpage.state_change = 1;
        webpage.mobile_display = 0;
    }
    width = webpage.side_nav_width() + "px"
    side.style.width = width;
    main.style.left = width;
    set_menu_width(width);
    set_font_size();
    set_frame_type();
    organize_main();
    if(init){
        handle_side_menu(2);
        handle_top_menu(2);
    } else {
        handle_side_menu(0);
        handle_top_menu(0);
    }
}
function set_frame_type(){
    var dynamic_item = document.getElementsByClassName("mobile_display");
    var item_count;
    var item_length = dynamic_item.length;
    for(item_count=item_length;item_count>0;item_count--){
        if(webpage.mobile_display) {
            dynamic_item[item_count-1].style.display = "";
        } else {
            dynamic_item[item_count-1].style.display = "none";
        }
    }
}
function set_font_size(){
    var font_groups = document.getElementsByClassName("media_font");
    var font_count;
    var font_length = font_groups.length;
    for(font_count=font_length;font_count>0;font_count--){
		font_groups[font_count-1].style.fontSize = webpage.dynamic_fonts[webpage.mobile_display];
    }
}
function set_menu_width(size){
    var menus = document.getElementsByClassName("side_menu_data");
    var menu_count;
    var menu_length = menus.length;
    for(menu_count=menu_length;menu_count>0;menu_count--){
        menus[menu_count-1].style.width = size;
        menus[menu_count-1].style.left = "-" + webpage.side_nav_max_width() + "px";
        if(menus[menu_count-1].classList.contains("active_menu")){
            menus[menu_count-1].style.transition = "left 0s";
            menus[menu_count-1].style.left = size;
        } else {
            menus[menu_count-1].style.transition = "";
        }
    }
}
function handle_side_menu(clicked){
    var side_bar = document.getElementsByClassName("media_side_nav");
    var main = document.getElementById("media_main");
    var menu_toggle = document.getElementById("side_toggle");
    var width = webpage.side_nav_width() + "px";
    
    if(webpage.mobile_display){
		main.style.left = "0px";
        if(clicked){
            if((clicked == 2) || menu_toggle.classList.contains("active_toggle")){
                menu_toggle.classList.remove("active_toggle");
                side_bar[0].style.left = "-" + webpage.side_nav_max_width() + "px";
                if(clicked == 1){
                    clear_menus(0,0,0);
                }
            } else {
                menu_toggle.classList.add("active_toggle");
                side_bar[0].style.left = "0px";
            }
        } else {
            if(menu_toggle.classList.contains("active_toggle")){
                side_bar[0].style.left = "0px";
            } else {
                side_bar[0].style.left = "-" + webpage.side_nav_max_width() + "px";
            }
        }
    } else {
        menu_toggle.classList.remove("active_toggle");
        main.style.left = width;
        side_bar[0].style.left = "0px";
    }
}
function handle_top_menu(clicked){
    if(webpage.mobile_display){
        
    } else {
        
    }
}

function init_media(){
    resize(1);
    read_status();
    search_media();
    select_playlist();
    setInterval(read_status, 10000);
}

function set_sizes(){
    var presets = 0;
    var precount = 0;
    //presets = getElementsByClassName("pre_setup");
    //for(precount=0;precount<presets.length;precount++){
        
    //}
}

function set_media_type(type){
    if(type == media_type){
        //media_type = type;
    } else {
        media_type = type;
        dir_tree = [];
        media_array = [];
        search_media();
    }
}

function show_menu(menu_button, menu_data){
    clear_menus(menu_button, menu_data, 0);
    if(menu_button.classList.contains("active_menu")){
        menu_button.classList.remove("active_menu");
    } else {
        menu_button.classList.add("active_menu");
    }
    if(menu_data.classList.contains("active_menu")){
        menu_data.classList.remove("active_menu");
        menu_data.style.transition = "";
        menu_data.style.left = "-" + webpage.side_nav_max_width() + "px";
    } else {
        menu_data.classList.add("active_menu");
        menu_data.style.transition = "";
        menu_data.style.left = webpage.side_nav_width() + "px";
    }
}
function clear_menus(button, data, target){
    var menus = document.getElementsByClassName("active_menu");
    var menu_count;
    var menu_length = menus.length;
    for(menu_count=menu_length;menu_count>0;menu_count--){
        if((menus[menu_count-1] === button) || (menus[menu_count-1] === data)){
            // ignore what's being processed
        } else {
            menus[menu_count-1].style.transition = "";
            menus[menu_count-1].style.left = "-" + webpage.side_nav_max_width() + "px";
            menus[menu_count-1].classList.remove("active_menu");
        }
    }
    if(!button && !data && webpage.mobile_display){
        handle_side_menu(2);
    }
}

function toggle_drop(drop_box, menu_option){
    var toggle_parent = 0;
    var parent_count = 0;
    var temp_count;

    toggle_parent = menu_option.parentNode;
    for(temp_count=0;temp_count<toggle_parent.childElementCount;temp_count++){
        if(menu_option === toggle_parent.children[temp_count]){
            toggle_parent.children[temp_count].classList.add("nav_box_active");
            parent_count = temp_count;
        } else {
            toggle_parent.children[temp_count].classList.remove("nav_box_active");
        }
    }

    if(top_toggle & (1 << parent_count)){
        top_toggle = top_toggle & ~(1 << parent_count);
        drop_box.children[parent_count].classList.remove("show_nav_drop");
        toggle_parent.children[parent_count].classList.remove("nav_box_active");
        //for(parent_count=1;parent_count<nav_top.childElementCount;parent_count++){
        //    menu_option.children[parent_count].classList.remove("show_nav_drop");
        //}
    } else {
        close_top();
        top_toggle = top_toggle | (1 << parent_count);
        drop_box.children[parent_count].classList.add("show_nav_drop");
        //for(parent_count=1;parent_count<nav_top.childElementCount;parent_count++){
        //    menu_option.children[parent_count].classList.add("show_nav_drop");
        //}
    }
}

function view_main(group, page){
	var child_count = 0;
	var group_length = 0;
	
	group_length = group.children.length;
	for(child_count=0;child_count<group_length;child_count++){
		if(group.children[child_count] == page){
			group.children[child_count].style.display = "block";
		} else {
			group.children[child_count].style.display = "none";
		}
	}
}

function organize_main(){
	var group = document.getElementById("media_page");
	var child_count = 0;
	var group_length = 0;
	
	if(webpage.mobile_display){
		group_length = group.children.length;
		for(child_count=0;child_count<group_length;child_count++){
			group.children[child_count].style.width = "100%";
			group.children[child_count].style.height = "100%";
			group.children[child_count].style.top = "0px";
			group.children[child_count].style.left = "0px";
			if(webpage.state_change){
				if(child_count){
					group.children[child_count].style.display = "none";
				} else {
					group.children[child_count].style.display = "block";
				}
			}
		}
		webpage.state_change = 0;
	} else {
		group_length = group.children.length;
		for(child_count=0;child_count<group_length;child_count++){
			group.children[child_count].style.display = "block";
			if(child_count == 0){
				group.children[child_count].style.width = "50%";
				group.children[child_count].style.height = "75%";
				group.children[child_count].style.top = "0px";
				group.children[child_count].style.left = "0px";
			}
			if(child_count == 1){
				group.children[child_count].style.width = "50%";
				group.children[child_count].style.height = "75%";
				group.children[child_count].style.top = "0px";
				group.children[child_count].style.left = "50%";
			}
			if(child_count == 2){
				group.children[child_count].style.width = "100%";
				group.children[child_count].style.height = "25%";
				group.children[child_count].style.top = "75%";
				group.children[child_count].style.left = "0px";
			}
		}
	}
}

function close_top(){
    var nav_divs;
    var nav_cnt = 0;
    var clen;
    
    nav_divs = document.getElementsByClassName("show_nav_drop");
    clen = nav_divs.length;
    for(nav_cnt=0;nav_cnt<clen;nav_cnt++){
        nav_divs[0].classList.remove("show_nav_drop");
    }
    
    top_toggle = 0;
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
    var name = "";
    active_clients = 0;
    client_name = [];
    status_length = status_data.length;
    for(status_count=0;status_count<status_length;status_count++){
        if(status_data.charAt(status_count) != "\n"){
            line_data = line_data + status_data.charAt(status_count);
        } else {
            // finished reading line... process
            if(line_data.startsWith("c")){
                active_clients = active_clients + 1;
                name = line_data.substr((line_data.indexOf(':')+1),line_data.length);
                if(name.length > 0){
                    client_name.push(name);
                } else {
                    name = "NoName" + active_clients;
                    client_name.push(name);
                }
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
    client_table = document.getElementById("client_list_menu");
    clength = client_table.children.length;
    for(ccount=0;ccount<(clength);ccount++){
        client_table.removeChild(client_table.children[clength-ccount-1]);
    }
    for(ccount=0;ccount<active_clients;ccount++){
        new_client = document.createElement("div");
        new_client.id = "client" + ccount;
        new_client.classList.add("nav_drop_content");
        new_client.classList.add("menu_data");
        new_client.onclick = select_client;
        new_client.innerText = client_name[ccount];
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

function search_media(){
	var xmlhttp = 0;
	var dir = "/";
	var error_html;
	if (window.XMLHttpRequest) {
		xmlhttp = new XMLHttpRequest();
	} else {
		// code for IE6, IE5
		xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
	}
    if(media_type == 0){
        xmlhttp.open("POST", "moviesearch.php?q=" + dir, true);
    } else {
        xmlhttp.open("POST", "musicsearch.php?q=" + dir, true);
    }
	xmlhttp.send();
	xmlhttp.onreadystatechange = function() {
		if (this.readyState == 4 && this.status == 200) {
			directory_list = this.responseText;
			if(directory_list == 0) {
				error_html = document.getElementById("medialist");
                if(media_type == 0){
				    error_html.innerHTML = "Could Not Open Movie Dir File";
                } else {
                    error_html.innerHTML = "Could Not Open Music Dir File";
                }
			} else {
				if(directory_list == 1) {
					error_html = document.getElementById("medialist");
                    if(media_type == 0){
					    error_html.innerHTML = "No Movie Directory Set";
                    } else {
                        error_html.innerHTML = "No Music Directory Set";
                    }
				} else {
					dir_tree.push(dir);
					update_media_list();
				}
			}
		}
	}
}

function enter_dir(){
    if(media_type == 0){
        var base = "movie";
    } else {
        var base = "music";
    }
	var idname = this.id;
	var media_num = 0;
	var xmlhttp = 0;
	var dir;
	
	idname = idname.slice(base.length);
	media_num = string_to_dec(idname);

	if(media_num <= media_array.length){
		dir = dir_tree[dir_tree.length - 1] + media_array[media_num] + "/";

		if (window.XMLHttpRequest) {
			xmlhttp = new XMLHttpRequest();
		} else {
			// code for IE6, IE5
			xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
		}
        if(media_type == 0){
            xmlhttp.open("POST", "moviesearch.php?q=" + dir, true);
        } else {
            xmlhttp.open("POST", "musicsearch.php?q=" + dir, true);
        }
		xmlhttp.send();
		xmlhttp.onreadystatechange = function() {
			if (this.readyState == 4 && this.status == 200) {
				directory_list = this.responseText;
				dir_tree.push(dir);
				update_media_list();
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
    if(media_type == 0){
        xmlhttp.open("POST", "moviesearch.php?q=" + dir, true);
    } else {
        xmlhttp.open("POST", "musicsearch.php?q=" + dir, true);
    }
	xmlhttp.send();
	xmlhttp.onreadystatechange = function() {
		if (this.readyState == 4 && this.status == 200) {
			directory_list = this.responseText;
			if(dir_tree.length) dir_tree.pop();
			update_media_list();
		}
	}
}

function update_media_list(){
	var mediabox = 0;
	var media_length = 0;
	var parent = 0;
	var count = 0;
	var array_length = 0;
	var tmp_id = 0;
	var i = 0;
	var media_name = 0;
	var new_elem, new_text;
    var saved_classes = 0;
    var class_count = 0;
	
	parse_dir_list();

	mediabox = document.getElementById("medialist");
	media_length = mediabox.children.length;
	for(count=media_length;count>0;count--){
		mediabox.removeChild(mediabox.children[count-1]);
	}
   
	array_length = media_array.length;
	count = 0;
	
	// make search bar
	new_elem = document.createElement("INPUT");
	tmp_id = "media_search_bar";
	new_elem.id = tmp_id;
	new_elem.classList.add("dropbtn");
	new_elem.style.fontSize = webpage.dynamic_fonts[webpage.mobile_display];
	new_elem.style.width = "100%";
	new_elem.type = "search";
	new_elem.oninput = update_media_search;
	new_elem.placeholder = "Search";
	mediabox.appendChild(new_elem);
	
	if(dir_tree.length > 1){
		new_elem = document.createElement("div");
		tmp_id = "Prev_Dir";
		new_elem.id = tmp_id;
		new_elem.onclick = exit_dir;
		new_elem.classList.add("dropbtn");
		new_elem.style.fontSize = webpage.dynamic_fonts[webpage.mobile_display];
		new_text = document.createTextNode("Previous Directory");
		new_elem.appendChild(new_text);
		mediabox.appendChild(new_elem);
	}
	while(count < array_length){
		new_elem = document.createElement("div");
		media_name = media_array[count];
		if((media_name.lastIndexOf(".") > 0) && ((media_name.length - media_name.lastIndexOf(".")) <= 4)){
			tmp_id = "media" + String(count);
			new_elem.id = tmp_id;
			new_elem.onclick = open_item_options;
            new_elem.classList.add("dropbtn");
            new_elem.style.fontSize = webpage.dynamic_fonts[webpage.mobile_display];
			media_name = media_name.slice(0, (media_name.lastIndexOf(".")));
		} else {
			tmp_id = "media" + String(count);
			new_elem.id = tmp_id;
			new_elem.onclick = open_folder_options;
            new_elem.classList.add("dropbtn");
            new_elem.style.fontSize = webpage.dynamic_fonts[webpage.mobile_display];
			media_name = media_name;
		}
		new_text = document.createTextNode(media_name);
		new_elem.appendChild(new_text);
		mediabox.appendChild(new_elem);
		count = count + 1;
	}
}

function update_media_search(){
	var mediabox = 0;
	var media_length = 0;
	var search_text = "";
	var count = 0;
	var start = 1;
	
	mediabox = document.getElementById("medialist");
	media_length = mediabox.children.length;
	search_text = document.getElementById("media_search_bar").value;
	
	if(media_length > 1){
		if(mediabox.children[1].id == "Prev_Dir") start++;
	}
	if(search_text.length > 0){
		for(count=start;count<media_length;count++){
			if(mediabox.children[count].innerText.toLowerCase().includes(search_text.toLowerCase())){
				mediabox.children[count].style.display = "";
			} else {
				mediabox.children[count].style.display = "none";
			}
		}
	} else {
		for(count=start;count<media_length;count++){
			mediabox.children[count].style.display = "";
		}
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
		
		// copy first character of media name and increment pointer
		if((list_rp < listlen) && (directory_list.charAt(list_rp) != "\n"))
			tmp_name = directory_list.charAt(list_rp);
		list_rp = list_rp + 1;

		// get rest of name and add to tmp_name
		while((list_rp < listlen) && (directory_list.charAt(list_rp) != "\n")){
			tmp_name = tmp_name + directory_list.charAt(list_rp);
			list_rp = list_rp + 1;
		}
		// add name to the array
		if(tmp_name.length > 0){
            if((tmp_name === ".") || (tmp_name === "..")){
                // . and .. aren't valid... from dir search
            } else {
                tmp_array.push(tmp_name);
            }
        }
	}
	
	media_array = tmp_array;
}

function start_media(){
	var location;
    if(media_type == 0){
        var base = "movie";
        var mediacall = "mc01";
    } else {
        var base = "music";
        var mediacall = "mc11";
    }
	var idname = this.id;
	var media_num = 0;
    mediacall = mediacall + selected_clients;

	idname = idname.slice(base.length);
	media_num = string_to_dec(idname);
	if(media_num <= media_array.length){
		// THIS WILL CALL FILE TO START OMX
		if(dir_tree.length > 1){
			mediacall = mediacall + dir_tree[dir_tree.length-1];
		}
		mediacall = mediacall + media_array[media_num];
		//this.innerText = mediacall;
		write_function(mediacall);
	}
}

function update_media(option){
    var updatecall = 0;
    if(media_type == 0){
        updatecall = "mc03" + selected_clients + option;
    } else {
        updatecall = "mc13" + selected_clients + option;
    }
    write_function(updatecall);
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
    var new_elem = 0;
    var just_clear = 0;
    var dropdowns;
    dropdowns = document.getElementsByClassName("dropdown-content");
    if(dropdowns.length > 1){
        // problem -- too many, just clear all and set this
    } else {
        if((dropdowns.length == 1) && (this === dropdowns[0].parentNode)){
            just_clear = 1;
        }
    }
    cleardropdowns();
    if(!just_clear){
        new_elem = document.createElement("div");
        new_elem.id = "item_dropdown";
        new_elem.classList.add("dropdown-content");
        new_div = document.createElement("div");
        new_div.id = this.id;
        new_div.onclick = start_media;
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
}

function open_folder_options(){
    var new_div = 0;
    var new_elem = 0;
    var just_clear = 0;
    var dropdowns;
    dropdowns = document.getElementsByClassName("dropdown-content");
    if(dropdowns.length > 1){
        // problem -- too many, just clear all and set this
    } else {
        if((dropdowns.length == 1) && (this === dropdowns[0].parentNode)){
            just_clear = 1;
        }
    }
    cleardropdowns();
    if(!just_clear){
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
        new_div.onclick = start_media;
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
}

window.onclick = function(event) {
    var tests;
    if(!((event.target.classList.contains("side_nav_button")) || 
        (event.target.classList.contains("media_side_nav_buttons")) || 
        (event.target.classList.contains("side_menu_data")) || 
        (event.target.classList.contains("menu_data")) || 
        (event.target.classList.contains("media_side_nav_container")) ||
        (event.target.id == "side_toggle")
        )){//|| event.target.classList.contains("active_menu"))){
        clear_menus(0,0, event.target);
    }
    if(!event.target.matches('.dropbtn')){
        cleardropdowns();
    }
    if((!event.target.matches('.nav_box')) && (!event.target.matches('.nav_drop_content')) && (!event.target.matches('.show_nav_drop'))){
        close_top();
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

function update_playlist_list(){
	var mediabox = 0;
	var media_length = 0;
	var parent = 0;
	var array_length = 0;
	var tmp_id = 0;
	var i = 0;
	var playlist_name = 0;
	var new_elem, new_text;
    var saved_classes = 0;
    var class_count = 0;
	var page = document.getElementById("playlist_dropdown");
	var count;
	var child_count;
	
	child_count = page.children.length;
	for(count=child_count;count>0;count--){
		page.removeChild(page.children[count-1]);
	}
	
	parse_playlist_list();
	show_playlists();
}

function display_playlist(data){
	var count;
	var array_length;
	var child_count;
	var page = document.getElementById("playlist_data");
	
	child_count = page.children.length;
	for(count=child_count;count>0;count--){
		page.removeChild(page.children[count-1]);
	}
	
	parse_playlist_data(data);
	
	/*new_elem = document.createElement("div");
	tmp_id = "ListView";
	new_elem.id = tmp_id;
	new_elem.onclick = search_playlists;
	new_elem.classList.add("dropbtn");
	new_elem.style.fontSize = webpage.dynamic_fonts[webpage.mobile_display];
	new_text = document.createTextNode("View Playlists");
	new_elem.appendChild(new_text);
	page.appendChild(new_elem);
	*/
	array_length = playlist_data.length;
	count = 0;
	while(count < array_length){
		new_elem = document.createElement("div");
		playlist_name = playlist_data[count];
		tmp_id = "item" + String(count);
		new_elem.id = tmp_id;
		new_elem.onclick = open_item_options;
		new_elem.classList.add("dropbtn");
		new_elem.style.fontSize = webpage.dynamic_fonts[webpage.mobile_display];
		if((playlist_name.lastIndexOf(".") == playlist_name.length) || ((playlist_name.length - playlist_name.lastIndexOf(".")) > 3)){
			playlist_name = playlist_name;
		} else {
			playlist_name = playlist_name.slice(0, (playlist_name.lastIndexOf(".")));
		}
		new_text = document.createTextNode(playlist_name);
		new_elem.appendChild(new_text);
		page.appendChild(new_elem);
		count = count + 1;
	}
}

function search_playlists(){
	var xmlhttp = 0;
	var error_html;
	if (window.XMLHttpRequest) {
		xmlhttp = new XMLHttpRequest();
	} else {
		// code for IE6, IE5
		xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
	}
    xmlhttp.open("POST", "playlistsearch.php?q=" + "", true);
	xmlhttp.send();
	xmlhttp.onreadystatechange = function() {
		if (this.readyState == 4 && this.status == 200) {
			playlist_list = this.responseText;
			if(playlist_list == 0) {
				error_html = document.getElementById("playlists");
				error_html.innerHTML = "Could Not Open Playlist Dir File";
			} else {
				if(playlist_list == 1) {
					error_html = document.getElementById("playlists");
					error_html.innerHTML = "No Playlist Directory Set";
				} else {
					update_playlist_list();
				}
			}
		}
	}
}

function open_playlist(playlist_name){
	var xmlhttp = 0;
	var error_html;
	var name;
	if (window.XMLHttpRequest) {
		xmlhttp = new XMLHttpRequest();
	} else {
		// code for IE6, IE5
		xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
	}
	name = playlist_name;
    xmlhttp.open("POST", "readplaylist.php?q=" + name, true);
	xmlhttp.send();
	xmlhttp.onreadystatechange = function() {
		if (this.readyState == 4 && this.status == 200) {
			if(playlist_list == 0) {
				error_html = document.getElementById("playlists");
				error_html.innerHTML = "Could Not Open Playlist File";
			} else {
				if(playlist_list == 1) {
					error_html = document.getElementById("playlists");
					error_html.innerHTML = "No Playlist Directory Set";
				} else {
					display_playlist(this.responseText);
				}
			}
		}
	}
}

function parse_playlist_list(){
	var tmp_array = [];
	var tmp_name = 0;
	var list_rp = 0;
	var listlen = 0;
	var i = 0;

	listlen = playlist_list.length;
	tmp_array.push("New Playlist");
	while(list_rp < listlen){
		// get to index, slice off before
		list_rp = playlist_list.indexOf(" => ");
		if(list_rp < 0) break;
		playlist_list = playlist_list.slice(list_rp + 4);

		// new length... reset pointer
		listlen = playlist_list.length;
		list_rp = 0;
		
		// copy first character of media name and increment pointer
		if((list_rp < listlen) && (playlist_list.charAt(list_rp) != "\n"))
			tmp_name = playlist_list.charAt(list_rp);
		list_rp = list_rp + 1;

		// get rest of name and add to tmp_name
		while((list_rp < listlen) && (playlist_list.charAt(list_rp) != "\n")){
			tmp_name = tmp_name + playlist_list.charAt(list_rp);
			list_rp = list_rp + 1;
		}
		// add name to the array
		if(tmp_name.length > 0){
            if((tmp_name === ".") || (tmp_name === "..")){
                // . and .. aren't valid... from dir search
            } else {
                tmp_array.push(tmp_name);
            }
        }
	}
	
	playlist_array = tmp_array;
}


function parse_playlist_data(data){
	var tmp_array = [];
	var tmp_name = "";
	var list_rp = 0;
	var listlen = 0;
	var i = 0;

	listlen = data.length;
	while(list_rp < listlen){
		// skip past new line
		list_rp = list_rp + 1;
		// copy first character of media name and increment pointer
		if((list_rp < listlen) && (data.charAt(list_rp) != "\n"))
			tmp_name = data.charAt(list_rp);
		list_rp = list_rp + 1;

		// get rest of name and add to tmp_name
		while((list_rp < listlen) && (data.charAt(list_rp) != "\n")){
			tmp_name = tmp_name + data.charAt(list_rp);
			list_rp = list_rp + 1;
		}
		// add name to the array
		if(tmp_name.length > 0){
			tmp_array.push(tmp_name);
        }
	}
	
	playlist_data = tmp_array;
}

function show_playlists(){
	var playlist_dropdown = document.getElementById("playlist_dropdown");
	var child_count;
	var child_length;
	child_length = playlist_dropdown.children.length;
	for(child_count=child_length;child_count>0;child_count--){
		playlist_dropdown.removeChild(playlist_dropdown.children[child_count-1]);
	}
	
	array_length = playlist_array.length;
	count = 0;
	
	while(count < array_length){
		new_elem = document.createElement("div");
		playlist_name = playlist_array[count];
		tmp_id = "playlist" + String(count);
		new_elem.id = tmp_id;
		new_elem.onclick = select_playlist;
		new_elem.classList.add("dropbtn");
		new_elem.style.fontSize = webpage.dynamic_fonts[webpage.mobile_display];
		if((playlist_name.lastIndexOf(".") == playlist_name.length) || ((playlist_name.length - playlist_name.lastIndexOf(".")) > 3)){
			playlist_name = playlist_name;
		} else {
			playlist_name = playlist_name.slice(0, (playlist_name.lastIndexOf(".")));
		}
		new_text = document.createTextNode(playlist_name);
		new_elem.appendChild(new_text);
		playlist_dropdown.appendChild(new_elem);
		count = count + 1;
	}
	playlist_dropdown_open = 1;
}

function select_playlist(){
	var dropdown = document.getElementById("playlist_dropdown");
	var selected = this;
	var playlist_name;
	var new_elem;
	var tmp_id;
	
	child_length = dropdown.children.length;
	for(child_count=child_length;child_count>0;child_count--){
		if(playlist_dropdown.children[child_count-1] === this){
			selected_playlist = child_count-1;
		}
		playlist_dropdown.removeChild(playlist_dropdown.children[child_count-1]);
	}
	
	new_elem = document.createElement("div");
	playlist_name = playlist_array[selected_playlist];
	tmp_id = "playlist" + String(selected_playlist);
	new_elem.id = tmp_id;
	new_elem.onclick = search_playlists;
	new_elem.classList.add("dropbtn");
	new_elem.style.fontSize = webpage.dynamic_fonts[webpage.mobile_display];
	if((playlist_name.lastIndexOf(".") == playlist_name.length) || ((playlist_name.length - playlist_name.lastIndexOf(".")) > 3)){
		playlist_name = playlist_name;
	} else {
		playlist_name = playlist_name.slice(0, (playlist_name.lastIndexOf(".")));
	}
	new_text = document.createTextNode(playlist_name);
	new_elem.appendChild(new_text);
	dropdown.appendChild(new_elem);
	
	if(selected_playlist > 0){
		open_playlist(selected.innerHTML);
	}
}
