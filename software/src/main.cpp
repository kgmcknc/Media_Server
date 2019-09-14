
#include "main.h"
#include "config.h"
#include "version.h"
#include "connections.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
 
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
// #include <sys/un.h>
// #include <sys/time.h>
//#include <wiringPi.h>

#define CONNECT_TEST

pid_t heartbeat_fork;
pid_t device_discovery_fork;
pid_t device_timeout_fork;
pid_t new_connection_fork;

FILE* config_file;

int main(int argc, char **argv) {
    system_config_struct system_config;
    init_system_config_struct(&system_config);
    
    uint8_t reload_configuration = 0;

    printf("\n\n----- Starting Media Server -----\n\n");

    config_file = fopen(CONFIG_PATH, "r+b");
    if(config_file != NULL){
        printf("\n\n----- Successfully Opened Config File -----\n\n");
        fclose(config_file);
    } else {
        printf("\n\n----- Failed To Open Config File      -----\n\n");
        printf("\n\n----- ReWriting Default Config File   -----\n\n");
        create_config_file(system_config);
    }

    if(argc > 1) {
        printf("\n\n----- Checking Input Argument -----\n\n");
        if(strcmp(argv[1], "Config") || strcmp(argv[1], "config")) {
                printf("\n\n----- Running System Config -----\n\n");
                configure_system(&system_config);
        } else {
            printf("\n\n----- Argument wasn't \"Config\"  -----\n\n");
            printf("\n\n----- Exiting Media Server        -----\n\n");
            exit(EXIT_FAILURE);
        }
    }
    
    do{
        printf("\n\n----- Checking Configuration Settings -----\n\n");
        load_config(&system_config);
        if(system_config.is_server){
            printf("\n\n----- Running as Server -----\n\n");
        } else {
            printf("\n\n----- Running as Client -----\n\n");
        }
        // set interrupt for handling program abort or close
        // execute system shutdown on sigterm
        signal(SIGTERM, &safe_shutdown);
        signal(SIGINT, &safe_shutdown);

        // start device discovery broadcasting
        heartbeat_fork = fork();
        if(heartbeat_fork == 0){
            printf("\n---- Inside Heartbeat Child ----\n");
            heartbeat(&system_config);
            printf("\n---- Exitting Heartbeat Child ----\n");
            exit(EXIT_SUCCESS);
        } else {
            printf("\n---- Inside Parent Server ----\n");
            // start server
            reload_configuration = main_process(&system_config);
        }

        // if we get here we broke out of system...
        system_shutdown();
        // we will reconfigure and restart unless it errored or quit
    } while(reload_configuration);
    
    printf("\n---- Exitting Media Server ----\n");
    exit(EXIT_SUCCESS);
}

