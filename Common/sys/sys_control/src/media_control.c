
#include "sys_control.h"
#include "sys_config.h"
#include "sys_functions.h"
#include "media_control.h"

unsigned int active_movie_count = 0;
unsigned int movie_clients[MOVIE_MAX];
unsigned int movie_clients_ready[MOVIE_MAX];

unsigned int active_music_count = 0;
unsigned int music_clients[MUSIC_MAX];
unsigned int music_clients_ready[MUSIC_MAX];

void init_media(){
    active_movie_count = 0;
    active_music_count = 0;
}

void start_listener(char type, unsigned int in_address[4]){
    char listening = 0;
    if(system("pgrep pwomxplayer") > 0) listening = 1;
    else listening = 0;
    if(listening){
        // can't listen - already listening
    } else {
        listening = 1;
        char start_string[MAX_FUNCTION_STRING];
        // start listener
        sprintf(start_string, "pwomxplayer -o hdmi https://%u.%u.%u.%u:8080/ms.flv?buffer_size 12000000B&", in_address[0], in_address[1], in_address[2], in_address[3]);
        system(start_string);
        if(type == 0) send_to("mc02", in_address);
        if(type == 1) send_to("mc12", in_address);
    }
}

char movie_control(char stream_select, char input_option, char* input_src, unsigned int out_count, unsigned int out_address[][4]){
    if(input_option == 1){ // start stream
        if(active_movie_count < MOVIE_MAX){
            #ifdef IS_SERVER
            start_movie(stream_select, input_option, input_src, out_count, out_address);
            #endif
            #ifdef IS_CLIENT
            start_listener(0,server_ip);
            #endif
        } else {
            // too many movies going... don't start
            return 0;
        }
    } else {
        if(active_movie_count > 0){
            update_movie(stream_select, input_option, input_src);
        } else {
            // can't do anything to streams... none going
            return 0;
        }
    }
    return 0;
}

char start_movie(char stream_select, char input_option, char* input_src, unsigned int out_count, unsigned int out_address[][4]){
    char send_count = 0;
    movie_clients_ready[stream_select] = 0;
    movie_clients[stream_select] = out_count;
    active_movie_count = active_movie_count + 1;
    //start vlc player and connect to pipe
    //send update to pause player
    //send start stream to all outputs
    for(send_count=0;send_count<out_count;send_count++){
        send_to("mc01", out_address[send_count]);
    }
}

char update_movie(char stream_select, char input_option, char* input_src){
    if(input_option == 0){
        // stop selected stream
    }
    if(input_option == 2){
        movie_clients_ready[stream_select] = movie_clients_ready[stream_select] + 1;
        if(movie_clients_ready[stream_select] >= movie_clients[stream_select]){
            // play stream - or... essentially unpause to start...
        }
    }
    if(input_option == 3){
        
    }
}

char music_control(char stream_select, char input_option, char* input_src, unsigned int out_count, unsigned int out_address[][4]){
    if(input_option == 1){ // start stream
        if(active_music_count < MUSIC_MAX){
            start_music(stream_select, input_option, input_src, out_count, out_address);
        } else {
            // too many movies going... don't start
            return 0;
        }
    } else {
        if(active_music_count > 0){
            update_music(stream_select, input_option, input_src);
        } else {
            // can't do anything to streams... none going
            return 0;
        }
    }
    return 0;
}

char start_music(char stream_select, char input_option, char* input_src, unsigned int out_count, unsigned int out_address[][4]){
    music_clients_ready[stream_select] = 0;
    music_clients[stream_select] = out_count;
    active_music_count = active_music_count + 1;
    //start vlc player and connect to pipe
    //send update to pause player
    //send start stream to all outputs
}

char update_music(char stream_select, char input_option, char* input_src){
    if(input_option == 0){
        // stop selected stream
    }
    if(input_option == 2){
        music_clients_ready[stream_select] = music_clients_ready[stream_select] + 1;
        if(music_clients_ready[stream_select] >= music_clients[stream_select]){
            // play stream - or... essentially unpause to start...
        }
    }
    if(input_option == 3){
        //other option like pause...
    }
}


