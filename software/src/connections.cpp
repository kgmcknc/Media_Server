
#include "config.h"
#include "connections.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <string.h>
// #include <fcntl.h>
//#include <sys/stat.h>
//#include <sys/types.h>
//#include <sys/wait.h>
 
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
// #include <sys/un.h>
// #include <sys/time.h>
//#include <wiringPi.h>

void heartbeat(struct system_config_struct* system_config){
    char packet_data[MAX_BROADCAST_PACKET];
    config_to_string(system_config, &packet_data[0]);

    while(CONTINUE_HEARTBEAT){
        sleep(HEARTBEAT_PERIOD_SEC);
        send_broadcast_packet(UDP_BROADCAST_PORT, &packet_data[0], UDP_TRANSFER_LENGTH);
        printf("\n---- Sent Heartbeat Command ----\n");
    }
}

int send_broadcast_packet(uint16_t broadcast_port, char* packet_data, uint16_t data_length){
    int conn_fd;
    int conn_socket;
    int conn_opt = 1;
    struct sockaddr_in conn_addr;
    int retval;

    conn_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(conn_fd == 0){
        printf("Main Socket Was 0\n");
        return -1;
    }
    if(setsockopt(conn_fd, SOL_SOCKET, SO_BROADCAST, (char *) &conn_opt, sizeof(conn_opt))){
        printf("Main Socket Opt Failed\n");
        return -1;
    }
    memset(&conn_addr, 0, sizeof(conn_addr));
    conn_addr.sin_family = AF_INET;
    conn_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    conn_addr.sin_port = htons(broadcast_port);

    retval = sendto(conn_fd,packet_data,data_length,0,(sockaddr *)&conn_addr,sizeof(conn_addr));
    close(conn_fd);
    return retval;
}

uint16_t receive_broadcast_packet(uint16_t broadcast_port, char* packet_data){
    int conn_fd;
    int conn_opt = 1;
    struct sockaddr_in conn_addr;
    unsigned int len = sizeof(struct sockaddr_in);
    uint16_t receive_length;
    int rx_len = MAX_BROADCAST_PACKET;

    conn_fd = socket(AF_INET, SOCK_DGRAM, 0);
    
    if(conn_fd == 0){
        printf("Main Socket Was 0\n");
        //return -1;
    }
    if(setsockopt(conn_fd, SOL_SOCKET, SO_BROADCAST, (char *) &conn_opt, sizeof(conn_opt))){
        printf("Main Socket Opt Failed\n");
        //return -1;
    }

    memset(&conn_addr, 0, sizeof(conn_addr));
    conn_addr.sin_family = AF_INET;
    conn_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    conn_addr.sin_port = htons(broadcast_port);
    bind(conn_fd,(sockaddr*)&conn_addr, sizeof (conn_addr));
    
    receive_length = recvfrom(conn_fd,packet_data,rx_len,0,(sockaddr *)&conn_addr,&len);
    close(conn_fd);
    return receive_length;
}

void listen_for_devices(struct system_config_struct* system_config, struct system_config_struct* new_device){
    char packet_data[MAX_BROADCAST_PACKET];
    uint16_t received_length;
    uint8_t found_media_server = 0;

    while(!found_media_server){
        received_length = receive_broadcast_packet(UDP_BROADCAST_PORT, &packet_data[0]);
        if(validate_packet(&packet_data[0], received_length)){
            string_to_config(&packet_data[0], new_device);
            found_media_server = (new_device->server_ip_addr != system_config->server_ip_addr);
        }
    }
    printf("found new device\n");
}

uint8_t validate_packet(char* packet_data, uint16_t packet_length){
    uint8_t valid_packet_length = 0;
    uint8_t valid_packet_data = 0;
    uint8_t valid_packet = 0;
    valid_packet_length = (packet_length == UDP_TRANSFER_LENGTH);
    valid_packet_data = (strstr(packet_data, "Media_Server System:") > 0);
    valid_packet = valid_packet_length & valid_packet_data;
    return valid_packet;
}

// #include "main.h"
// #include "sys_config.h"
// #include "sys_functions.h"
// #include "media_control.h"

// struct function_struct {
//     char type_flags;
//     char in_string[MAX_INPUT_STRING];
//     char in_length;
//     char f_string[MAX_FUNCTION_STRING];
//     char f_length;
//     char linked_function;
    
// };
// struct function_struct struct_functions[FUNCTION_COUNT];

