/*
 * sys_control.c
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
#include <signal.h>
#include <unistd.h>
//#include <wiringPi.h>

#define PI_IS_ON 1
#define ERROR_HALT 1
#define FILE_DEBUG 1
#define USE_WPI 1
#define OPTION_COUNT 3
#define FUNCTION_COUNT 6
#define MAX_STRING 400
#define MAX_FUNCTION_STRING 400
#define MAX_CONFIG_FILE 4096
#define MIN_CONFIG_SIZE 20
#define TMP_DATA_SIZE 40
#define MAX_PORT 5000
#define MAX_CLIENTS 8

#define CONFIG_PATH "/usr/share/media_server/sys_control/sys_config.kmf"
#define RX_PATH "/var/www/html/media_server/control/web_control/rxwebpipe"
#define WEB_PATH "/var/www/html/media_server/control/web_control/txwebpipe"

// function name to match web call string
char function_name[FUNCTION_COUNT][MAX_FUNCTION_STRING] = {
        "starttightvnc",
        "stoptightvnc",
        "startvideo",
        "stopvideo",
        "startaudio",
        "stopaudio"
    };
// function call for system
char function_call[FUNCTION_COUNT][MAX_FUNCTION_STRING] = {
        "tightvncserver",
        "pkill Xtightvnc",
        "omxplayer -o hdmi -b \"/home/pi/linux-main-share/MovieHD/\" </usr/share/myfolder/mysysproc/moviectrl/omxctrl >/usr/share/myfolder/mysysproc/moviectrl/omxlog & >/usr/share/myfolder/mysysproc/moviectrl/omxlog2 ; echo -n \"\" > /usr/share/myfolder/mysysproc/moviectrl/omxctrl",
        "echo -n \"q\" > /usr/share/myfolder/mysysproc/moviectrl/omxctrl",
        "omxplayer -o hdmi -b \"/home/pi/linux-main-share/MusicHD/\" </usr/share/myfolder/mysysproc/musicctrl/omxctrl >/usr/share/myfolder/mysysproc/musicctrl/omxlog & >/usr/share/myfolder/mysysproc/musicctrl/omxlog2 ; echo -n \"\" > /usr/share/myfolder/mysysproc/musicctrl/omxctrl",
        "echo -n \"q\" > /usr/share/myfolder/mysysproc/musicctrl/omxctrl"
    };
// length of the function call strings
int function_length[FUNCTION_COUNT] = {
    14,
    15,
    271,
    34,
    271,
    34
};
// type of call: 0 - no condition, 1 - start, 2 - stop, 3 - extra text
int function_type[FUNCTION_COUNT] = {
    // one hot options:
    // 0 - no options
    // 1 - starter
    // 2 - stopper
    // 3 - extra text
    0x2, // starter
    0x4, // stopper
    0xA, // starter with extra text
    0x4, // stopper
    0xA, // starter with extra text
    0x4  // stopper
};
// option_status that "start" or "stop" is linked to
int option_link[FUNCTION_COUNT] = {
    0, // option 0 - tightvncserver
    0, // option 0 - tightvncserver
    1, // option 1 - startvideo
    1, // option 1 - stopvideo
    2, // option 2 - startaudio
    2 // option 2 - stopaudio
};
// extra_text_offset
char extra_offset[FUNCTION_COUNT] = {
    0,
    0,
    56,
    0,
    56,
    0
};

int option_status[OPTION_COUNT] = {0};
char option_name[OPTION_COUNT][MAX_STRING] = {
    "tightvncserver",
    {10}, // length of command constant
    {10} // length of command constant
    };

void configure_system(void);
void print_config_menu(void);
char check_config(FILE* config_file);
char read_file_size(FILE* config_file, long int* config_data, long int offset);
char write_file_size(FILE* config_file, long int* config_data, long int offset);
char read_port(FILE* config_file, unsigned int* config_data, long int offset);
char write_port(FILE* config_file, unsigned int* config_data, long int offset);
char read_ip_address(FILE* config_file, unsigned int* config_data, long int offset);
char write_ip_address(FILE* config_file, unsigned int* config_data, long int offset);
char add_client(FILE* config_file, unsigned int* config_data, long int offset);
unsigned int read_clients(FILE* config_file, unsigned int config_data[][4], long int offset);
char remove_client(FILE* config_file, unsigned int config_data, long int offset);
char reorder_clients(FILE* config_file, unsigned int config_data, unsigned int config_data_n, long int offset);
char read_config_data(FILE* config_file, char* config_data, long int offset, int size);
char write_config_data(FILE* config_file, char* config_data, long int offset, int size);

void piusleep(int sleeptime);
void checkfunctionfile(void);
void updatewebstate(FILE* out_file);
void init_webstate(FILE* out_file);
void process_function(void);

char filestring[MAX_STRING] = {0};
char funcstring[MAX_STRING] = {0};
char fvalid = 0;
int flength = 0;

FILE* config_file;

int main(int argc, char **argv)
{
    
    printf("\n\n----- Starting Kyle's System -----\n\n");
    
    printf("\n\n----- Opening Config File -----\n\n");
    config_file = fopen(CONFIG_PATH, "r+b");
    if(config_file != NULL){
        printf("\n\n----- Successfully Opened Config File -----\n\n");
    } else {
        printf("\n\n----- Failed To Open Config File Trying to Create-----\n\n");
        config_file = fopen(CONFIG_PATH, "a+b");
        if(config_file != NULL){
            printf("\n\n----- Successfully Created Config File -----\n\n");
            fclose(config_file);
            config_file = fopen(CONFIG_PATH, "r+b");
            if(config_file != NULL){
                printf("\n\n----- Successfully Re-Opened Config File -----\n\n");
            } else {
                printf("\n\n----- Failed To Open Config File, Exiting -----\n\n");
                exit(EXIT_FAILURE);
            }
        } else {
            printf("\n\n----- Failed To Open Config File, Exiting -----\n\n");
            exit(EXIT_FAILURE);
        }
    }
    rewind(config_file);
    
    if(argc > 1){
        printf("\n\n----- Checking Input Argument -----\n\n");
        if(strcmp(argv[1], "Config") || strcmp(argv[1], "config")){
            printf("\n\n----- Running System Config -----\n\n");
            configure_system();
        } else {
            printf("\n\n----- Unknown Argument... Exiting -----\n\n");
            exit(EXIT_FAILURE);
        }
    }
    
    /*printf("\n\n----- Checking Configuration Settings -----\n\n");
    if(check_config(config_file)){
        printf("\n\n----- Configuration Failure -----\n\n");
        exit(EXIT_FAILURE);
    } else {
        printf("\n\n----- Valid Configuration -----\n\n");
    }*/
    
    printf("\n\n----- Resetting Web State -----\n\n");
    //init_webstate(webfile);
    
    printf("\n\n----- Starting Main Loop -----\n\n");
    while(PI_IS_ON){
        // wait, so pi only checks function file every second or so...
        //if(USE_WPI) delay(2500);
        //else piusleep(1000000);
        
        //check to see if file changed
        checkfunctionfile();
        
        //process functions
        // have a function list
        // have function states that get sent to php site...
        process_function();
        
        //update web page file
        //updatewebstate(webfile); // use hash to make sure states are stable
    }
    fclose(config_file);
    return 0;
}

