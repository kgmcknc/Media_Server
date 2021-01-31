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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PI_IS_ON 1
#define ERROR_HALT 1
#define FILE_DEBUG 1
#define USE_WPI 1
#define OPTION_COUNT 1
#define MAX_STRING 200
#define MAX_FUNCTION_STRING 200
#define TIMEOUT 10
#define KMF_COM_PORT 11
#define KMF_S_IP 13
#define MAX_CONFIG_FILE 4096

#define CONFIG_PATH "/usr/share/media_server/sys_control/sys_config.kmf"
#define RX_PIPE_PATH "/var/www/html/media_server/control/web_control/rxwebpipe"
#define TX_PIPE_PATH "/var/www/html/media_server/control/web_control/txwebpipe"

char function_name[MAX_FUNCTION_STRING] = {0};
unsigned int ms_ip[4] = {0};
void check_file(FILE* out_file);
void write_function(int socket, char* in_string);
int connect_client_socket(unsigned int ip[4], unsigned int port);
char read_ip_address(FILE* config_file, unsigned int* config_data, long int offset);
char read_port(FILE* config_file, unsigned int* config_data, long int offset);
FILE* config_file;
unsigned int file_ready;
unsigned int ms_port;
int new_socket;
int cfg_cnt = 0;
int file_length = 0;
char file_end = 0;

int main(int argc, char **argv)
{
    int rxfile;
    
    if(argc ==  2) {
        if(FILE_DEBUG) printf("New Function! text: %s, size: %d\n", argv[1], (int) strlen(argv[1]));
        
        config_file = fopen(CONFIG_PATH, "r");
        if(config_file != NULL){
            printf("\n\n----- Successfully Opened Config File -----\n\n");
        } else {
            printf("\n\n----- Failed To Open Config File, Exiting -----\n\n");
            return 0;
        }
        rewind(config_file);
        while(!file_end && (cfg_cnt < MAX_CONFIG_FILE)){
            fgetc(config_file);
            file_end = feof(config_file);
            cfg_cnt = cfg_cnt + 1;
        }
        rewind(config_file);
        file_length = cfg_cnt;
        read_ip_address(config_file, &ms_ip[0], KMF_S_IP);
        rewind(config_file);
        read_port(config_file, &ms_port, KMF_COM_PORT);
        fclose(config_file);
        new_socket = connect_client_socket(ms_ip, ms_port);
        if(new_socket < 0){
            printf("Couldn't Connect to System...\n");
        } else {
            write_function(new_socket, argv[1]);
            close(new_socket);
        }
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

int connect_client_socket(unsigned int ip[4], unsigned int port){
    char address[16];
    // connect to unix_socket as client
    int com_fd, com_socket, com_len, com_opt = 1;
    struct sockaddr_in com_addr;
    com_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(com_socket < 0){
        printf("Client Failed Socket\n");
        return -1;
    }
    memset(&com_addr, '0', sizeof(com_addr));
    com_addr.sin_family = AF_INET;
    com_addr.sin_port = htons(port);
    sprintf(address, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    if(inet_pton(AF_INET, address, &com_addr.sin_addr) <= 0){
        printf("Client Failed Address\n");
        return -1;
    }
    printf("Doing Connect in Client\n");
    
    if((connect(com_socket, (struct sockaddr *)&com_addr, sizeof(com_addr))) < 0){
        printf("Client Failed Connect\n");
        return -1;
    }
    
    return com_socket;
}

char read_ip_address(FILE* config_file, unsigned int* config_data, long int offset){
    unsigned char tmp_data = 0;
    unsigned char tmp_cnt = 0;
    
    if((offset < MAX_CONFIG_FILE) && (offset <= file_length)){
        if(fseek(config_file, offset, SEEK_SET)) return 0;
    } else {
        return 0;
    }
    
    while(tmp_cnt < 4){
        tmp_data = getc(config_file);
        if(tmp_data == EOF) return 0;
        config_data[tmp_cnt] = tmp_data;
        tmp_cnt = tmp_cnt + 1;
    }
    
    return 1;
}

char read_port(FILE* config_file, unsigned int* config_data, long int offset){
    unsigned char tmp_data = 0;
    unsigned int tmp_int = 0;
    
    if((offset < MAX_CONFIG_FILE) && (offset <= file_length)){
        if(fseek(config_file, offset, SEEK_SET)) return 0;
    } else {
        return 0;
    }
    
    tmp_data = getc(config_file);

    if(tmp_data == EOF) return 0;
    tmp_int = tmp_data;
    tmp_int = tmp_int << 8;
    tmp_data = getc(config_file);

    if(tmp_data == EOF) return 0;
    tmp_int = tmp_int | tmp_data;
    *config_data = tmp_int;

    return 1;
}

void write_function(int socket, char* in_string){
    int write_size = 0;
    strcat(function_name, "1%");
    strcat(function_name, in_string);
    strcat(function_name, "%");
    write_size = strlen(function_name);
    if(FILE_DEBUG) printf("Function is %s\n", function_name);
    send(socket, function_name, write_size, 0);
}
