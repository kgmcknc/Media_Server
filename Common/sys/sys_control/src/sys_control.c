
#include "sys_control.h"
#include "sys_config.h"
#include "sys_functions.h"
#include <stdio.h>
#include <stdlib.h>

void updatewebstate(FILE* out_file);
void init_webstate(FILE* out_file);

char filestring[MAX_STRING] = {0};
char funcstring[MAX_STRING] = {0};
char fvalid = 0;
int flength = 0;
char valid_config = 0;
long int file_length = 0;

char usertmp[MAX_STRING] = {0};
char username[MAX_STRING] = {0};
unsigned int ms_port = 0;
unsigned int ms_ip[4] = {0};
char client_count = 0;
unsigned int client_ips[MAX_CLIENTS][4] = {{0}};
char client_state[MAX_CLIENTS] = {0};
unsigned int client_id[MAX_CLIENTS] = {0};
char user_option = 0;
char send_heartbeat = 0;
char restart_heartbeat = 1;

char restart_listener = 0;
pid_t listener;

FILE* main_rx;
FILE* main_tx;
pid_t heartbeat_fork;

FILE* user_fp;
FILE* config_file;

int sys_sockets[MAX_SYS_SOCKETS] = {0};
int unix_sockets[MAX_UNIX_SOCKETS] = {0};
int local_sockets[MAX_LOCAL_SOCKETS] = {0};
int client_sockets[MAX_INET_SOCKETS] = {0};

char exit_server = 0;
char active_clients = 0;
char active_unix = 0;
char active_local = 0;
char active_system = 0;

int main_socket;
int local_socket;
int web_socket;

fd_set all_sockets;
int max_fds;

struct system_function sf_heartbeat;

int main(int argc, char **argv) {

    printf("\n\n----- Starting Kyle's System -----\n\n");
    
    user_fp = popen("ls /home/", "r");
    if(user_fp == NULL){
        printf("Failed to get User...\n");
        exit(1);
    } else {
        fgets(usertmp, sizeof(usertmp)-1, user_fp);
        strncpy(username, usertmp, (strlen(usertmp)-1));
        printf("\n\n----- Got %s As User -----\n\n", username);
    }
    pclose(user_fp);

    config_file = fopen(CONFIG_PATH, "r+b");
    if(config_file != NULL){
        printf("\n\n----- Successfully Opened Config File -----\n\n");
    } else {
        printf("\n\n----- Failed To Open Config File, Exiting -----\n\n");
        exit(EXIT_FAILURE);
    }
    rewind(config_file);

    if(argc > 1) {
        printf("\n\n----- Checking Input Argument -----\n\n");
        if(strcmp(argv[1], "Config") || strcmp(argv[1], "config")) {
                 printf("\n\n----- Running System Config -----\n\n");
        configure_system();
        } else {
            printf("\n\n----- Unknown Argument... Exiting -----\n\n");
            exit(EXIT_FAILURE);
        }
    }

    printf("\n\n----- Checking Configuration Settings -----\n\n");
    check_config(config_file);
    #ifdef IS_SERVER
        printf("\n\n----- Running as Server -----\n\n");
    #endif
    #ifdef IS_CLIENT
        printf("\n\n----- Running as Client -----\n\n");
    #endif
    
    system_setup();

#ifdef USE_HEARTBEAT
    heartbeat_fork = fork();
#endif
#ifndef USE_HEARTBEAT
    heartbeat_fork = 1;
#endif
    if(heartbeat_fork == 0){
        int heartbeat_socket_c;
        printf("\n---- Inside Heartbeat Child ----\n");
        sleep(2);
        heartbeat_socket_c = connect_unix_socket(HEARTBEAT_PATH);
        heartbeat(heartbeat_socket_c);
        printf("\n---- Exitting Heartbeat Child ----\n");
        exit(EXIT_SUCCESS);
    } else {
        printf("\n---- Inside Parent Server ----\n");
        server_system();
    }
    
    // kill all child processes
    kill(heartbeat_fork, SIGKILL);
    printf("\n---- Exitting System ----\n");
    exit(EXIT_SUCCESS);
}

