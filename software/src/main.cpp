
#include "main.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <signal.h>
#include <string.h>
// #include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
 
// #include <unistd.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <sys/un.h>
// #include <sys/time.h>
//#include <wiringPi.h>

pid_t heartbeat_fork;

FILE* config_file;

int main(int argc, char **argv) {
    system_config_struct system_config = SYS_CONFIG_DEFAULT;
    uint8_t reload_configuration = 0;

    printf("\n\n----- Starting Media Server -----\n\n");
    
    config_file = fopen(CONFIG_PATH, "r+b");
    if(config_file != NULL){
        printf("\n\n----- Successfully Opened Config File -----\n\n");
    } else {
        printf("\n\n----- Failed To Open Config File      -----\n\n");
        printf("\n\n----- ReWriting Default Config File   -----\n\n");
        create_config_file(config_file, system_config);
    }
    rewind(config_file);

    if(argc > 1) {
        printf("\n\n----- Checking Input Argument -----\n\n");
        if(strcmp(argv[1], "Config") || strcmp(argv[1], "config")) {
                printf("\n\n----- Running System Config -----\n\n");
                configure_system(config_file, &system_config);
        } else {
            printf("\n\n----- Argument wasn't \"Config\"  -----\n\n");
            printf("\n\n----- Exiting Media Server        -----\n\n");
            fclose(config_file);
            exit(EXIT_FAILURE);
        }
    }
    
    do{
        printf("\n\n----- Checking Configuration Settings -----\n\n");
        load_config(config_file, &system_config);
        fclose(config_file);
        if(system_config.is_server){
            printf("\n\n----- Running as Server -----\n\n");

            // set interrupt for handling program abort or close
            // execute system shutdown on sigterm
            signal(SIGTERM, &safe_server_shutdown);
            signal(SIGINT, &safe_server_shutdown);

            // start server
            reload_configuration = run_server(system_config);
        } else {
            printf("\n\n----- Running as Client -----\n\n");

            // set interrupt for handling program abort or close
            // execute system shutdown on sigterm
            signal(SIGTERM, &safe_client_shutdown);
            signal(SIGINT, &safe_client_shutdown);

            // start client
            reload_configuration = run_client(system_config);
        }

        // if we get here we broke out of system...
        if(system_config.is_server){
            safe_server_shutdown(0);
        } else {
            safe_client_shutdown(0);
        }
    } while(reload_configuration);
    
    printf("\n---- Exitting Media Server ----\n");
    exit(EXIT_SUCCESS);
}

