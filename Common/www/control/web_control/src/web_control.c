/*
 * websys.c
 * 
 * Copyright 2017  <pi@raspberrypi>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define PI_IS_ON 1
#define ERROR_HALT 1
#define FILE_DEBUG 1
#define USE_WPI 1
#define OPTION_COUNT 1
#define MAX_STRING 200
#define MAX_FUNCTION_STRING 200
#define TIMEOUT 10

#define RX_PIPE_PATH "/var/www/html/media_server/control/web_control/rxwebpipe"
#define TX_PIPE_PATH "/var/www/html/media_server/control/web_control/txwebpipe"

char function_name[MAX_FUNCTION_STRING] = {0};

void check_file(FILE* out_file);
void write_function(int out_file, char* in_string);

unsigned int file_ready;

int main(int argc, char **argv)
{
    int rxfile;
    
    if(argc ==  2) {
        if(FILE_DEBUG) printf("New Function! text: %s, size: %d\n", argv[1], (int) strlen(argv[1]));
        
        rxfile = open(RX_PIPE_PATH, O_WRONLY | O_NONBLOCK, 0x0);
        
        write_function(rxfile, argv[1]);
        
        close(rxfile);
        
    } else {
        if(FILE_DEBUG) printf("No Function...Closing\n");
    }
    
    return 0;
}

/*void check_file(FILE* out_file){
    int localcount;
    char tempval;
    char restart;
    
    localcount = 0;
    restart = 0;
    while (!file_ready && !restart && (localcount < timeout)){
        if(localcount){
            if(FILE_DEBUG) printf("Waiting before retry...\n");
            if(USE_WPI) delay(1000);
            else piusleep(1000000);
        }
        tempval = getc(out_file);
        if(tempval > 0){
            if(tempval < 48){
                if(FILE_DEBUG) printf("Invalid First...\n");
                file_ready = 0;
                localcount = localcount + 1;
                if (localcount == timeout) restart = 1;
                else restart = 0;
            } else {
                tempval = tempval - 48;
                if (tempval == 0){
                    if(FILE_DEBUG) printf("First was 0!\n");
                    file_ready = 1;
                    restart = 0;
                }
                if (tempval == 1) {
                    if(FILE_DEBUG) printf("First was 1...\n");
                    file_ready = 0;
                    restart = 0;
                    localcount = localcount + 1;
                }
                if (tempval > 1) {
                    if(FILE_DEBUG) printf("First was %d, not 0 or 1...\n", tempval);
                    file_ready = 0;
                    localcount = localcount + 1;
                    if (localcount == timeout) restart = 1;
                    else restart = 0;
                }
            }
        } else {
            if(FILE_DEBUG) printf("Empty File...\n");
            file_ready = 0;
            restart = 1;
        }
        rewind(out_file);
    }
    
    if(!file_ready){
        if(restart){
            if(FILE_DEBUG) printf("Restarting File...\n");
            putc('0', out_file);
            rewind(out_file);
            restart = 0;
            file_ready = 1;
        } else {
            if(FILE_DEBUG) printf("No Restart...\n");
        }
    }
}*/

void write_function(int out_file, char* in_string){
    int write_size = 0;
    strcat(function_name, "1%");
    strcat(function_name, in_string);
    strcat(function_name, "%");
    write_size = strlen(function_name);
    if(FILE_DEBUG) printf("Function is %s\n", function_name);
    write(out_file, function_name, write_size);
}