void configure_system(void){
    long int file_size = 0;
    int cfg_cnt = 0;
    char file_end = 0;
    char user_option = 0;
    unsigned int tmp_ip[4] = {0};
    char cfg_data[TMP_DATA_SIZE] = {0};
    unsigned int tmp_data = 0;
    unsigned int tmp_data_n = 0;
    unsigned int tmp_port = 0;
    int tmp_count = 0;
    char handled = 0;
    char data = 0;
    char empty;
    unsigned int num_clients;
    unsigned int tmp_clients[MAX_CLIENTS][4];
    
    // user configuration of system
    // read file and post data to user to use or change
    // fix config file it broken
    rewind(config_file);
    cfg_cnt = 0;
    while(!file_end && (cfg_cnt < MAX_CONFIG_FILE)){
        fgetc(config_file);
        file_end = feof(config_file);
        if(!file_end) cfg_cnt = cfg_cnt + 1;
    }
    printf("\nConfig File Was %d Bytes Long\n", cfg_cnt);
    
    if(cfg_cnt < MIN_CONFIG_SIZE){
        printf("\nConfig File Was Too Short! Adding Length!\n");
        if(cfg_cnt < MAX_CONFIG_FILE) fseek(config_file, cfg_cnt, SEEK_SET);
        data = 0;
        while(cfg_cnt < MIN_CONFIG_SIZE){
            fputc(data, config_file);
            cfg_cnt = cfg_cnt + 1;
        }
        printf("\nSet Config File to Minimum Length\n");
    }
    rewind(config_file);
    
    printf("\nConfig File Is Now %d Bytes Long\n", cfg_cnt);
    file_size = cfg_cnt;
    write_file_size(config_file, &file_size, 6);
    
    strncpy(&cfg_data[0], "kmf", 3);
    write_config_data(config_file, &cfg_data[0], 0, 3);
    
    cfg_data[0] = 0;
    cfg_data[1] = 0;
    cfg_data[2] = 1;
    write_config_data(config_file, &cfg_data[0], 3, 3);
    
    while(user_option != 'q'){
        handled = 0;
        user_option = 0;
        print_config_menu();
        scanf("%c%c", &user_option, &empty);
        if(user_option == 'q'){
            handled = 1;
            printf("\nExiting Configuration\n");
            break;
        }
        if(user_option == 'p'){
            handled = 1;
            printf("\nDefault Port Is 3000\n");
            if(read_port(config_file, &tmp_port, 10)){
                printf("port read: %d\n", tmp_port);
                if(tmp_port > 0){
                    printf("Current Port Is Set To: %d", tmp_port);
                } else {
                    printf("\nPort Is Not Currently Set\n");
                }
            } else {
                printf("\nCouldn't Read Port Information\n");
            }
            printf("\nSet Port or enter 0 to Go Back\n");
            printf("\nNew Port: "); scanf("%d%c", &tmp_port, &empty);
            if(tmp_port == 0){
                printf("\nGot 0, Going Back to Menu\n");
            }else{
                printf("\nGot %d for new port\n", tmp_port);
                if((tmp_port > 0) && (tmp_port < MAX_PORT)){
                    write_port(config_file, &tmp_port, 10);
                } else {
                    printf("\nPort Was Out of Range, Try Setting Again\n");
                }
            }
        }
        if(user_option == 's'){
            handled = 1;
            printf("\nServer IP Should Be Static\n");
            if(read_ip_address(config_file, &tmp_ip[0], 12)){
                printf("Current Server IP Is Set To: %d.%d.%d.%d", tmp_ip[0], tmp_ip[1], tmp_ip[2], tmp_ip[3]);
            } else {
                printf("\nServer IP Is Not Currently Set\n");
            }
            printf("\nSet New Server IP with \"XXX.XXX.XXX.XXX\" or enter 0 to Go Back\n");
            printf("New Server IP: ");
            scanf("%u.%u.%u.%u%c", &tmp_ip[0], &tmp_ip[1], &tmp_ip[2], &tmp_ip[3], &empty);
            if((tmp_ip[0] == 0) || (tmp_ip[1] == 0) || (tmp_ip[2] == 0) || (tmp_ip[3] == 0)){
                printf("\nGot 0, Going Back to Menu\n");
            }else{
                printf("\nGot %d.%d.%d.%d For New Server IP\n", tmp_ip[0], tmp_ip[1], tmp_ip[2], tmp_ip[3]);
                if((tmp_ip[0] > 0) && (tmp_ip[1] > 0) && (tmp_ip[2] > 0) && (tmp_ip[3] > 0)){
                    write_ip_address(config_file, &tmp_ip[0], 12);
                } else {
                    printf("\nIP Was Out of Range, Try Setting Again\n");
                }
            }
        }
        if(user_option == 'c'){
            handled = 1;
            printf("\nListing Attached Clients\n");
            num_clients = read_clients(config_file, tmp_clients, 16);
            if(num_clients){
                printf("Number of Clients: %d\n", num_clients);
                tmp_count = 0;
                while(tmp_count < num_clients){
                    printf("Client %d: %d.%d.%d.%d\n", tmp_count + 1, tmp_clients[tmp_count][0], tmp_clients[tmp_count][1], tmp_clients[tmp_count][2], tmp_clients[tmp_count][3]);
                    tmp_count = tmp_count + 1;
                }
            } else {
                printf("\nNo Clients Connected\n");
            }
        }
        if(user_option == 'a'){
            handled = 1;
            printf("\nAdding Client To System\n");
            num_clients = read_clients(config_file, tmp_clients, 16);
            if(num_clients < MAX_CLIENTS){
                printf("number of clients: %d\n", num_clients);
                printf("\nSet New Client IP with \"XXX.XXX.XXX.XXX\" or enter 0 to Go Back\n");
                printf("New Client IP: ");
                scanf("%u.%u.%u.%u%c", &tmp_ip[0], &tmp_ip[1], &tmp_ip[2], &tmp_ip[3], &empty);
                if((tmp_ip[0] == 0) || (tmp_ip[1] == 0) || (tmp_ip[2] == 0) || (tmp_ip[3] == 0)){
                    printf("\nGot 0, Going Back to Menu\n");
                }else{
                    printf("\nGot %d.%d.%d.%d For New Client IP\n", tmp_ip[0], tmp_ip[1], tmp_ip[2], tmp_ip[3]);
                    if((tmp_ip[0] > 0) && (tmp_ip[1] > 0) && (tmp_ip[2] > 0) && (tmp_ip[3] > 0)){
                        add_client(config_file, &tmp_ip[0], 16);
                    } else {
                        printf("\nIP Was Out of Range, Try Setting Again\n");
                    }
                }
            } else {
                printf("\nToo Many Clients To Add Another\n");
            }
        }
        if(user_option == 'r'){
            handled = 1;
            printf("\nRemoving Client From System\n");
            num_clients = read_clients(config_file, tmp_clients, 16);
            if(num_clients > 0){
                printf("Number of Clients: %d\n", num_clients);
                tmp_count = 0;
                while(tmp_count < num_clients){
                    printf("Client %d: %d.%d.%d.%d\n", tmp_count + 1, tmp_clients[tmp_count][0], tmp_clients[tmp_count][1], tmp_clients[tmp_count][2], tmp_clients[tmp_count][3]);
                    tmp_count = tmp_count + 1;
                }
                printf("\nSelect Client To Remove: ");
                scanf("%u%c", &tmp_data, &empty);
                if((tmp_data == 0) || (tmp_data > num_clients)){
                    printf("Cannot Remove Client %d\n", tmp_data);
                } else {
                    remove_client(config_file, tmp_data, 16);
                }
            } else {
                printf("\nNo Clients To Remove\n");
            }
        }
        if(user_option == 'o'){
            handled = 1;
            printf("\nRe-Order Clients On System\n");
            num_clients = read_clients(config_file, tmp_clients, 16);
            if(num_clients > 0){
                printf("Number of Clients: %d\n", num_clients);
                tmp_count = 0;
                while(tmp_count < num_clients){
                    printf("Client %d: %d.%d.%d.%d\n", tmp_count + 1, tmp_clients[tmp_count][0], tmp_clients[tmp_count][1], tmp_clients[tmp_count][2], tmp_clients[tmp_count][3]);
                    tmp_count = tmp_count + 1;
                }
                printf("\nSelect 2 Clients To Re-Order (Switch)\n");
                printf("\nInput Two Numbers Separated By A Single Space. x x: ");
                scanf("%u %u%c", &tmp_data, &tmp_data_n, &empty);
                if((tmp_data > 0) && (tmp_data_n > 0) && (tmp_data <= num_clients) && (tmp_data_n <= num_clients)){
                    reorder_clients(config_file, tmp_data, tmp_data_n, 16);
                }
            } else {
                printf("\nNo Clients To Re-Order\n");
            }
        }
        if(handled == 0){
            printf("\nUnknown Value!\n");
        }
    }
}