void server_system(void){
    int heartbeat_socket_s;
    char system_state = RUNNING;
    active_system = 0;
    active_unix = 0;
    active_local = 0;
    active_clients = 0;

    sys_sockets[active_system] = create_main_socket(ms_port);
    active_system = active_system + 1;
#ifdef USE_HEARTBEAT
    unix_sockets[active_unix] = create_unix_socket(HEARTBEAT_PATH);
    active_unix = active_unix + 1;
#endif
#ifdef IS_CLIENT
    client_sockets[active_clients] = create_unix_socket(ms_ip, ms_port);
    active_clients = active_clients + 1;
#endif

    while(system_state == RUNNING){
        printf("In System While\n");
        system_state = socket_handler();
    }
}

void system_setup(void){
    function_setup();
}

void function_setup(void){
    set_function(&sf_heartbeat, "sendheartbeat");
}

void set_function(struct system_function* sf, char f_string[MAX_FUNCTION_STRING]){
    sprintf(sf->string, "1%%%s%%", f_string);
}

int socket_handler(){
    int s_count, r_count;
    int activity;
    int new_socket = 0;
    int new_addr_len;
    int num_read;
    int shift;
    char read_data[MAX_FUNCTION_STRING];
    struct sockaddr_in new_addr;
    new_addr_len = sizeof(new_addr);
    
    // clear socket set
    FD_ZERO(&all_sockets);
    // always update max file descriptor for select
    max_fds = 0;
    // set system sockets
    for(s_count=0;s_count<MAX_SYS_SOCKETS;s_count++){
        if(s_count >= active_system) break;
        FD_SET(sys_sockets[s_count], &all_sockets);
        if(sys_sockets[s_count] > max_fds) max_fds = sys_sockets[s_count];
    }
    // set unix sockets
    for(s_count=0;s_count<MAX_UNIX_SOCKETS;s_count++){
        if(s_count >= active_unix) break;
        FD_SET(unix_sockets[s_count], &all_sockets);
        if(unix_sockets[s_count] > max_fds) max_fds = unix_sockets[s_count];
    }
    // set local sockets
    for(s_count=0;s_count<MAX_LOCAL_SOCKETS;s_count++){
        if(s_count >= active_local) break;
        FD_SET(local_sockets[s_count], &all_sockets);
        if(local_sockets[s_count] > max_fds) max_fds = local_sockets[s_count];
    }
    // set client sockets
    for(s_count=0;s_count<MAX_INET_SOCKETS;s_count++){
        if(s_count >= active_clients) break;
        FD_SET(client_sockets[s_count], &all_sockets);
        if(client_sockets[s_count] > max_fds) max_fds = client_sockets[s_count];
    }
    
    // wait for activity on any sockets, timeout in timeout time
    // nfds, readfds, writefds, exeptfds, timeout
    printf("Starting Select\n");
    activity = select((max_fds + 1), &all_sockets, NULL, NULL, NULL);
    if(activity < 0){
        printf("---- ERROR IN SERVER ---\n");
    }
    printf("Got Something...\n");
    
    // check system sockets
    for(s_count=0;s_count<MAX_SYS_SOCKETS;s_count++){
        if(s_count >= active_system) break;
        if(FD_ISSET(sys_sockets[s_count], &all_sockets)){
            new_socket = accept(sys_sockets[s_count], (struct sockaddr*)&new_addr, (socklen_t*) &new_addr_len);
            if(new_socket < 0){
                printf("---- Failure Getting New Socket ---- \n");
                return STOPPED;
            } else {
                printf("New Connection: FD is %d, ip is %s, port is %d\n", new_socket,
                    inet_ntoa(new_addr.sin_addr), ntohs(new_addr.sin_port));
#ifdef IS_SERVER
                // check for local connection
                if(new_addr.sin_addr.s_addr == ((ms_ip[3]<<24)|(ms_ip[2]<<16)|(ms_ip[1]<<8)|(ms_ip[0]<<0))){
                    printf("Found Local Client\n");
                    if(active_local < MAX_LOCAL_SOCKETS){
                        printf("Added To Local: %d\n", active_local);
                        local_sockets[active_local] = new_socket;
                        active_local = active_local + 1;
                    } else {
                        // Too Many Locals - reject
                        printf("Too Many Locals... Rejected\n");
                        close(new_socket);
                    }
                } else {
                    printf("Didn't Match Local...\n");
                    // must be client connection
                    if(active_clients < MAX_CLIENTS){
                        printf("Added To Client: %d\n", active_clients);
                        client_sockets[active_clients] = new_socket;
                        active_clients = active_clients + 1;
                    } else {
                        // Too Many Clients - reject
                        printf("Too Many Clients... Rejected\n");
                        close(new_socket);
                    }
                }
#endif
#ifdef IS_CLIENT
                // check for local connection
                if(new_addr.sin_addr.s_addr == ((client_ips[0][3]<<24)|(client_ips[0][2]<<16)|(client_ips[0][1]<<8)|(client_ips[0][0]))){
                    printf("Found Local Client\n");
                    if(active_local < MAX_LOCAL_SOCKETS){
                        printf("Added To Local: %d\n", active_local);
                        local_sockets[active_local] = new_socket;
                        active_local = active_local + 1;
                    } else {
                        // Too Many Locals - reject
                        printf("Too Many Locals... Rejected\n");
                        close(new_socket);
                    }
                } else {
                    // must be client connection
                    //if(active_clients < MAX_CLIENTS){
                    //    printf("Added To Client: %d\n", active_clients);
                    //    client_sockets[active_clients] = new_socket;
                    //    active_clients = active_clients + 1;
                    //} else {
                        // Too Many Clients - reject
                        printf("Can't Have Clients... Rejected\n");
                        close(new_socket);
                    //}
                }
#endif
            }
        }
    }
    // check unix sockets
    for(s_count=0;s_count<MAX_UNIX_SOCKETS;s_count++){
        if(s_count >= active_unix) break;
        if(FD_ISSET(unix_sockets[s_count], &all_sockets)){
            printf("Got Data On Unix:%d\n", s_count);
            num_read = read(unix_sockets[s_count], read_data, sizeof(read_data));
            if(num_read){
                // Got Data
                read_data[num_read] = '\0';
                process(s_count, read_data);
            } else {
                // No Data... Closing Connection
                printf("Closing Unix Socket: %d\n", s_count);
                close(unix_sockets[s_count]);
                unix_sockets[s_count] = 0;
            }
        }
    }
    // check local sockets
    for(s_count=0;s_count<MAX_LOCAL_SOCKETS;s_count++){
        if(s_count >= active_local) break;
        if(FD_ISSET(local_sockets[s_count], &all_sockets)){
            printf("Got Data On Local:%d\n", s_count);
            num_read = read(local_sockets[s_count], read_data, sizeof(read_data));
            if(num_read){
                // Got Data
                read_data[num_read] = '\0';
                process(s_count, read_data);
            } else {
                // No Data... Closing Connection
                printf("Closing Local Socket: %d\n", s_count);
                close(local_sockets[s_count]);
                local_sockets[s_count] = 0;
            }
        }
    }
    // check client sockets
    for(s_count=0;s_count<MAX_INET_SOCKETS;s_count++){
        if(s_count >= active_clients) break;
        if(FD_ISSET(client_sockets[s_count], &all_sockets)){
            printf("Got Data On Client:%d\n", s_count);
            num_read = read(client_sockets[s_count], read_data, sizeof(read_data));
            if(num_read){
                // Got Data
                read_data[num_read] = '\0';
                process(s_count, read_data);
            } else {
                // No Data... Closing Connection
                printf("Closing Client Socket: %d\n", s_count);
                close(client_sockets[s_count]);
                client_sockets[s_count] = 0;
            }
        }
    }
    
    // shift down to remove gaps in clients after one closes
    shift = 0;
    for(r_count=0;r_count<active_unix;r_count++){
        if(unix_sockets[r_count] == 0){
            while((unix_sockets[r_count+shift] == 0)){
                if((r_count+shift)<active_unix){
                    shift = shift + 1;
                } else {
                    break;
                }
            }
            unix_sockets[r_count] = unix_sockets[r_count+shift];
            active_unix = active_unix - 1;
        }
    }
    shift = 0;
    for(r_count=0;r_count<active_local;r_count++){
        if(local_sockets[r_count] == 0){
            while((local_sockets[r_count+shift] == 0)){
                if((r_count+shift)<active_local){
                    shift = shift + 1;
                } else {
                    break;
                }
            }
            local_sockets[r_count] = local_sockets[r_count+shift];
            active_local = active_local - 1;
        }
    }
    shift = 0;
    for(r_count=0;r_count<active_clients;r_count++){
        if(client_sockets[r_count] == 0){
            while((client_sockets[r_count+shift] == 0)){
                if((r_count+shift)<active_clients){
                    shift = shift + 1;
                } else {
                    break;
                }
            }
            client_sockets[r_count] = client_sockets[r_count+shift];
            active_clients = active_clients - 1;
        }
    }
    
    return RUNNING;
}

