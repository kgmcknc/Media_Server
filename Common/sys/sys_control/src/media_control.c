
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
    char ps_id = 0;
    printf("In Media Listener Function\n");
    ps_id = system("pgrep \"pwomxplayer\"");
    if(ps_id > 0) listening = 1;
    else listening = 0;
    if(listening){
        // can't listen - already listening
        printf("Already Listening to a stream!!\n");
    } else {
        listening = 1;
        char start_string[MAX_FUNCTION_STRING];
        // start listener
        if(type == 0){
            // video
            sprintf(start_string, "pwomxplayer -o hdmi http://%u.%u.%u.%u:8080/ms0.mkv?buffer_size 12000000B&", in_address[0], in_address[1], in_address[2], in_address[3]);
        }
        if(type == 1){
            // audio
            sprintf(start_string, "pwomxplayer -o hdmi http://%u.%u.%u.%u:8080/ms0.mkv?buffer_size 12000000B&", in_address[0], in_address[1], in_address[2], in_address[3]);
        }
        system(start_string);
        if(type == 0) send(client_sockets[0], "1%mc02%", sizeof("1%mc02%"), 0);
        if(type == 1) send(client_sockets[0], "1%mc12%", sizeof("1%mc02%"), 0);
    }
}

char movie_control(char stream_select, char input_option, char* input_src, unsigned int out_clients, unsigned int out_address[][4], char client){
    char movie_text[MAX_STRING] = {0};
    char ps_id[8] = {0};
    sprintf(movie_text, "\'/home/kyle/linux-main-share/MovieHD/%s\'", input_src);
    printf("Movie inputs: %d, %d, %s, %u\n", stream_select, input_option, input_src, out_clients);
    grep_fp = popen("pgrep \"vlc\"", "r");
    if(grep_fp == NULL){
        printf("Failed to grep vlc...\n");
        return -1;
    } else {
        fgets(ps_id, sizeof(ps_id)-1, grep_fp);
        printf("\n\n----- Got %d for ps_id -----\n\n", ps_id[0]);
    }
    pclose(grep_fp);
    if(ps_id[0] > 0) active_movie_count = 1;
    else active_movie_count = 0;
    if(input_option == 1){ // start stream
        printf("Start Stream Option\n");
        printf("Active: %d\n", active_movie_count);
        if(active_movie_count == 0){
            #ifdef IS_SERVER
            start_movie(stream_select, input_option, movie_text, out_clients, out_address);
            #endif
            #ifdef IS_CLIENT
            start_listener(0, ms_ip);
            #endif
        } else {
            // too many movies going... don't start
            return 0;
        }
    } else {
        printf("Active: %d\n", active_movie_count);
        if(active_movie_count){
            printf("Update Stream Option\n");
            update_movie(stream_select, input_option, input_src, out_clients, client);
        } else {
            // can't do anything to streams... none going
            printf("Unknown Option\n");
            return 0;
        }
    }
    return 0;
}

char start_movie(char stream_select, char input_option, char* input_src, unsigned int out_clients, unsigned int out_address[][4]){
    char send_count = 0;
    char stream_string[MAX_STRING] = {0};
    movie_clients_ready[stream_select] = 0;
    movie_clients[stream_select] = out_clients;
    //active_movie_count = active_movie_count + 1;
    sprintf(stream_string, "su - %s -c \"cvlc -I rc --rc-host %u.%u.%u.%u:%u --extraintf=http --http-password=ms %s --sout \'#standard{access=http,dst=:8080/ms0.mkv}\'&\"", username, ms_ip[0], ms_ip[1], ms_ip[2], ms_ip[3], (ms_port+1), input_src);
    printf("Starting: %s\n", stream_string);
    system(stream_string);
    sleep(2);
    send_media("pause", ms_ip);
    //send start stream to all outputs
    for(send_count=0;send_count<active_clients;send_count++){
        if(out_clients & (1<<send_count)) send(client_sockets[send_count], "1%mc01%", sizeof("1%mc01%"), 0);
    }
}