// // function name to match web call string
// char function_name[FUNCTION_COUNT][MAX_FUNCTION_STRING] = {
//      "kmfsetport",
//      "kmfsetserverip",
//      "kmfsetclientip",
//      "kmfaddclient",
//      "kmfremclient",
//      "kmfupdclient",
//      "sendheartbeat",
//      "hello",
//      "imhere",
//      "starttightvnc",
//      "stoptightvnc",
//      "startvideo",
//      "stopvideo",
//      "startaudio",
//      "stopaudio",
//      "mc"
//  };
    
// // function call for system
// char function_call[FUNCTION_COUNT][MAX_FUNCTION_STRING] = {
//      "",
//      "",
//      "",
//      "",
//      "",
//      "",
//      "",
//      "",
//      "",
//      "tightvncserver",
//      "pkill Xtightvnc",
//      "omxplayer -o hdmi -b \"/home/pi/linux-main-share/MovieHD/\" </usr/share/myfolder/mysysproc/moviectrl/omxctrl >/usr/share/myfolder/mysysproc/moviectrl/omxlog & >/usr/share/myfolder/mysysproc/moviectrl/omxlog2 ; echo -n \"\" > /usr/share/myfolder/mysysproc/moviectrl/omxctrl",
//      "echo -n \"q\" > /usr/share/myfolder/mysysproc/moviectrl/omxctrl",
//      "omxplayer -o hdmi -b \"/home/pi/linux-main-share/MusicHD/\" </usr/share/myfolder/mysysproc/musicctrl/omxctrl >/usr/share/myfolder/mysysproc/musicctrl/omxlog & >/usr/share/myfolder/mysysproc/musicctrl/omxlog2 ; echo -n \"\" > /usr/share/myfolder/mysysproc/musicctrl/omxctrl",
//      "echo -n \"q\" > /usr/share/myfolder/mysysproc/musicctrl/omxctrl",
//      ""
//  };
 
// // length of the function call strings
// int function_length[FUNCTION_COUNT] = {
//     0,
//     0,
//     0,
//     0,
//     0,
//     0,
//     0,
//     0,
//     0,
//     14,
//     15,
//     271,
//     34,
//     271,
//     34,
//     0
// };

// // type of call: 0 - no condition, 1 - start, 2 - stop, 3 - extra text
// int function_type[FUNCTION_COUNT] = {
//     // one hot options:
//     // 0 - no options
//     // 1 - starter
//     // 2 - stopper
//     // 3 - extra text
//     // 4 - config
//     // 5 - function
//     // 6 - media control
//     0x18, // config extra text
//     0x18, // config extra text
//     0x18, // config extra text
//     0x18, // config extra text
//     0x18, // config extra text
//     0x18, // config extra text
//     0x18, // config extra text
//     0x18, // config extra text
//     0x18, // config extra text
//     0x2, // starter
//     0x4, // stopper
//     0xA, // starter with extra text
//     0x4, // stopper
//     0xA, // starter with extra text
//     0x4, // stopper
//     0x48
// };

// // option_status that "start" or "stop" is linked to
// int option_link[FUNCTION_COUNT] = {
//     0,
//     1,
//     2,
//     3,
//     4,
//     5,
//     6,
//     7,
//     8,
//     9, // option 0 - tightvncserver
//     9, // option 0 - tightvncserver
//     1, // option 1 - startvideo
//     1, // option 1 - stopvideo
//     2, // option 2 - startaudio
//     2, // option 2 - stopaudio
//     12
// };

// // extra_text_offset
// char extra_offset[FUNCTION_COUNT] = {
//     0,
//     0,
//     0,
//     0,
//     0,
//     0,
//     0,
//     0,
//     0,
//     0,
//     0,
//     56,
//     0,
//     56,
//     0,
//     2
// };

// int option_status[OPTION_COUNT] = {0};
// char option_name[OPTION_COUNT][MAX_STRING] = {
//     {10},
//     {14},
//     {14},
//     {12},
//     {12},
//     {12},
//     {13},
//     {5},
//     {6},
//     "tightvncserver",
//     {10}, // length of command constant
//     {10}, // length of command constant
//     {2}
// };

// void check_function(char client, char f_string[MAX_FUNCTION_STRING]){
//     int newfunction = 0;
//     int functionready = 0;
//     int localcount = 0;
//     int in_file = 0;
//     //char clearfile = 0;
//     char fend = 0;
//     pid_t forkid;
//     //int clrcnt = 0;
//     fvalid = 0;