void system_shutdown(void){
    // kill all child processes
    kill(heartbeat_fork, SIGKILL);
    kill(device_discovery_fork, SIGKILL);
    kill(device_timeout_fork, SIGKILL);
    kill(new_connection_fork, SIGKILL);
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

void safe_shutdown(int sig){
    system_shutdown();
    exit(EXIT_FAILURE);
}

uint8_t main_process(struct system_config_struct* system_config){
    uint8_t reconfigure = 0;
    struct network_struct network;
    struct system_config_struct new_device;
    struct device_table_struct local_devices;
    struct device_table_struct active_devices;
    struct device_table_struct connected_devices;
    struct device_table_struct linked_devices; // gets loaded from database
    int pipefd[2];
    //int data_size;
    uint8_t timeout_set = 0;
    uint8_t new_connection_set = 0;

    clear_system_config_struct(&new_device);

    active_devices.device_count = 0;
    linked_devices.device_count = 0;
    connected_devices.device_count = 0;
    local_devices.device_count = 0;

    #ifdef CONNECT_TEST
    linked_devices.device_count = 1;
    clear_system_config_struct(&linked_devices.device[0].config);
    linked_devices.device[0].is_connected = 0;
    linked_devices.device[0].config.is_server = 0;
    linked_devices.device[0].config.device_id = 261516760;
    linked_devices.device[0].config.major_version = 1;
    linked_devices.device[0].config.minor_version = 1;
    linked_devices.device[0].config.server_tcp_port = 28500;
    #endif
    
    network.network_socket_fd = create_network_socket(system_config);

    while(!reconfigure){
        // we return here after every new broadcast packet we get from a device other than ourselves to restart the broadcast listener
        if (pipe(pipefd) == -1) {
            perror("pipe");
            //exit(EXIT_FAILURE);
        }
        // set pipe to not block on reads
        fcntl(pipefd[0], F_SETFL, O_NONBLOCK);
        device_discovery_fork = fork();
        if(device_discovery_fork == 0){
            while(1){ // search indefinitely
                // inside child listening for broadcast packet
                close(pipefd[0]); // won't read in child
                clear_system_config_struct(&new_device);
                listen_for_devices(system_config, &new_device);
                //int data_size = sizeof(struct system_config_struct);
                //write(pipefd[1], &data_size, sizeof(data_size));
                write(pipefd[1], &new_device, sizeof(struct system_config_struct)); // write new device data into pipe
            }
            exit(EXIT_SUCCESS);
        } else {
            // inside parent waiting for stuff to process
            close(pipefd[1]); // won't write in parent
            while(1){//waitpid(device_discovery_fork, NULL, WNOHANG) == 0
                // check for new broadcast client
                if(read(pipefd[0], &new_device, sizeof(struct system_config_struct)) > 0){//read(pipefd[0], &data_size, sizeof(data_size)) > 0){
                    //read(pipefd[0], &new_device, data_size);
                    // essentially add to a table of all currently available clients
                    //printf("new_ip: %d\n", new_device.server_ip_addr);
                    add_device_to_table(&active_devices, &new_device);
                    // clear struct after saving to tables
                    clear_system_config_struct(&new_device);
                    if(system_config->is_server){
                        // we are server, see if packet is another server (bad) or a client (good)
                        // if client packet, check to see if it's not in available client table
                            // if so, add it to available client table
                        // if client packet and already in table, find location in table and update status bit
                    } else {
                        // we are client, see if packet is another client (bad) or a server (good)
                        // if server packet, check to see if it's not in available server table
                            // if so, add it to available server table
                    }
                    // set flag for new client received
                }
                
                // fork here and do timeout for sleep with timeout count and essentially
                // if a timeout happens it clears all active clients
                // and then next timeout if a client is cleared (hasn't received a broadcast packet), then it's not active anymore
                
                if(timeout_set){
                    if(waitpid(device_timeout_fork, NULL, WNOHANG) != 0){
                        // process client connected table
                        // remove any no longer "active" clients from the table
                        remove_inactive_devices(&active_devices);
                        timeout_set = 0;
                    }
                } else {
                    device_timeout_fork = fork();
                    if(device_timeout_fork == 0){
                        // in child - do sleep and then exit
                        sleep(TIMEOUT_TIME);
                        exit(EXIT_SUCCESS);
                    } else {
                        // in parent
                        // clear all flags for active clients
                        set_device_timeout_flags(&active_devices);
                    }
                    timeout_set = 1;
                }
                if(new_connection_set){
                    if(waitpid(new_connection_fork, NULL, WNOHANG) != 0){
                        receive_connections(&network, system_config, &active_devices, &linked_devices, &connected_devices, &local_devices); // receives server and network connections
                        new_connection_set = 0;
                    }
                } else {
                    set_new_connections(&network);
                    new_connection_fork = fork();
                    if(new_connection_fork == 0){
                        wait_for_new_connections(&network); // receives server and network connections
                        exit(EXIT_SUCCESS);
                    }
                    new_connection_set = 1;
                }
                #ifdef CONNECT_TEST
                linked_devices.device[0].config.server_ip_addr = active_devices.device[0].config.server_ip_addr;
                #endif
                // create array of "connected" clients (reads from database)
                // then if a new client comes in we will check to see if it's in "connected" array
                // if so, then we will make the socket connection
                if(system_config->is_server){
                    connect_linked_devices(&network, system_config, &linked_devices, &active_devices, &connected_devices); // connects to clients
                }
                
                // do all normal socket stuff here
                check_connections(&network, system_config, &active_devices, &linked_devices, &connected_devices, &local_devices); // checks, processes, closes
                
                //kill process if reconfiguring
                if(reconfigure){
                    kill(device_discovery_fork, SIGKILL);
                    break;
                }
            }
            // finished reading...
            close(pipefd[0]);
        }
    }

    close(network.network_socket_fd);

    return reconfigure;
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
    com_addr.sin_port = htons(4000);
    int test = bind(com_fd,(sockaddr*)&com_addr, sizeof (com_addr));
    int list = listen(com_fd, 1024);
    int rx_len = 100;
    char my_message[100];
    printf("receiving get\n");
    message = accept(com_fd, NULL, NULL);
    int retval = recvfrom(message,my_message,rx_len,0,(sockaddr *)&com_addr,&len);
    int error = 0;//errno;
    char response[] = "HTTP/1.1 200 OK\r\nAccept-Ranges: bytes\r\nContent-Length: 44\r\nConnection: close\r\nContent-Type: text/html\r\nX-Pad: avoid browser bug\r\n\r\n<html><body><h1>It works!</h1></body></html>\r\n";
    //char response[] = "HTTP/1.0 404 NOT FOUND\r\nContent-Length: 17\r\nContent-Type: text/html\r\n\r\ntwentyisonetoomanytt\r\n";
    socklen_t sa_len = sizeof(web_addr);
    getsockname(message,(sockaddr *) &web_addr,&sa_len);
    retval = sendto(message,response,strlen(response)+1,0,(sockaddr *)&web_addr,sizeof(web_addr));
    //retval = write(message,response,strlen(response)+1);
    sleep(3);
    close(message);
    close(com_fd);
    printf("status: %d, %d, Message: %s\n", retval, error, my_message);
}
/*
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