void safe_server_shutdown(int sig){
    // kill all child processes
    kill(heartbeat_fork, SIGKILL);
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

void safe_client_shutdown(int sig){
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

uint8_t run_server(struct system_config_struct system_config){
    //system_setup();
/*
    heartbeat_fork = fork();
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
    */
    return 0;
}

uint8_t run_client(struct system_config_struct system_config){
    return 0;
}
/*
void send_broadcast_packet(void){
    int com_fd;
    int com_socket, com_len, com_opt = 1;
    struct sockaddr_in com_addr;
    struct sockaddr_in Recv_addr;  
    com_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(com_fd == 0){
        printf("Main Socket Was 0\n");
        //return -1;
    }
    if(setsockopt(com_fd, SOL_SOCKET, SO_BROADCAST, (char *) &com_opt, sizeof(com_opt))){
        printf("Main Socket Opt Failed\n");
        //return -1;
    }
    memset(&com_addr, 0, sizeof(com_addr));
    com_addr.sin_family = AF_INET;
    com_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);//INADDR_ANY;
    //com_addr.sin_addr.s_addr = inet_addr("192.168.255.255");
    com_addr.sin_port = htons(65400);

    char my_message[] = "testudpbroadcast";

    printf("sending message\n");
    int retval = sendto(com_fd,my_message,strlen(my_message)+1,0,(sockaddr *)&com_addr,sizeof(com_addr));
    int error = errno;
    close(com_fd);
    printf("status: %d, %d\n", retval, error);
}

void receive_broadcast_packet(void){
    int com_fd;
    int com_socket, com_len, com_opt = 1;
    struct sockaddr_in com_addr;
    struct sockaddr_in Recv_addr;  
    com_fd = socket(AF_INET, SOCK_DGRAM, 0);
    unsigned int len = sizeof(struct sockaddr_in);
    if(com_fd == 0){
        printf("Main Socket Was 0\n");
        //return -1;
    }
    if(setsockopt(com_fd, SOL_SOCKET, SO_BROADCAST, (char *) &com_opt, sizeof(com_opt))){
        printf("Main Socket Opt Failed\n");
        //return -1;
    }
    memset(&com_addr, 0, sizeof(com_addr));
    com_addr.sin_family = AF_INET;
    com_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);//INADDR_ANY;
    //com_addr.sin_addr.s_addr = inet_addr("192.168.255.255");
    com_addr.sin_port = htons(65400);
    bind(com_fd,(sockaddr*)&com_addr, sizeof (com_addr));
    int rx_len = 100;
    char my_message[100];
    printf("receiving message\n");
    int retval = recvfrom(com_fd,my_message,rx_len,0,(sockaddr *)&com_addr,&len);
    int error = errno;
    close(com_fd);
    printf("status: %d, %d, Message: %s\n", retval, error, my_message);
}

void receive_get_request(void){
    int com_fd, message;
    int com_socket, com_len, com_opt = 1;
    struct sockaddr_in com_addr;
    struct sockaddr_in web_addr;  
    com_fd = socket(AF_INET, SOCK_STREAM, 0);
    unsigned int len = sizeof(struct sockaddr_in);
    if(com_fd <= 0){
        printf("Main Socket Was less or zero\n");
        //return -1;
    }
    //if(setsockopt(com_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, (char *) &com_opt, sizeof(com_opt))){
    //    printf("Main Socket Opt Failed\n");
    //    //return -1;
    //}
    memset(&com_addr, 0, sizeof(com_addr));
    com_addr.sin_family = AF_INET;
    com_addr.sin_addr.s_addr = htonl(INADDR_ANY);//INADDR_ANY;
    //com_addr.sin_addr.s_addr = inet_addr("192.168.1.33");
    com_addr.sin_port = htons(65005);
    int test = bind(com_fd,(sockaddr*)&com_addr, sizeof (com_addr));
    int list = listen(com_fd, 1024);
    int rx_len = 100;
    char my_message[100];
    printf("receiving get\n");
    message = accept(com_fd, NULL, NULL);
    int retval = recvfrom(message,my_message,rx_len,0,(sockaddr *)&com_addr,&len);
    int error = errno;
    //char response[] = "HTTP/1.1 200 OK\r\nAccept-Ranges: bytes\r\nContent-Length: 44\r\nConnection: close\r\nContent-Type: text/html\r\nX-Pad: avoid browser bug\r\n\r\n<html><body><h1>It works!</h1></body></html>\r\n";
    char response[] = "HTTP/1.0 404 NOT FOUND\r\nContent-Length: 17\r\nContent-Type: text/html\r\n\r\ntwentyisonetoomanytt\r\n";
    socklen_t sa_len = sizeof(web_addr);
    getsockname(message,(sockaddr *) &web_addr,&sa_len);
    retval = sendto(message,response,strlen(response)+1,0,(sockaddr *)&web_addr,sizeof(web_addr));
    //retval = write(message,response,strlen(response)+1);
    sleep(3);
    close(message);
    close(com_fd);
    printf("status: %d, %d, Message: %s\n", retval, error, my_message);
}

void server_system(void){
    int heartbeat_socket_s;
    int new_client;
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

    while(system_state == RUNNING){
        printf("In System While\n");
        while(waitpid(-1, NULL, WNOHANG) > 0);
        set_status();
        system_state = socket_handler();
    }
}

void system_setup(void){
    function_setup();
}

void function_setup(void){
    set_function(&sf_heartbeat, "sendheartbeat");
}

void set_function(struct system_function* sf, const char f_string[MAX_FUNCTION_STRING]){
    sprintf(sf->string, "1%%%s%%", f_string);
}

int socket_handler(void){
    int s_count, r_count;
    int activity;
    int new_socket = 0;
    int new_addr_len;
    int num_read;
    int shift;
    char read_data[MAX_FUNCTION_STRING];
    struct sockaddr_in new_addr;
    new_addr_len = sizeof(new_addr);
    
    printf("Active Clients: %d\n", active_clients);
    
#ifdef IS_CLIENT
    if(active_clients == 0){
        new_socket = connect_client_socket(ms_ip, ms_port);
        if(new_socket >= 0){
            client_sockets[active_clients] = new_socket;
            active_clients = active_clients + 1;
        }
        new_socket = 0;
    }
#endif

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
            if(num_read > 0){
                // Got Data
                read_data[num_read] = '\0';
                process(0, s_count, read_data);
            } else {
                if(num_read == 0){
                    // No Data... Closing Connection
                    printf("Closing Unix Socket: %d\n", s_count);
                    close(unix_sockets[s_count]);
                    unix_sockets[s_count] = 0;
                } else {
                    // Error... Closing Connection
                    printf("Error On Unix Socket: %d\n", s_count);
                    close(unix_sockets[s_count]);
                    unix_sockets[s_count] = 0;
                }
            }
        }
    }
    // check local sockets
    for(s_count=0;s_count<MAX_LOCAL_SOCKETS;s_count++){
        if(s_count >= active_local) break;
        if(FD_ISSET(local_sockets[s_count], &all_sockets)){
            printf("Got Data On Local:%d\n", s_count);
            num_read = read(local_sockets[s_count], read_data, sizeof(read_data));
            if(num_read > 0){
                // Got Data
                read_data[num_read] = '\0';
                process(1, s_count, read_data);
            } else {
                if(num_read == 0){
                    // No Data... Closing Connection
                    printf("Closing Local Socket: %d\n", s_count);
                    close(local_sockets[s_count]);
                    local_sockets[s_count] = 0;
                } else {
                    // Error... Closing Connection
                    printf("Error On Local Socket: %d\n", s_count);
                    close(local_sockets[s_count]);
                    local_sockets[s_count] = 0;
                }
            }
        }
    }
    // check client sockets
    for(s_count=0;s_count<MAX_INET_SOCKETS;s_count++){
        if(s_count >= active_clients) break;
        if(FD_ISSET(client_sockets[s_count], &all_sockets)){
            printf("Got Data On Client:%d\n", s_count);
            num_read = read(client_sockets[s_count], read_data, sizeof(read_data));
            if(num_read > 0){
                // Got Data
                read_data[num_read] = '\0';
                process(2, s_count, read_data);
            } else {
                if(num_read == 0){
                    // No Data... Closing Connection
                    printf("Closing Client Socket: %d\n", s_count);
                    close(client_sockets[s_count]);
                    client_sockets[s_count] = 0;
                } else {
                    // Error... Closing Connection
                    printf("Error On Client Socket: %d\n", s_count);
                    close(client_sockets[s_count]);
                    client_sockets[s_count] = 0;
                }
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

int create_unix_socket(const char path[MAX_FILE_STRING]){
    // Create a unix socket as a server
    int com_fd, com_socket, com_len, com_opt = 1;
    struct sockaddr_un com_addr;
    com_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(com_fd == 0){
        printf("Parent Socket Was 0\n");
        return -1;
    }
    //if(setsockopt(com_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, (char *) &com_opt, sizeof(com_opt))){
    //    printf("Parent Socket Opt Failed\n");
    //    return -1;
    //}
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

int connect_unix_socket(const char path[MAX_FILE_STRING]){
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
    
    if((connect(com_socket, (struct sockaddr *)&com_addr, sizeof(com_addr))) < 0){
        printf("Client Failed Connect\n");
        return -1;
    }
    
    return com_socket;
}

void process(char type, char client, char f_string[MAX_FUNCTION_STRING]){
    printf("Processing: %s\n", f_string);
    check_function(client, f_string);
}

*/