
#include "main.h"
#include "config.h"
#include "connections.h"
#include "http.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <string.h>
#include <fcntl.h>
#include <errno.h>
//#include <sys/stat.h>
#include <sys/types.h>
//#include <sys/wait.h>
 
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
// #include <sys/un.h>
// #include <sys/time.h>
//#include <wiringPi.h>

void heartbeat(struct system_config_struct* system_config){
    char packet_data[MAX_BROADCAST_PACKET];
    config_to_string(system_config, &packet_data[0]);

    while(CONTINUE_HEARTBEAT){
        sleep(HEARTBEAT_PERIOD_SEC);
        send_broadcast_packet(UDP_BROADCAST_PORT, &packet_data[0], UDP_TRANSFER_LENGTH);
        //printf("\n---- Sent Heartbeat Command ----\n");
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
    memset(&conn_addr, 0, sizeof(sockaddr_in));
    conn_addr.sin_family = AF_INET;
    conn_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    conn_addr.sin_port = htons(broadcast_port);

    retval = sendto(conn_fd,packet_data,data_length,0,(sockaddr *)&conn_addr,sizeof(sockaddr_in));
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

    memset(&conn_addr, 0, sizeof(sockaddr_in));
    conn_addr.sin_family = AF_INET;
    conn_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    conn_addr.sin_port = htons(broadcast_port);
    bind(conn_fd,(sockaddr*)&conn_addr, sizeof (sockaddr_in));
    
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

void add_device_to_table(struct system_struct* system, struct system_config_struct* new_device){
    int i;
    uint8_t device_in_table = 0;
    if(system->active_devices.device_count < MAX_ACTIVE_DEVICES){
        for(i=0;i<system->active_devices.device_count;i++){
            //in_addr device_ip;
            //device_ip.s_addr = new_device->server_ip_addr;
            //printf("Count: %d, Device IP: %s\n", active_devices->device_count, inet_ntoa(device_ip));
            if(memcmp(&system->active_devices.device[i].config, new_device, sizeof(system_config_struct)) == 0){
                device_in_table = 1;
                system->active_devices.device[i].is_active = 1;
                system->active_devices.device[i].timeout_set = 0;
            }
        }
        if(device_in_table){
            //printf("Device Already In Table!\n");
        } else {
            init_system_config_struct(&system->active_devices.device[system->active_devices.device_count].config);
            memcpy(&system->active_devices.device[system->active_devices.device_count].config, new_device, sizeof(system_config_struct));
            system->active_devices.device[system->active_devices.device_count].is_active = 1;
            system->active_devices.device[system->active_devices.device_count].timeout_set = 0;
            for(int i=0;i<system->linked_devices.device_count;i++){
                if(system->linked_devices.device[i].device_id == system->active_devices.device[system->active_devices.device_count].config.device_id){
                    system->active_devices.device[system->active_devices.device_count].is_linked = 1;
                    strcpy(&system->active_devices.device[system->active_devices.device_count].config.device_name[0], system->linked_devices.device[i].device_name);
                }
            }
            //in_addr ip_val;
            //ip_val.s_addr = active_devices->device[active_devices->device_count].config.server_ip_addr;
            //printf("Count: %d, new area IP: %s\n", active_devices->device_count, inet_ntoa(ip_val));
            system->active_devices.device_count = system->active_devices.device_count + 1;
            printf("Adding Device To Table!: %d\n", system->active_devices.device_count);
        }
    } else {
        printf("Too Many Active Devices!!!\n");
    }
}

void remove_inactive_devices(struct device_table_struct* active_devices){
    int i;
    uint8_t removed = 0;
    for(i=0;i<active_devices->device_count;i++){
        if(active_devices->device[i].timeout_set && active_devices->device[i].is_active){
            printf("Removing Inactive Client!\n");
            if(active_devices->device[i].is_connected){
                close_device_connection(&active_devices->device[i]);
            } else {
                active_devices->device[i].is_active = 0;
            }
            removed = 1;
            active_devices->device[i].timeout_set = 0;
        }
    }
    // add function (cleanup_device_table) to shift devices to lowest position as they get removed
    if(removed){
        clean_up_table_order(active_devices);
    }
}

void clean_up_local_table(struct local_connection_table_struct* device_table){
    struct local_connection_table_struct device_table_copy;

    memcpy(&device_table_copy, device_table, sizeof(struct local_connection_table_struct));
    init_local_table_struct(device_table);

    for(int i=0;i<device_table_copy.device_count;i++){
        // find a valid position to put into table
        if(device_table_copy.device[i].is_connected){
            device_table->device[device_table->device_count].is_connected = 1;
            device_table->device[device_table->device_count].device_socket = device_table_copy.device[i].device_socket;
            device_table->device_count = device_table->device_count + 1;
        }
    }
    printf("Local Table: %d\n", device_table->device_count);
}

void clean_up_table_order(struct device_table_struct* device_table){
    struct device_table_struct device_table_copy;

    memcpy(&device_table_copy, device_table, sizeof(struct device_table_struct));
    init_device_table_struct(device_table);

    for(int i=0;i<device_table_copy.device_count;i++){
        // find a valid position to put into table
        if(device_table_copy.device[i].is_active || device_table_copy.device[i].is_connected){
            memcpy(&device_table->device[device_table->device_count], &device_table_copy.device[i], sizeof(struct device_info_struct));
            device_table->device_count = device_table->device_count + 1;
        }
    }
    printf("Active/Connected Table: %d\n", device_table->device_count);
}

void clean_up_linked_table(struct linked_device_table_struct* device_table){
    struct linked_device_table_struct device_table_copy;

    memcpy(&device_table_copy, device_table, sizeof(struct linked_device_table_struct));
    init_linked_table_struct(device_table);

    for(int i=0;i<device_table_copy.device_count;i++){
        // find a valid position to put into table
        if(device_table_copy.device[i].is_valid){
            device_table->device[device_table->device_count].is_valid = 1;
            device_table->device[device_table->device_count].device_id = device_table_copy.device[i].device_id;
            strncpy(&device_table->device[device_table->device_count].device_name[0], &device_table_copy.device[i].device_name[0], MAX_DEVICE_NAME);
            device_table->device_count = device_table->device_count + 1;
        }
    }
    printf("Linked Table: %d\n", device_table->device_count);
}

void set_device_timeout_flags(struct device_table_struct* active_devices){
    int i;
    for(i=0;i<active_devices->device_count;i++){
        active_devices->device[i].timeout_set = 1;
    }
}

int create_network_socket(struct system_config_struct* system_config){
    int conn_fd;
    int conn_socket, conn_opt = 1;
    struct sockaddr_in conn_addr;
    
    conn_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(conn_fd == 0){
        printf("Couldn't connect socket\n");
        return 0;
    }

    if(setsockopt(conn_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, (char *) &conn_opt, sizeof(conn_opt))){
        printf("Couldn't Set Socket Opt\n");
        return 0;
    }

    memset(&conn_addr, 0, sizeof(sockaddr_in));
    conn_addr.sin_family = AF_INET;
    conn_addr.sin_addr.s_addr = htonl(INADDR_ANY);//device->config.server_ip_addr;
    conn_addr.sin_port = htons(system_config->server_tcp_port);

    printf("Doing Bind for socket\n");
    if(bind(conn_fd, (sockaddr*) &conn_addr, sizeof(conn_addr)) < 0){
        printf("Failed in Bind\n");
        return 0;
    }
    
    if(listen(conn_fd, LISTEN_DEPTH) < 0){
        printf("Failed in Listen\n");
        return 0;
    }

    printf("Created Socket!\n");
    return conn_fd;
}

void connect_linked_devices(struct system_struct* system){
    int c,a;
    uint8_t connection_failed = 0;
    
    for(a=0;a<system->active_devices.device_count;a++){
        if(system->active_devices.device[a].config.is_server){
            // dont connect server to server
            printf("Active Device Is Server!: %d\n", system->active_devices.device_count);
        } else {
            if(system->active_devices.device[a].is_linked && !system->active_devices.device[a].is_connected){
                if(connect_device(&system->network, &system->config, &system->active_devices.device[a])){
                    system->active_devices.device[a].is_connected = 1;
                } else {
                    system->active_devices.device[a].is_connected = 0;
                    system->active_devices.device[a].is_active = 0;
                    connection_failed = 1;
                }
            }
        }
    }
    if(connection_failed){
        clean_up_table_order(&system->active_devices);
    }
}

// add system config to connect linked devices and connect device and then change the config data of the packet to the system

uint8_t connect_device(struct network_struct* network, struct system_config_struct* system_config, struct device_info_struct* device){
    sockaddr_in conn_addr;
    char packet_data[MAX_PACKET_LENGTH];
    config_to_string(system_config, &packet_data[0]);
    
    device->device_socket = socket(AF_INET, SOCK_STREAM, 0);
    memset(&conn_addr, 0, sizeof(conn_addr));
    conn_addr.sin_family = AF_INET;
    conn_addr.sin_addr.s_addr = device->config.server_ip_addr;
    conn_addr.sin_port = htons(device->config.server_tcp_port);

    if(connect(device->device_socket, (struct sockaddr *) &conn_addr, sizeof(sockaddr_in)) < 0){
        printf("Error connecting device!\n");
        return NOT_CONNECTED;
    } else {
        fcntl(device->device_socket, F_SETFL, O_NONBLOCK);
        send(device->device_socket,packet_data,UDP_TRANSFER_LENGTH,0);
        printf("Connected Device!\n");
        return IS_CONNECTED;
    }
}

void set_new_connections(struct network_struct* network){
    int max_fd_val = 0;

    FD_ZERO(&network->network_set);
    network->max_network_set = network->network_socket_fd;
    FD_SET(network->max_network_set, &network->network_set);
}

void wait_for_new_connections(struct network_struct* network){
    int new_connection;
    printf("Looking for New Connections\n");
    new_connection = select((network->max_network_set + 1), &network->network_set, NULL, NULL, NULL);
    
    if(new_connection < 0){
        printf("---- ERROR IN SERVER ---\n");
    } else {
        if(new_connection == 0){
            printf("Nothing...\n");
        } else {
            printf("New Connection!\n");
        }
    }
}

void receive_connections(struct system_struct* system){
    sockaddr_in new_addr;
    int new_socket = 0;
    char packet_data[MAX_PACKET_LENGTH];
    int32_t read_length;
    socklen_t new_sock_length;
    uint32_t usleep_time;
    uint8_t found_connection = 0;
    uint8_t read_wait_count = 0;
    struct system_config_struct new_config;

    usleep_time = READ_USLEEP_COUNT;
    
    new_socket = accept(system->network.network_socket_fd, (sockaddr*) &new_addr, &new_sock_length);
    if(new_socket < 0){
        printf("Error on receive accept...\n");
    } else {
        fcntl(new_socket, F_SETFL, O_NONBLOCK);
        while(read_wait_count < MAX_CONNECT_READ_WAIT){
            read_length = recv(new_socket, packet_data, MAX_PACKET_LENGTH, MSG_PEEK);
            if(read_length < 0){
                if((errno != EAGAIN) && (errno != EWOULDBLOCK)){
                    printf("Read Error In Receive\n");
                    break;
                }
            } else {
                if(read_length > 0){
                    printf("got a peek: %s\n", packet_data);
                    break;
                }
            }
            printf("Waiting for receive read\n");
            usleep(usleep_time);
            read_wait_count = read_wait_count + 1;
        }
        if(read_length){
            if(strstr(packet_data, "Media_Server System:") > 0){
                // valid packet from server connecting
                read_length = recv(new_socket, packet_data, MAX_PACKET_LENGTH, 0);
                string_to_config(packet_data, &new_config);
                for(int i=0;i<system->active_devices.device_count;i++){
                    //printf("rx addr: %d, %d\n", new_config.server_ip_addr, active_devices->device[i].config.server_ip_addr);
                    if(new_config.server_ip_addr == system->active_devices.device[i].config.server_ip_addr){
                        if(system->config.is_server){
                            // we are server
                            // we should only receive network connections (not in active device table)
                        } else {
                            // we are client
                            if(system->active_devices.device[i].config.is_server){
                                printf("Connecting Device to Server!\n");
                                system->active_devices.device[i].is_connected = 1;
                                system->active_devices.device[i].device_socket = new_socket;
                            } else {
                                printf("Clients Can't Connect To Other Clients\n");
                            }
                        }
                        found_connection = 1;
                    }
                }
                if(!found_connection){
                    printf("Media_Server Device Not In Table... Closing\n");
                    shutdown(new_socket, 2);
                    close(new_socket);
                }
            } else {
                // not valid media server connect packet... maybe http
                if(strstr(packet_data, "HTTP") > 0){
                    // HTTP packet from web
                    system->local_connections.device[system->local_connections.device_count].is_connected = 1;
                    system->local_connections.device[system->local_connections.device_count].device_socket = new_socket;
                    system->local_connections.device_count = system->local_connections.device_count + 1;
                    printf("Local Network Connection!\n");
                } else {
                    printf("Not Valid HTTP close\n");
                    //printf("Packet: %s\n", packet_data);
                    shutdown(new_socket, 2);
                    close(new_socket);
                }
            }
        } else {
            printf("Didn't receive data on connect... close\n");
            shutdown(new_socket, 2);
            close(new_socket);
        }
    }
}

void check_connections(struct system_struct* system){
    int i;
    char packet_data[MAX_PACKET_LENGTH];
    int read_length;
    uint8_t device_closed = 0;

    memset(packet_data, 0, MAX_PACKET_LENGTH);

    for(i=0;i<system->active_devices.device_count;i++){
        if(system->active_devices.device[i].is_connected){
            read_length = read(system->active_devices.device[i].device_socket, packet_data, MAX_PACKET_LENGTH);
            if(read_length < 0){
                if((errno != EAGAIN) && (errno != EWOULDBLOCK)){
                    printf("dev socket read error - closing connection\n");
                    close_device_connection(&system->active_devices.device[i]);
                    device_closed = 1;
                }
            } else {
                if(read_length == 0){
                    printf("closing dev connection\n");
                    close_device_connection(&system->active_devices.device[i]);
                    device_closed = 1;
                } else {
                    printf("ConPack: %s\n", packet_data);
                }
            }
        }
    }
    if(device_closed == 1){
        clean_up_table_order(&system->active_devices);
    }

    device_closed = 0;
    for(i=0;i<system->local_connections.device_count;i++){
        read_length = read(system->local_connections.device[i].device_socket, packet_data, MAX_PACKET_LENGTH);
        if(read_length < 0){
            if((errno == EAGAIN) || (errno == EWOULDBLOCK)){
                printf("net socket read error - closing connection\n");
                system->local_connections.device[i].is_connected = 0;
                shutdown(system->local_connections.device[i].device_socket, 2);
                close(system->local_connections.device[i].device_socket);
                device_closed = 1;
            }
        } else {
            if(read_length == 0){
                printf("closing net connection\n");
                system->local_connections.device[i].is_connected = 0;
                shutdown(system->local_connections.device[i].device_socket, 2);
                close(system->local_connections.device[i].device_socket);
                device_closed = 1;
            } else {
                //printf("NETPACK: %s\n", packet_data);
                handle_http_message(system, packet_data, &system->local_connections.device[i]);
                system->local_connections.device[i].is_connected = 0;
                shutdown(system->local_connections.device[i].device_socket, 2);
                close(system->local_connections.device[i].device_socket);
                device_closed = 1;
            }
        }
    }
    
    if(device_closed == 1){
        clean_up_local_table(&system->local_connections);
    }
    
}

void close_device_connection(struct device_info_struct* device){
    device->is_active = 0;
    device->is_linked = 0;
    device->is_connected = 0;
    shutdown(device->device_socket, 2);
    close(device->device_socket);
}

void link_device_to_server(struct system_struct* system, struct linked_device_struct* new_linked_device){
    uint8_t device_added = 0;
    for(int i=0;i<system->linked_devices.device_count;i++){
        if(system->linked_devices.device[i].device_id == new_linked_device->device_id){
            strcpy(&system->linked_devices.device[i].device_name[0], new_linked_device->device_name);
            device_added = 1;
        }
    }
    if(device_added){
        // device already added
        printf("device_already added\n");
        update_device_in_db(&system->database.conn, new_linked_device);
    } else {
        system->linked_devices.device[system->linked_devices.device_count].device_id = new_linked_device->device_id;
        system->linked_devices.device_count = system->linked_devices.device_count + 1;
        add_device_to_db(&system->database.conn, new_linked_device);
        for(int i=0;i<system->active_devices.device_count;i++){
            if(new_linked_device->device_id == system->active_devices.device[i].config.device_id){
                system->active_devices.device[i].is_linked = 1;
                strcpy(&system->active_devices.device[i].config.device_name[0], new_linked_device->device_name);
                printf("device linked: %d, %s\n", system->linked_devices.device_count, new_linked_device->device_name);
            }
        }
    }
}

void remove_device_from_server(struct system_struct* system, int32_t device_id){
    printf("removing device from server: Linked %d, Count %d\n", system->linked_devices.device_count, system->active_devices.device_count);
    for(int i=0;i<system->linked_devices.device_count;i++){
        if(device_id == system->linked_devices.device[i].device_id){
            system->linked_devices.device[i].is_valid = 0;
            remove_device_from_db(&system->database.conn, &system->linked_devices.device[i]);
            printf("Cleared Linked Flag!\n");
        }
    }
    for(int i=0;i<system->active_devices.device_count;i++){
        if(device_id == system->active_devices.device[i].config.device_id){
            if(system->active_devices.device[i].is_connected){
                close_device_connection(&system->active_devices.device[i]);
                printf("Removed Device!\n");
            }
        }
    }
    clean_up_linked_table(&system->linked_devices);
    clean_up_table_order(&system->active_devices);
}

void init_device_table_struct(struct device_table_struct* device_table){
    device_table->device_count = 0;
    for(int i=0;i<MAX_ACTIVE_DEVICES;i++){
        init_device_info_struct(&device_table->device[i]);
    }
}

void init_local_table_struct(struct local_connection_table_struct* device_table){
    device_table->device_count = 0;
    for(int i=0;i<MAX_ACTIVE_DEVICES;i++){
        device_table->device[i].is_connected = 0;
        device_table->device[i].device_socket = 0;
    }
}

void init_linked_table_struct(struct linked_device_table_struct* device_table){
    device_table->device_count = 0;
    for(int i=0;i<MAX_ACTIVE_DEVICES;i++){
        device_table->device[i].device_id = 0;
        device_table->device[i].is_valid = 0;
        device_table->device[i].device_name[0] = '\0';
    }
}

void init_device_info_struct(struct device_info_struct* device_info){
    init_system_config_struct(&device_info->config);
    device_info->timeout_set = 0;
    device_info->device_socket = 0;
    device_info->is_active = 0;
    device_info->is_connected = 0;
    device_info->is_linked = 0;
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



