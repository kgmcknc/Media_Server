
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include "configuration.h"

#define MOVIE_MAX 1
#define MUSIC_MAX 4


char movie_control(char stream_select, char input_option, char* input_src, unsigned int out_clients, unsigned int out_address[][4], char client);
char music_control(char stream_select, char input_option, char* input_src, unsigned int out_clients, unsigned int out_address[][4], char client);
char start_movie(char stream_select, char input_option, char* input_src, unsigned int out_count, unsigned int out_address[][4]);
char start_music(char stream_select, char input_option, char* input_src, unsigned int out_count, unsigned int out_address[][4]);
char update_movie(char stream_select, char input_option, char* input_src, char out_clients, char client);
char update_music(char stream_select, char input_option, char* input_src, char out_clients, char client);
void media_init(void);
void start_listener(char type, unsigned int in_address[4]);
void send_media(unsigned char input_string[MAX_INPUT_STRING], unsigned int address[4]);