void get_rx(int com_socket){
    char rx_data[400] = {0};
    memset(rx_data, '\0', sizeof(rx_data));
    read(com_socket, rx_data, 400);
    printf("Server Read: %s\n", rx_data);
}

void heartbeat(int hb_socket){
    char system_heartbeat = RUNNING;
    while(system_heartbeat == RUNNING){
        sleep(HB_PERIOD);
        send(hb_socket, sf_heartbeat.string, sizeof(sf_heartbeat.string), 0);
        printf("\n---- Sent Heartbeat Command ----\n");
    }
}

int create_unix_socket(char path[MAX_FILE_STRING]){
    // Create a unix socket as a server
    int com_fd, com_socket, com_len, com_opt = 1;
    struct sockaddr_un com_addr;
    com_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(com_fd == 0){
        printf("Parent Socket Was 0\n");
        return -1;
    }
    /*if(setsockopt(com_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, (char *) &com_opt, sizeof(com_opt))){
        printf("Parent Socket Opt Failed\n");
        return -1;
    }*/
    memset(&com_addr, 0, sizeof(com_addr));
    com_addr.sun_family = AF_UNIX;
    strncpy(com_addr.sun_path, path, sizeof(com_addr.sun_path)-1);
    printf("Doing Bind in Parent\n");
    if(bind(com_fd, (struct sockaddr*) &com_addr, sizeof(com_addr)) < 0){
        printf("Failed in Parent Bind\n");
        return -1;
    }
    printf("Doing Listen in Parent\n");
    if(listen(com_fd, 1) < 0){
        printf("Failed in Parent Listen\n");
        return -1;
    }
    com_len = sizeof(com_addr);
    printf("Doing Accept in Parent\n");
    com_socket = accept(com_fd, (struct sockaddr*) &com_addr, (socklen_t*) &com_len);
    if(com_socket < 0){
        printf("Failed in Parent Accept\n");
        return -1;
    }
    return com_socket;
}