//     newfunction = 0;
//     functionready = 0;
//     strcpy(filestring, f_string);
//     printf("Function is: %s\n", filestring);
//     //clearfile = 1;
//     functionready = 0;
//     if(filestring[0] < 48){ // character
//         if(FILE_DEBUG) printf("First was %c, not 0 or 1\n", filestring[0]);
//         functionready = 0;
//     } else {
//         filestring[0] = filestring[0] - 48;
//         if(filestring[0] == 1){
//             if(FILE_DEBUG) printf("First was 1, Valid set\n");
//             functionready = 1;
//         } else {
//             if(FILE_DEBUG) printf("First was %d, not 1\n", filestring[0]);
//             functionready = 0;
//         }
//     }
    
//     if(functionready){
//         functionready = 0;
//         if(FILE_DEBUG) printf("Found Valid Function!\n");
//         if(filestring[1] == '%'){
//             if(FILE_DEBUG) printf("Second was %%!\n");
//             fend = 0;
//             while(!fend && (localcount < (MAX_FUNCTION_STRING - 2))){
//                 if(filestring[2 + localcount] == '%'){
//                     if(FILE_DEBUG) printf("Found end %%!\n");
//                     fvalid = 1;
//                     fend = 1;
//                     flength = localcount;
//                     if(FILE_DEBUG) printf("Function Is: %s\n", funcstring);
//                     if(FILE_DEBUG) printf("Function Length Is: %d\n", flength);
//                 } else {
//                     funcstring[localcount] = filestring[2 + localcount];
//                 }
//                 localcount = localcount + 1;
//             }
//         } else {
//             if(FILE_DEBUG) printf("Second was %c, not %%\n", filestring[1]);
//         }
//     }
//     process_function(client);
// }