char update_movie(char stream_select, char input_option, char* input_src, char out_clients, char client){
    unsigned int media_option = 0;
    char tmp_count = 0;
    if(input_option == 0){
        // stop selected stream
    }
    if(input_option == 2){
        movie_clients_ready[stream_select] = movie_clients_ready[stream_select] | (1 << client);
        if(movie_clients_ready[stream_select] == movie_clients[stream_select]){
            send_media("play", ms_ip);
        }
    }
    if(input_option == 3){
        sscanf(input_src, "%u", &media_option);
        if(media_option == 0){
            #ifdef IS_SERVER
                for(tmp_count=0;tmp_count<active_clients;tmp_count++){
                    if(out_clients & (1<<tmp_count)) send(client_sockets[tmp_count], "1%mc0300%", sizeof("1%mc0300%"), 0);
                }
            #endif
            #ifdef IS_CLIENT
                system("echo \"on 0\" | cec-client -s&");
            #endif
        }
        if(media_option == 1){
            #ifdef IS_SERVER
                for(tmp_count=0;tmp_count<active_clients;tmp_count++){
                    if(out_clients & (1<<tmp_count)) send(client_sockets[tmp_count], "1%mc0301%", sizeof("1%mc0301%"), 0);
                }
            #endif
            #ifdef IS_CLIENT
                system("echo \"standby 0\" | cec-client -s&");
            #endif
        }
        if(media_option == 2){
            #ifdef IS_SERVER
                for(tmp_count=0;tmp_count<active_clients;tmp_count++){
                    if(out_clients & (1<<tmp_count)) send(client_sockets[tmp_count], "1%mc0302%", sizeof("1%mc0302%"), 0);
                }
            #endif
            #ifdef IS_CLIENT
                system("echo \"as\" | cec-client -s&");
            #endif
        }
        if(media_option == 3){
            #ifdef IS_SERVER
                send_media("stop", ms_ip);
                sleep(1);
                send_media("shutdown", ms_ip);
                //active_movie_count = active_movie_count - 1;
            #endif
        }
        if(media_option == 4){
            #ifdef IS_SERVER
                send_media("pause", ms_ip);
            #endif
        }
        if(media_option == 5){
            #ifdef IS_SERVER
                send_media("play", ms_ip);
            #endif
        }
    }
}


char music_control(char stream_select, char input_option, char* input_src, unsigned int out_clients, unsigned int out_address[][4], char client){
    char music_text[MAX_STRING] = {0};
    char ps_id[8] = {0};
    sprintf(music_text, "\'/home/kyle/linux-main-share/MusicHD/%s\'", input_src);
    printf("Music inputs: %d, %d, %s, %u\n", stream_select, input_option, input_src, out_clients);
    grep_fp = popen("pgrep \"vlc\"", "r");
    if(grep_fp == NULL){
        printf("Failed to grep vlc...\n");
        return -1;
    } else {
        fgets(ps_id, sizeof(ps_id)-1, grep_fp);
        printf("\n\n----- Got %d for ps_id -----\n\n", ps_id[0]);
    }
    pclose(grep_fp);
    if(ps_id[0] > 0) active_music_count = 1;
    else active_music_count = 0;
    if(input_option == 1){ // start stream
        if(active_music_count == 0){
            #ifdef IS_SERVER
            start_music(stream_select, input_option, music_text, out_clients, out_address);
            #endif
            #ifdef IS_CLIENT
            start_listener(1, ms_ip);
            #endif
        } else {
            // too many movies going... don't start
            return 0;
        }
    } else {
        if(active_music_count){
            update_music(stream_select, input_option, input_src, out_clients, client);
        } else {
            // can't do anything to streams... none going
            return 0;
        }
    }
    return 0;
}

char start_music(char stream_select, char input_option, char* input_src, unsigned int out_count, unsigned int out_address[][4]){
    char send_count = 0;
    char stream_string[MAX_STRING] = {0};
    music_clients_ready[stream_select] = 0;
    music_clients[stream_select] = out_count;
    //active_music_count = active_music_count + 1;
    sprintf(stream_string, "su - %s -c \"cvlc -I rc --rc-host %u.%u.%u.%u:%u --extraintf=http --http-password=ms %s --sout-keep --sout-all --sout \'#gather:std{access=http,mux=ts,dst=:8080/ms0.mkv}\'&\"", username, ms_ip[0], ms_ip[1], ms_ip[2], ms_ip[3], (ms_port+1), input_src);
    printf("Starting: %s\n", stream_string);
    system(stream_string);
    sleep(2);
    send_media("pause", ms_ip);
    //send start stream to all outputs
    for(send_count=0;send_count<active_clients;send_count++){
        send(client_sockets[send_count], "1%mc11%", sizeof("1%mc11%"), 0);
    }
}

char update_music(char stream_select, char input_option, char* input_src, char out_clients, char client){
    if(input_option == 0){
        // stop selected stream
    }
    if(input_option == 2){
        music_clients_ready[stream_select] = music_clients_ready[stream_select] + 1;
        if(music_clients_ready[stream_select] >= music_clients[stream_select]){
            send_media("play", ms_ip);
        }
    }
    if(input_option == 3){
        #ifdef IS_SERVER
            send_media("stop", ms_ip);
            sleep(1);
            send_media("shutdown", ms_ip);
            //active_music_count = active_music_count - 1;
        #endif
    }
}

void send_media(unsigned char input_string[MAX_INPUT_STRING], unsigned int address[4]){
   char send_string[MAX_FUNCTION_STRING] = {0};
   printf("In Send Media: %s, %u.%u.%u.%u\n", input_string, address[0], address[1], address[2], address[3]);
   sprintf(send_string, "echo \"%s\" | nc -q 0 %u.%u.%u.%u %u", input_string, address[0], address[1], address[2], address[3], (ms_port+1));
   //send(client_sockets[0], send_string, sizeof(send_string), 0);
   system(send_string);
}