void print_config_menu(void){
    printf("\n------------------------------\n");
    printf("\n---Choose An Option To View---\n");
    printf("\n------------------------------\n");
    printf("\nq  Exit System Configuration  \n");
    printf("\np  Communication Port:        \n");
    printf("\ns  Server IP Address:         \n");
    printf("\nc  View Attached Clients      \n");
    printf("\na  Add New Client IPs         \n");
    printf("\nr  Remove Attached Clients    \n");
    printf("\no  Re-Order Attached Clients  \n");
    printf("\n------------------------------\n");
    printf("\nType Input Selection: ");
}

char check_config(FILE* config_file){
    //int cfg_cnt = 0;
    char cfg_data[TMP_DATA_SIZE] = {0}; // max temp data - 40 bytes
    int cfg_cnt = 0;
    char file_end = 0;
    
    rewind(config_file);
    while(!file_end && (cfg_cnt < MAX_CONFIG_FILE)){
        fgetc(config_file);
        file_end = feof(config_file);
        cfg_cnt = cfg_cnt + 1;
    }
    rewind(config_file);
    
    if(cfg_cnt < MIN_CONFIG_SIZE){
        printf("\nConfig File Was Too Short! Need to Re-Configure!\n");
        return 1;
    } else {
        // Parsing config file (kmf file)
        if(!read_config_data(config_file, &cfg_data[0], 0, 3)) return 1;
        printf("config data: %s\n", cfg_data);
        if(!read_config_data(config_file, &cfg_data[0], 3, 3)) return 1;
        printf("config data: %s\n", cfg_data);
        if(!read_config_data(config_file, &cfg_data[0], 6, 4)) return 1;
        printf("config data: %s\n", cfg_data);
        if(!read_config_data(config_file, &cfg_data[0], 10, 2)) return 1;
        printf("config data: %s\n", cfg_data);
        if(!read_config_data(config_file, &cfg_data[0], 14, 2)) return 1;
        printf("config data: %s\n", cfg_data);
        if(!read_config_data(config_file, &cfg_data[0], 18, 2)) return 1;
        printf("config data: %s\n", cfg_data);
        if(!read_config_data(config_file, &cfg_data[0], 22, 4)) return 1;
        printf("config data: %s\n", cfg_data);
        return 0;
    }
}