// void process_function(char client){
//     int localcount = 0;
//     char match = 0;
//     int func_const = 0;
//     int total_cnt = 0;
//     int extra_cnt = 0;
//     int cust_cnt = 0;
//     int tmp_cnt = 0;
//     char cust_func[MAX_FUNCTION_STRING] = {0};
//     if(fvalid){
//         if(FILE_DEBUG) printf("Function: %s\n", funcstring);
//         fvalid = 0;
//         for(localcount=0;localcount<FUNCTION_COUNT;localcount=localcount+1){
//             if(function_type[localcount] & 0x8){ // check for extra text type
//                 func_const = option_name[option_link[localcount]][0];
//                 if(!strncmp(funcstring,function_name[localcount],func_const)){
//                     match = 1;
//                     break;
//                 }
//             } else {
//                 if(!strcmp(funcstring,function_name[localcount])){
//                     match = 1;
//                     break;
//                 }
//             }
//         }
//         if(match == 1){
//             if(function_type[localcount] == 0){
//                 system(function_call[localcount]);
//             }
//             if(function_type[localcount] == 0x2){
//                 if(option_link[localcount] < OPTION_COUNT){
//                     if(option_status[option_link[localcount]] == 0){
//                         option_status[option_link[localcount]] = 1;
//                         system(function_call[localcount]);
//                     } else {
//                         printf("wrong state to process\n");
//                     }
//                 } else {
//                     printf("link is wrong\n");
//                 }
//             }
//             if(function_type[localcount] == 0x4){
//                 if(option_link[localcount] < OPTION_COUNT){
//                     if(option_status[option_link[localcount]] == 1){
//                         system(function_call[localcount]);
//                         option_status[option_link[localcount]] = 0;
//                     }else {
//                         printf("wrong state to process\n");
//                     }
//                 } else {
//                     printf("link is wrong\n");
//                 }
//             }
//             if(function_type[localcount] == 0x18){
//                 if(option_link[localcount] < OPTION_COUNT){
//                      extra_cnt = func_const;
//                      tmp_cnt = 0;
//                      printf("Extra: %d, Flength: %d\n", extra_cnt, flength);
//                      for(cust_cnt=extra_cnt;cust_cnt<flength;cust_cnt++){
//                         cust_func[tmp_cnt] = funcstring[cust_cnt];
//                         tmp_cnt = tmp_cnt + 1;
//                      }
//                      printf("String: %s\n", cust_func);
//                      if(localcount == 0) web_port_update(&cust_func[0]);
//                      if(localcount == 1) web_server_ip_update(&cust_func[0]);
//                      if(localcount == 2) web_set_client_ip(&cust_func[0]);
//                      if(localcount == 3) web_add_client(&cust_func[0]);
//                      if(localcount == 4) web_remove_client(&cust_func[0]);
//                      if(localcount == 5) web_update_client_ip(&cust_func[0]);
//                      if(localcount == 6) send_heartbeat_to_clients();
//                      if(localcount == 7) receive_heartbeat_from_server(&cust_func[0]);
//                      if(localcount == 8) receive_heartbeat_from_client(client, &cust_func[0]);
//                 } else {
//                     printf("link is wrong\n");
//                 }
//             }
//             if(function_type[localcount] == 0xA){
//                 if(option_link[localcount] < OPTION_COUNT){
//                     if(option_status[option_link[localcount]] == 0){
//                         if(FILE_DEBUG) printf("flength: %d, offset: %d\n", flength, extra_offset[localcount]);
//                         extra_cnt = flength - func_const;
//                         total_cnt = extra_cnt + function_length[localcount];
//                         if(FILE_DEBUG) printf("extra: %d, tot: %d\n", extra_cnt, total_cnt);
//                         tmp_cnt = 0;
//                         for(cust_cnt=0;cust_cnt<total_cnt;cust_cnt++){
//                             if(cust_cnt < extra_offset[localcount]){
//                                 cust_func[cust_cnt] = function_call[localcount][cust_cnt];
//                             } else {
//                                 if(cust_cnt < (extra_offset[localcount] + extra_cnt)){
//                                     cust_func[cust_cnt] = funcstring[func_const + tmp_cnt];
//                                     tmp_cnt = tmp_cnt + 1;
//                                 } else {
//                                     cust_func[cust_cnt] = function_call[localcount][cust_cnt - extra_cnt];
//                                 }
//                             }
//                         }
//                         if(FILE_DEBUG) printf("custom is: %s\n", cust_func);
//                         system(cust_func);
//                         // make that function call with additional text too..
//                         option_status[option_link[localcount]] = 1;
//                         // have to fix/link status stuff with this
//                     }else {
//                         printf("wrong state to process\n");
//                     }
//                 } else {
//                     printf("link is wrong\n");
//                 }
//             }
//             if(function_type[localcount] == 0x48){
//                 char mediatext[MAX_STRING] = {0};
//                 printf("Found Media Function!: %s\n", funcstring);
//                 char media_type = funcstring[2] - 48;
//                 if(media_type == 0){
//                     printf("Found Movie Function!\n");
//                     strncpy(mediatext, &funcstring[5], strlen(funcstring)-1);
//                     movie_control(0, (funcstring[3] - 48), mediatext, (funcstring[4] - 48), &client_ips[0], client);
//                 }
//                 if(media_type == 1){
//                     printf("Found Music Function!\n");
//                     strncpy(mediatext, &funcstring[5], strlen(funcstring)-1);
//                     music_control(0, (funcstring[3] - 48), mediatext, (funcstring[4] - 48), &client_ips[0], client);
//                 }
//             }
//         } else {
//             printf("Wasn't Known Function\n");
//         }
        
//         fvalid = 0;
//         for(localcount = 0;localcount<MAX_FUNCTION_STRING;localcount=localcount+1){
//             funcstring[localcount] = 0;
//             filestring[localcount] = 0;
//         }
//     }
// }

// void updatewebstate(FILE* out_file){
//     int localcount = 0;
    
//     for(localcount=0;localcount<OPTION_COUNT;localcount=localcount+1){
//         if(localcount > 0) fprintf(out_file, "\n");
//         fprintf(out_file, "%s: %d", option_name[localcount], option_status[localcount]);
//     }
    
//     rewind(out_file);
// }

// void init_webstate(FILE* out_file){
//     int localcount = 0;
    
//     for(localcount=0;localcount<OPTION_COUNT;localcount=localcount+1){
//         if(localcount > 0) fprintf(out_file, "\n");
//         fprintf(out_file, "%s: %d", option_name[localcount], option_status[localcount]);
//     }
    
//     rewind(out_file);
// }

// void transmit(int tx_port, char* tx_ip, char* tx_data){
   
// }

// void system_kill(char* proc_name){
//     char sys_string[MAX_STRING] = {0};
//     unsigned int proc_id = 0;
//     sprintf(sys_string, "pgrep \"%s\"", proc_name);
//     proc_id = system(sys_string);
//     if(proc_id > 0){
//         printf("Found %s, Killing...\n", proc_name);
//         sprintf(sys_string, "pkill \"%u\"", proc_id);
//         system(sys_string);
//     }
// }