int connect_unix_socket(char path[MAX_FILE_STRING]){
    // connect to unix_socket as client
    int com_fd, com_socket, com_len, com_opt = 1;
    struct sockaddr_un com_addr;
    com_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    memset(&com_addr, 0, sizeof(com_addr));
    com_addr.sun_family = AF_UNIX;
    strncpy(com_addr.sun_path, path, sizeof(com_addr.sun_path)-1);
    printf("Doing Connect in Listener\n");
    connect(com_socket, (struct sockaddr *)&com_addr, sizeof(com_addr));
    return com_socket;
}

int create_main_socket(unsigned int port){
    // Create main socket as a server
    int com_fd, com_socket, com_len, com_opt = 1;
    struct sockaddr_in com_addr;
    com_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(com_fd == 0){
        printf("Main Socket Was 0\n");
        return -1;
    }
    if(setsockopt(com_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, (char *) &com_opt, sizeof(com_opt))){
        printf("Main Socket Opt Failed\n");
        return -1;
    }
    memset(&com_addr, 0, sizeof(com_addr));
    com_addr.sin_family = AF_INET;
    com_addr.sin_addr.s_addr = INADDR_ANY;
    com_addr.sin_port = htons(port);

    printf("Doing Bind in Main\n");
    if(bind(com_fd, (struct sockaddr*) &com_addr, sizeof(com_addr)) < 0){
        printf("Failed in Parent Bind\n");
        return -1;
    }
    
    if(listen(com_fd, 3) < 0){
        printf("Failed in Main Listen\n");
        return -1;
    }

    printf("Finished Listen in Main\n");
    return com_fd;
}

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
    connect(com_socket, (struct sockaddr *)&com_addr, sizeof(com_addr));
    if(com_socket < 0){
        printf("Client Failed Connect\n");
        return -1;
    }
    
    return com_socket;
}

void process(int client, char f_string[MAX_FUNCTION_STRING]){
    printf("Client: %d, Function: %s\n", client, f_string);
}