char read_config_data(FILE* config_file, char* config_data, long int offset, int size){
    char data[TMP_DATA_SIZE] = {0};
    int tmp_cnt = 0;
    char tmp_data = 0;
    
    if(offset < MAX_CONFIG_FILE) fseek(config_file, offset, SEEK_SET);
    
    while((tmp_cnt < size) && (tmp_cnt < TMP_DATA_SIZE)){
        tmp_data = getc(config_file);
        if(tmp_data == EOF) return 0;
        data[tmp_cnt] = tmp_data;
        tmp_cnt = tmp_cnt + 1;
    }
    
    strncpy(config_data, &data[0], size);
    return 1;
}

char write_config_data(FILE* config_file, char* config_data, long int offset, int size){
    unsigned int tmp_cnt = 0;
    char tmp_data = 0;
    
    if(offset < MAX_CONFIG_FILE) fseek(config_file, offset, SEEK_SET);
    
    while((tmp_cnt < size) && (tmp_cnt < TMP_DATA_SIZE)){
        tmp_data = config_data[tmp_cnt];
        fputc(tmp_data, config_file);
        tmp_cnt = tmp_cnt + 1;
    }
    
    return 0;
}

char read_port(FILE* config_file, unsigned int* config_data, long int offset){
    unsigned char tmp_data = 0;
    unsigned int tmp_int = 0;
    
    if(offset < MAX_CONFIG_FILE) fseek(config_file, offset, SEEK_SET);
    
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

char write_port(FILE* config_file, unsigned int* config_data, long int offset){
    unsigned char tmp_data = 0;
    int tmp_int = 0;
    
    tmp_int = *config_data;
    printf("got write port: %d\n", tmp_int);
    if(offset < MAX_CONFIG_FILE) fseek(config_file, offset, SEEK_SET);
    tmp_data = (tmp_int >> 8) & 0xFF;

    fputc(tmp_data, config_file);
    tmp_data = tmp_int & 0xFF;

    fputc(tmp_data, config_file);
    
    return 0;
}

char read_ip_address(FILE* config_file, unsigned int* config_data, long int offset){
    unsigned char tmp_data = 0;
    unsigned char tmp_cnt = 0;
    
    if(offset < MAX_CONFIG_FILE) fseek(config_file, offset, SEEK_SET);
    
    while(tmp_cnt < 4){
        tmp_data = getc(config_file);
        if(tmp_data == EOF) return 0;
        config_data[tmp_cnt] = tmp_data;
        tmp_cnt = tmp_cnt + 1;
    }
    
    return 1;
}

char write_ip_address(FILE* config_file, unsigned int* config_data, long int offset){
    unsigned char tmp_cnt = 0;
    unsigned char tmp_data = 0;
    
    if(offset < MAX_CONFIG_FILE) fseek(config_file, offset, SEEK_SET);
    
    while(tmp_cnt < 4){
        tmp_data = config_data[tmp_cnt];
        fputc(tmp_data, config_file);
        tmp_cnt = tmp_cnt + 1;
    }
    
    return 0;
}

char add_client(FILE* config_file, unsigned int* config_data, long int offset){
    unsigned char tmp_data = 0;
    unsigned char tmp_cnt = 0;
    unsigned char client_count = 0;
    long int new_offset;
    long int file_size;
    
    if(offset < MAX_CONFIG_FILE) fseek(config_file, offset, SEEK_SET);
    
    tmp_data = getc(config_file);
    if(tmp_data == EOF) return 0;
    client_count = tmp_data;
    client_count = (client_count << 8);
    tmp_data = getc(config_file);
    if(tmp_data == EOF) return 0;
    client_count = client_count | tmp_data;
    
    new_offset = offset + 4 + (4*(client_count));
    if(new_offset < MAX_CONFIG_FILE) fseek(config_file, new_offset, SEEK_SET);
    
    while(tmp_cnt < 4){
        tmp_data = config_data[tmp_cnt];
        fputc(tmp_data, config_file);
        tmp_cnt = tmp_cnt + 1;
    }
    
    client_count = client_count + 1;
    if(offset < MAX_CONFIG_FILE) fseek(config_file, offset, SEEK_SET);
    tmp_data = (client_count >> 8) & 0xFF;
    fputc(tmp_data, config_file);
    tmp_data = client_count & 0xFF;
    fputc(tmp_data, config_file);
    
    read_file_size(config_file, &file_size, 6);
    file_size = file_size + 4;
    write_file_size(config_file, &file_size, 6);
    
    return 0;
}

char remove_client(FILE* config_file, unsigned int config_data, long int offset){
    unsigned char tmp_data = 0;
    unsigned char loop_count = 0;
    unsigned char client_count = 0;
    unsigned int tmp_ip[4];
    long int new_offset;
    long int file_size;
    
    if(offset < MAX_CONFIG_FILE) fseek(config_file, offset, SEEK_SET);
    tmp_data = getc(config_file);
    if(tmp_data == EOF) return 0;
    client_count = tmp_data;
    client_count = (client_count << 8);
    tmp_data = getc(config_file);
    if(tmp_data == EOF) return 0;
    client_count = client_count | tmp_data;
    
    loop_count = (config_data - 1);
    while(loop_count < client_count){
        new_offset = offset + 4 + (4*(loop_count+1));
        if(new_offset < MAX_CONFIG_FILE){
            read_ip_address(config_file, &tmp_ip[0], new_offset);
            write_ip_address(config_file, &tmp_ip[0], (new_offset - 4));
        }
        loop_count = loop_count + 1;
    }
    
    client_count = client_count - 1;
    if(offset < MAX_CONFIG_FILE) fseek(config_file, offset, SEEK_SET);
    tmp_data = (client_count >> 8) & 0xFF;
    fputc(tmp_data, config_file);
    tmp_data = client_count & 0xFF;
    fputc(tmp_data, config_file);
    
    read_file_size(config_file, &file_size, 6);
    file_size = file_size - 4;
    write_file_size(config_file, &file_size, 6);
    ftruncate(fileno(config_file), file_size);
    
    return 0;
}

char reorder_clients(FILE* config_file, unsigned int config_data, unsigned int config_data_n, long int offset){
    unsigned char tmp_data = 0;
    unsigned char client_count = 0;
    unsigned int tmp_ip[4];
    unsigned int tmp_ip_n[4];
    long int new_offset;
    
    if(offset < MAX_CONFIG_FILE) fseek(config_file, offset, SEEK_SET);
    tmp_data = getc(config_file);
    if(tmp_data == EOF) return 0;
    client_count = tmp_data;
    client_count = (client_count << 8);
    tmp_data = getc(config_file);
    if(tmp_data == EOF) return 0;
    client_count = client_count | tmp_data;
    
    new_offset = offset + 4 + (4*(config_data - 1));
    if(new_offset < MAX_CONFIG_FILE){
        read_ip_address(config_file, &tmp_ip[0], new_offset);
    }
    new_offset = offset + 4 + (4*(config_data_n - 1));
    if(new_offset < MAX_CONFIG_FILE){
        read_ip_address(config_file, &tmp_ip_n[0], new_offset);
    }
    new_offset = offset + 4 + (4*(config_data - 1));
    if(new_offset < MAX_CONFIG_FILE){
        write_ip_address(config_file, &tmp_ip_n[0], new_offset);
    }
    new_offset = offset + 4 + (4*(config_data_n - 1));
    if(new_offset < MAX_CONFIG_FILE){
        write_ip_address(config_file, &tmp_ip[0], new_offset);
    }
    
    return 0;
}

char read_file_size(FILE* config_file, long int* config_data, long int offset){
    int tmp_data = 0;
    int tmp_cnt = 4;
    long int tmp_int= 0;
    
    if(offset < MAX_CONFIG_FILE) fseek(config_file, offset, SEEK_SET);
    
    while(tmp_cnt){
        tmp_cnt = tmp_cnt - 1;
        tmp_data = getc(config_file);
        if(tmp_data == EOF) return 0;
        tmp_int = tmp_int | (tmp_data << (8*tmp_cnt));
    }
    
    *config_data = tmp_int;
    return 1;
}

unsigned int read_clients(FILE* config_file, unsigned int config_data[][4], long int offset){
    unsigned char tmp_data = 0;
    unsigned char tmp_cnt = 0;
    unsigned char tmp_counter = 0;
    unsigned char client_count = 0;
    
    if(offset < MAX_CONFIG_FILE) fseek(config_file, offset, SEEK_SET);
    
    tmp_data = getc(config_file);
    if(tmp_data == EOF) return 0;
    client_count = tmp_data;
    client_count = (client_count << 8);
    tmp_data = getc(config_file);
    if(tmp_data == EOF) return 0;
    client_count = client_count | tmp_data;
    
    tmp_data = getc(config_file);
    tmp_data = getc(config_file);
    
    while((tmp_cnt < client_count) && (tmp_cnt < MAX_CLIENTS)){
        tmp_counter = 0;
        while(tmp_counter < 4){
            tmp_data = getc(config_file);
            if(tmp_data == EOF) return 0;
            config_data[tmp_cnt][tmp_counter] = tmp_data;
            tmp_counter = tmp_counter + 1;
        }
        tmp_cnt = tmp_cnt + 1;
    }
    
    return client_count;
}

char write_file_size(FILE* config_file, long int* config_data, long int offset){
    char tmp_cnt = 3;
    char tmp_data = 0;
    
    if(offset < MAX_CONFIG_FILE) fseek(config_file, offset, SEEK_SET);
    
    while(tmp_cnt){
        tmp_data = ((*config_data >> (8*tmp_cnt)) & 0xFF);
        fputc(tmp_data, config_file);
        tmp_cnt = tmp_cnt - 1;
    }
    tmp_data = *config_data & 0xFF;
    fputc(tmp_data, config_file);
    
    return 0;
}

void checkfunctionfile(void){
    int newfunction = 0;
    int functionready = 0;
    int localcount = 0;
    int in_file = 0;
    //char clearfile = 0;
    char fend = 0;
    pid_t forkid;
    //int clrcnt = 0;
    
    fvalid = 0;
    forkid = fork();
    if(forkid == 0){
        // child id -- timeout catch
        printf("called fork!\n");
        sleep(10);
        in_file = open(RX_PATH, O_WRONLY, 0x0);
        write(in_file, "T", 1);
        exit(EXIT_SUCCESS);
    } else {
        // Parent id -- read and handle timeout
        in_file = open(RX_PATH, O_RDONLY, 0x0);
        //newfunction = fscanf(in_file, "%400[^\n]", filestring);
        newfunction = read(in_file, filestring, 400);
        
        if(FILE_DEBUG) printf("Read: %d, String: %s\n", newfunction, filestring);
    //  rewind(in_file);
        if(newfunction > 0){
            //clearfile = 1;
            if(filestring[0] == 'T'){
                 kill(forkid, SIGKILL);
                 printf("Timeout!\n");
            } else {
                if(filestring[0] < 48){ // character
                    if(FILE_DEBUG) printf("First was %c, not 0 or 1\n", filestring[0]);
                    functionready = 0;
                } else {
                    filestring[0] = filestring[0] - 48;
                    if(filestring[0] == 1){
                        if(FILE_DEBUG) printf("First was 1, Valid set\n");
                        functionready = 1;
                    } else {
                        if(FILE_DEBUG) printf("First was %d, not 1\n", filestring[0]);
                        functionready = 0;
                    }
                }
            }
        } else {
            if(FILE_DEBUG) printf("Couldn't Read File...\n");
            functionready = 0;
        }
        
        if(functionready){
            if(FILE_DEBUG) printf("Found Valid Function!\n");
            if(filestring[1] == '%'){
                if(FILE_DEBUG) printf("Second was %%!\n");
                fend = 0;
                while(!fend && (localcount < (MAX_FUNCTION_STRING - 2))){
                    if(filestring[2 + localcount] == '%'){
                        if(FILE_DEBUG) printf("Found end %%!\n");
                        fvalid = 1;
                        fend = 1;
                        flength = localcount;
                        if(FILE_DEBUG) printf("Function Is: %s\n", funcstring);
                        if(FILE_DEBUG) printf("Function Length Is: %d\n", flength);
                    } else {
                        funcstring[localcount] = filestring[2 + localcount];
                    }
                    localcount = localcount + 1;
                }
            } else {
                if(FILE_DEBUG) printf("Second was %c, not %%\n", filestring[1]);
            }
        }
        //if(clearfile) {
        //  clearfile = 0;
        //  fputc('0', in_file);
        //  fputc('\0', in_file);
        //  rewind(in_file);
        //  for(clrcnt=0;clrcnt<MAX_STRING;clrcnt++) filestring[clrcnt] = 0;
        //}
        close(in_file);
    }
}

void process_function(void){
    int localcount = 0;
    char match = 0;
    int func_const = 0;
    int total_cnt = 0;
    int extra_cnt = 0;
    int cust_cnt = 0;
    int tmp_cnt = 0;
    char cust_func[MAX_FUNCTION_STRING] = {0};
    if(fvalid){
        if(FILE_DEBUG) printf("Function: %s\n", funcstring);
        
        for(localcount=0;localcount<FUNCTION_COUNT;localcount=localcount+1){
            if(function_type[localcount] & 0x8){ // check for extra text type
                func_const = option_name[option_link[localcount]][0];
                if(!strncmp(funcstring,function_name[localcount],func_const)){
                    match = 1;
                    break;
                }
            } else {
                if(!strcmp(funcstring,function_name[localcount])){
                    match = 1;
                    break;
                }
            }
        }
        if(match == 1){
            if(function_type[localcount] == 0){
                system(function_call[localcount]);
            }
            if(function_type[localcount] == 0x2){
                if(option_link[localcount] < OPTION_COUNT){
                    if(option_status[option_link[localcount]] == 0){
                        option_status[option_link[localcount]] = 1;
                        system(function_call[localcount]);
                    } else {
                        printf("wrong state to process\n");
                    }
                } else {
                    printf("link is wrong\n");
                }
            }
            if(function_type[localcount] == 0x4){
                if(option_link[localcount] < OPTION_COUNT){
                    if(option_status[option_link[localcount]] == 1){
                        system(function_call[localcount]);
                        option_status[option_link[localcount]] = 0;
                    }else {
                        printf("wrong state to process\n");
                    }
                } else {
                    printf("link is wrong\n");
                }
            }
            if(function_type[localcount] == 0xA){
                if(option_link[localcount] < OPTION_COUNT){
                    if(option_status[option_link[localcount]] == 0){
                        if(FILE_DEBUG) printf("flength: %d, offset: %d\n", flength, extra_offset[localcount]);
                        extra_cnt = flength - func_const;
                        total_cnt = extra_cnt + function_length[localcount];
                        if(FILE_DEBUG) printf("extra: %d, tot: %d\n", extra_cnt, total_cnt);
                        tmp_cnt = 0;
                        for(cust_cnt=0;cust_cnt<total_cnt;cust_cnt++){
                            if(cust_cnt < extra_offset[localcount]){
                                cust_func[cust_cnt] = function_call[localcount][cust_cnt];
                            } else {
                                if(cust_cnt < (extra_offset[localcount] + extra_cnt)){
                                    cust_func[cust_cnt] = funcstring[func_const + tmp_cnt];
                                    tmp_cnt = tmp_cnt + 1;
                                } else {
                                    cust_func[cust_cnt] = function_call[localcount][cust_cnt - extra_cnt];
                                }
                            }
                        }
                        if(FILE_DEBUG) printf("custom is: %s\n", cust_func);
                        system(cust_func);
                        // make that function call with additional text too..
                        option_status[option_link[localcount]] = 1;
                        // have to fix/link status stuff with this
                    }else {
                        printf("wrong state to process\n");
                    }
                } else {
                    printf("link is wrong\n");
                }
            }
        } else {
            printf("Wasn't Known Function\n");
        }
        
        fvalid = 0;
        for(localcount = 0;localcount<MAX_FUNCTION_STRING;localcount=localcount+1){
            funcstring[localcount] = 0;
            filestring[localcount] = 0;
        }
    }
}

void updatewebstate(FILE* out_file){
    int localcount = 0;
    
    for(localcount=0;localcount<OPTION_COUNT;localcount=localcount+1){
        if(localcount > 0) fprintf(out_file, "\n");
        fprintf(out_file, "%s: %d", option_name[localcount], option_status[localcount]);
    }
    
    rewind(out_file);
}

void init_webstate(FILE* out_file){
    int localcount = 0;
    
    for(localcount=0;localcount<OPTION_COUNT;localcount=localcount+1){
        if(localcount > 0) fprintf(out_file, "\n");
        fprintf(out_file, "%s: %d", option_name[localcount], option_status[localcount]);
    }
    
    rewind(out_file);
}

#define microtime 88
void piusleep(int sleeptime){
    int localcount;
    while(sleeptime){
        localcount = microtime;
        while(localcount){
             localcount = localcount - 1;
         }
         sleeptime = sleeptime - 1;
    }
}

void bin_to_char(){
    
}

void char_to_dec(char* input_char, int* output_dec){
    sscanf(input_char, "%d", output_dec);
    printf("decimal is %d\n", *output_dec);
}
