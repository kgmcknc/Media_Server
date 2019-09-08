
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "sys_functions.h"
#include <ifaddrs.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void get_this_ip(char* ip)
{
    struct ifaddrs *ifaddr, *ifa;
    int family, s, n;
    char host[NI_MAXHOST];
    char print_data = 0;

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    /* Walk through linked list, maintaining head pointer so we
        can free list later */

    for (ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++) {
        if (ifa->ifa_addr == NULL)
            continue;

        family = ifa->ifa_addr->sa_family;

        /* Display interface name and family (including symbolic
            form of the latter for the common families) */
        if(print_data){
            printf("%-8s %s (%d)\n",
                    ifa->ifa_name,
                    (family == AF_PACKET) ? "AF_PACKET" :
                    (family == AF_INET) ? "AF_INET" :
                    (family == AF_INET6) ? "AF_INET6" : "???",
                    family);
        }
        /* For an AF_INET* interface address, display the address */

        if (family == AF_INET || family == AF_INET6) {
            s = getnameinfo(ifa->ifa_addr,
                    (family == AF_INET) ? sizeof(struct sockaddr_in) :
                                            sizeof(struct sockaddr_in6),
                    host, NI_MAXHOST,
                    NULL, 0, NI_NUMERICHOST);
            if (s != 0) {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }
            if(print_data){
                printf("\t\taddress: <%s>\n", host);
            }
            // see if this looks like valid ip... rough check
            int p=0;
            for(int i=0;i<16;i++){
                if(host[i] == 0){
                    break;
                }
                if(host[i] == '.'){
                    p++;
                }
                if(i>=3){
                    if(p>0){
                        if(strncmp(host, "127", 3) == 0){
                            // don't want local
                        } else {
                            strcpy(ip,host);
                        }
                        break;
                    }
                }
            }

        } else if (family == AF_PACKET && ifa->ifa_data != NULL) {
           /* struct rtnl_link_stats *stats = (struct rtnl_link_stats*) ifa->ifa_data;
            if(print_data){
                printf("\t\ttx_packets = %10u; rx_packets = %10u\n"
                        "\t\ttx_bytes   = %10u; rx_bytes   = %10u\n",
                        stats->tx_packets, stats->rx_packets,
                        stats->tx_bytes, stats->rx_bytes);
            }*/
        }
    }

    freeifaddrs(ifaddr);
    
}

void create_config_file(struct system_config_struct system_config){
    FILE* config_file;
    config_file = fopen(CONFIG_PATH, "ab");
    char this_ip[16] = "0";
    if(config_file != NULL){
        fprintf(config_file, "// Media Server C Code Config File\n");
        fprintf(config_file, "\n");
        fprintf(config_file, "// Double Slashes are comments and lines are ignored\n");
        fprintf(config_file, "// (not sure why, but may be needed)\n");
        fprintf(config_file, "\n");
        fprintf(config_file, "is_server 0\n");
        fprintf(config_file, "\n");
        fprintf(config_file, "device_id -1\n");
        fprintf(config_file, "\n");
        fprintf(config_file, "// server_ip_addr will get set by c code if you are connected to the internet... or you can overwrite it with format xxx.xxx.xxx.xxx if you want..?\n");
        get_this_ip(&this_ip[0]);
        fprintf(config_file, "server_ip_addr %s\n", this_ip);
        fprintf(config_file, "\n");
        fprintf(config_file, "// server_tcp_port will get set by c code to default 28500 (randomly picked) or you can overwrite it if you want\n");
        fprintf(config_file, "server_tcp_port 0\n");
        fprintf(config_file, "\n");
    } else {
        printf("Couldn't Create Config File... Exitting");
        exit(EXIT_FAILURE);
    }
}

void load_config(struct system_config_struct* system_config){
    FILE* config_file;
    char* continue_reading = 0;
    char config_file_data[MAX_CONFIG_LINE_SIZE];
    uint8_t loaded_type = 0, loaded_id = 0, loaded_ip = 0, loaded_port = 0;

    config_file = fopen(CONFIG_PATH, "r+b");
    
    memset(&config_file_data[0], 0, MAX_CONFIG_LINE_SIZE);

    do{
        // read line from file
        continue_reading = fgets(&config_file_data[0], MAX_CONFIG_LINE_SIZE, config_file);
        // process line
        char* comment_pointer;
        char* data_pointer;

        // look for comment "//"
        comment_pointer = strstr(&config_file_data[0], "//");
        
        // look for "is_server";
        data_pointer = strstr(&config_file_data[0], "is_server");
        if(data_pointer){
            if(data_pointer > comment_pointer){
                sscanf(data_pointer, "is_server %d", &system_config->is_server);
                loaded_type = 1;
            }
        }
        // look for "device_id";
        data_pointer = strstr(&config_file_data[0], "device_id");
        if(data_pointer){
            if(data_pointer > comment_pointer){
                sscanf(data_pointer, "device_id %d", &system_config->device_id);
                loaded_id = 1;
            }
        }
        // look for "server_ip_addr";
        data_pointer = strstr(&config_file_data[0], "server_ip_addr");
        if(data_pointer){
            if(data_pointer > comment_pointer){
                char this_ip[16];
                sscanf(data_pointer, "server_ip_addr %s", &this_ip[0]);
                inet_aton(&this_ip[0], (struct in_addr *) &system_config->server_ip_addr);
                loaded_ip = 1;
            }
        }
        // look for "server_tcp_port";
        data_pointer = strstr(&config_file_data[0], "server_tcp_port");
        if(data_pointer){
            if(data_pointer > comment_pointer){
                sscanf(data_pointer, "server_tcp_port %d", &system_config->server_tcp_port);
                loaded_port = 1;
            }
        }

    }while(continue_reading); // read end of file, stop

    // load anything we didn't find from file
    if(!loaded_type){
        system_config->is_server = 0; // default to client if not specified
    }
    if(!loaded_id){
        if(system_config->is_server){
            system_config->device_id = 0; // id is zero for servers
        } else {
            system_config->device_id = -1; // set to -1 to mark that it has not yet been given an id
        }
    }
    if(!loaded_ip){
        char this_ip[16];
        get_this_ip(&this_ip[0]);
        inet_aton(&this_ip[0], (struct in_addr *) &system_config->server_ip_addr);
    }
    if(!loaded_port){
        system_config->server_tcp_port = DEFAULT_TCP_PORT;
    }
    fclose(config_file);
}

void configure_system(struct system_config_struct* system_config){
    #ifdef IS_SERVER
    int tmp_count = 0;
    unsigned int tmp_data = 0;
    unsigned int tmp_data_n = 0;
    #endif
    printf("system_configured\n");
    /*unsigned int tmp_ip[4] = {0};
    unsigned int tmp_port = 0;
    char tmp_name[MAX_NAME_LENGTH] = {0};
    char handled = 0;
    char empty;
    char directory[100] = {0};
    char string_count = 0;
    FILE* dir_fp = 0;
    unsigned int num_clients;
    unsigned int tmp_clients[MAX_CLIENTS][4];

    rewind(config_file);
    user_option = 0;
    
    check_file_size(config_file);
    check_config(config_file);
    initial_config(config_file);
    
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
            if(read_port(config_file, &tmp_port, KMF_COM_PORT)){
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
                    write_port(config_file, &tmp_port, KMF_COM_PORT);
                    send_new_port_to_clients(tmp_port);
                    check_config(config_file);
                } else {
                    printf("\nPort Was Out of Range, Try Setting Again\n");
                }
            }
        }
        if(user_option == 's'){
            handled = 1;
            printf("\nServer IP Should Be Static\n");
            if(read_ip_address(config_file, &tmp_ip[0], KMF_S_IP)){
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
                    write_ip_address(config_file, &tmp_ip[0], KMF_S_IP);
                    send_new_ip_to_clients(&tmp_ip[0]);
                    check_config(config_file);
                } else {
                    printf("\nIP Was Out of Range, Try Setting Again\n");
                }
            }
        }
        #ifdef IS_SERVER
        if(user_option == 'n'){
            handled = 1;
            if(read_name(&tmp_name[0], 0)){
                printf("Current Server Name Is: %s\n", tmp_name);
            } else {
                printf("\nServer Name Is Not Currently Set\n");
            }
            printf("\nSet New Server Name or enter 0 to Go Back\n");
            printf("New Server Name: ");
            scanf("%s%c", tmp_name, &empty);
            if(tmp_name[0] == '0'){
                printf("\nGot 0, Going Back to Menu\n");
            }else{
                printf("\nGot %s For New Server Name\n", tmp_name);
                write_name(&tmp_name[0], 0);
            }
        }
        if(user_option == 'v'){
            handled = 1;
            printf("\nListing Attached Clients\n");
            num_clients = read_clients(config_file, tmp_clients, KMF_CLIENT_COUNT);
            if(num_clients){
                printf("Number of Clients: %d\n", num_clients);
                tmp_count = 0;
                while((tmp_count < num_clients) && (tmp_count < MAX_CLIENTS)){
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
            num_clients = read_clients(config_file, tmp_clients, KMF_CLIENT_COUNT);
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
                        add_client(config_file, &tmp_ip[0], KMF_CLIENT_COUNT);
                    } else {
                        printf("\nIP Was Out of Range, Try Setting Again\n");
                    }
                }
            } else {
                printf("\nToo Many Clients To Add Another\n");
            }
        }
        if(user_option == 'c'){
            handled = 1;
            printf("\nChanging Client's IP\n");
            num_clients = read_clients(config_file, tmp_clients, KMF_CLIENT_COUNT);
            if(num_clients > 0){
                printf("Number of Clients: %d\n", num_clients);
                tmp_count = 0;
                while((tmp_count < num_clients) && (tmp_count < MAX_CLIENTS)){
                    printf("Client %d: %d.%d.%d.%d\n", tmp_count + 1, tmp_clients[tmp_count][0], tmp_clients[tmp_count][1], tmp_clients[tmp_count][2], tmp_clients[tmp_count][3]);
                    tmp_count = tmp_count + 1;
                }
                printf("\nSelect Client To Change, or 0 to Go Back: ");
                scanf("%u%c", &tmp_data, &empty);
                if((tmp_data == 0) || (tmp_data > num_clients)){
                    printf("Cannot Remove Client %d\n", tmp_data);
                } else {
                    printf("New IP for Client: ");
                    scanf("%u.%u.%u.%u%c", &tmp_ip[0], &tmp_ip[1], &tmp_ip[2], &tmp_ip[3], &empty);
                    if((tmp_ip[0] == 0) || (tmp_ip[1] == 0) || (tmp_ip[2] == 0) || (tmp_ip[3] == 0)){
                        printf("\nGot 0, Going Back to Menu\n");
                    }else{
                        printf("\nGot %d.%d.%d.%d For New Client IP\n", tmp_ip[0], tmp_ip[1], tmp_ip[2], tmp_ip[3]);
                        if((tmp_ip[0] > 0) && (tmp_ip[1] > 0) && (tmp_ip[2] > 0) && (tmp_ip[3] > 0)){
                            change_client_ip(config_file, tmp_data, &tmp_ip[0], KMF_CLIENT_COUNT);
                        } else {
                            printf("\nIP Was Out of Range, Try Setting Again\n");
                        }
                    }
                }
            } else {
                printf("\nNo Clients To Change\n");
            }
        }
        if(user_option == 'r'){
            handled = 1;
            printf("\nRemoving Client From System\n");
            num_clients = read_clients(config_file, tmp_clients, KMF_CLIENT_COUNT);
            if(num_clients > 0){
                printf("Number of Clients: %d\n", num_clients);
                tmp_count = 0;
                while((tmp_count < num_clients) && (tmp_count < MAX_CLIENTS)){
                    printf("Client %d: %d.%d.%d.%d\n", tmp_count + 1, tmp_clients[tmp_count][0], tmp_clients[tmp_count][1], tmp_clients[tmp_count][2], tmp_clients[tmp_count][3]);
                    tmp_count = tmp_count + 1;
                }
                printf("\nSelect Client To Remove: ");
                scanf("%u%c", &tmp_data, &empty);
                if((tmp_data == 0) || (tmp_data > num_clients)){
                    printf("Cannot Remove Client %d\n", tmp_data);
                } else {
                    remove_client(config_file, tmp_data, KMF_CLIENT_COUNT);
                }
            } else {
                printf("\nNo Clients To Remove\n");
            }
        }
        if(user_option == 'o'){
            handled = 1;
            printf("\nRe-Order Clients On System\n");
            num_clients = read_clients(config_file, tmp_clients, KMF_CLIENT_COUNT);
            if(num_clients > 0){
                printf("Number of Clients: %d\n", num_clients);
                tmp_count = 0;
                while((tmp_count < num_clients) && (tmp_count < MAX_CLIENTS)){
                    printf("Client %d: %d.%d.%d.%d\n", tmp_count + 1, tmp_clients[tmp_count][0], tmp_clients[tmp_count][1], tmp_clients[tmp_count][2], tmp_clients[tmp_count][3]);
                    tmp_count = tmp_count + 1;
                }
                printf("\nSelect 2 Clients To Re-Order (Switch)\n");
                printf("\nInput Two Numbers Separated By A Single Space. x x: ");
                scanf("%u %u%c", &tmp_data, &tmp_data_n, &empty);
                if((tmp_data > 0) && (tmp_data_n > 0) && (tmp_data <= num_clients) && (tmp_data_n <= num_clients)){
                    reorder_clients(config_file, tmp_data, tmp_data_n, KMF_CLIENT_COUNT);
                }
            } else {
                printf("\nNo Clients To Re-Order\n");
            }
        }
	    if(user_option == 't'){
            handled = 1;
            dir_fp = fopen(MOVIEDIR_PATH, "r");
            if(dir_fp == NULL){
                printf("Couldn't Open Movie Directory File...");
            } else {
                if(fscanf(dir_fp, "%s", &directory[0])){
                    printf("Current Movie Directory Is Set To: %s", &directory[0]);
                } else {
                    printf("\nMovie Directory Is Not Currently Set\n");
                }
                fclose(dir_fp);
                printf("\nSet New Movie Directory from root or enter 0 to Go Back\n");
                printf("New Movie Directory: ");
                scanf("%s%c", &directory[0], &empty);
                if(directory[0] == '0'){
                    printf("\nGot 0, Going Back to Menu\n");
                }else{
                    printf("\nGot %s For New Movie Directory\n", &directory[0]);
                    string_count = strlen(directory);
                    if(directory[string_count - 1] == '/'){
                        directory[string_count - 1] = '\0';
                    }
                    dir_fp = fopen(MOVIEDIR_PATH, "w");
                    if(dir_fp == NULL){
                        printf("Couldn't Open Movie Dir\n");
                    } else {
                        fprintf(dir_fp, "%s", &directory[0]);
                        fclose(dir_fp);
                    }
	            }
            }
        }
	    if(user_option == 'b'){
            handled = 1;
            dir_fp = fopen(MUSICDIR_PATH, "r");
            if(dir_fp == NULL){
                printf("Couldn't Open Music Directory File...");
            } else {
                if(fscanf(dir_fp, "%s", &directory[0])){
                    printf("Current Music Directory Is Set To: %s", &directory[0]);
                } else {
                    printf("\nMusic Directory Is Not Currently Set\n");
                }
                fclose(dir_fp);
                printf("\nSet New Music Directory from root or enter 0 to Go Back\n");
                printf("New Music Directory: ");
                scanf("%s%c", &directory[0], &empty);
                if(directory[0] == '0'){
                    printf("\nGot 0, Going Back to Menu\n");
                }else{
                    printf("\nGot %s For Music Directory\n", &directory[0]);
                    string_count = strlen(directory);
                    if(directory[string_count - 1] == '/'){
                        directory[string_count - 1] = '\0';
                    }
                    dir_fp = fopen(MUSICDIR_PATH, "w");
                    if(dir_fp == NULL){
                        printf("Couldn't Open Music Dir\n");
                    } else {
                        fprintf(dir_fp, "%s", &directory[0]);
                        fclose(dir_fp);
                    }
	            }
            }
        }
        if(user_option == 'l'){
            handled = 1;
            dir_fp = fopen(PLAYLISTDIR_PATH, "r");
            if(dir_fp == NULL){
                printf("Couldn't Open Playlist Directory File...");
            } else {
                if(fscanf(dir_fp, "%s", &directory[0])){
                    printf("Current Playlist Directory Is Set To: %s", &directory[0]);
                } else {
                    printf("\nPlaylist Directory Is Not Currently Set\n");
                }
                fclose(dir_fp);
                printf("\nSet New Playlist Directory from root or enter 0 to Go Back\n");
                printf("New Playlist Directory: ");
                scanf("%s%c", &directory[0], &empty);
                if(directory[0] == '0'){
                    printf("\nGot 0, Going Back to Menu\n");
                }else{
                    printf("\nGot %s For Playlist Directory\n", &directory[0]);
                    string_count = strlen(directory);
                    if(directory[string_count - 1] == '/'){
                        directory[string_count - 1] = '\0';
                    }
                    dir_fp = fopen(PLAYLISTDIR_PATH, "w");
                    if(dir_fp == NULL){
                        printf("Couldn't Open Playlist Dir\n");
                    } else {
                        fprintf(dir_fp, "%s", &directory[0]);
                        fclose(dir_fp);
                    }
	            }
            }
        }
        #endif
        #ifdef IS_CLIENT
        if(user_option == 'n'){
            handled = 1;
            if(read_name(&tmp_name[0], 0)){
                printf("Current Client Name Is: %s\n", tmp_name);
            } else {
                printf("\nClient Name Is Not Currently Set\n");
            }
            printf("\nSet New Client Name or enter 0 to Go Back\n");
            printf("New Client Name: ");
            scanf("%s%c", tmp_name, &empty);
            if(tmp_name[0] == '0'){
                printf("\nGot 0, Going Back to Menu\n");
            }else{
                printf("\nGot %s For New Client Name\n", tmp_name);
                write_name(&tmp_name[0], 0);
            }
        }
        if(user_option == 'i'){
            handled = 1;
            printf("\nSetting Client IP\n");
             if(read_clients(config_file, tmp_clients, KMF_CLIENT_COUNT)){
                printf("Current Clint IP Is Set To: %d.%d.%d.%d", tmp_clients[0][0], tmp_clients[0][1], tmp_clients[0][2], tmp_clients[0][3]);
            } else {
                printf("\nClient IP Is Not Currently Set\n");
            }
             printf("\nSet New Client IP with \"XXX.XXX.XXX.XXX\" or enter 0 to Go Back\n");
             printf("New Client IP: ");
             scanf("%u.%u.%u.%u%c", &tmp_ip[0], &tmp_ip[1], &tmp_ip[2], &tmp_ip[3], &empty);
             if((tmp_ip[0] == 0) || (tmp_ip[1] == 0) || (tmp_ip[2] == 0) || (tmp_ip[3] == 0)){
                 printf("\nGot 0, Going Back to Menu\n");
             }else{
                 printf("\nGot %d.%d.%d.%d For New Client IP\n", tmp_ip[0], tmp_ip[1], tmp_ip[2], tmp_ip[3]);
                 if((tmp_ip[0] > 0) && (tmp_ip[1] > 0) && (tmp_ip[2] > 0) && (tmp_ip[3] > 0)){
                     set_client_ip(config_file, &tmp_ip[0], KMF_CLIENT_COUNT);
                     check_config(config_file);
                 } else {
                     printf("\nIP Was Out of Range, Try Setting Again\n");
                 }
             }
        }
        if(user_option == 'a'){
            handled = 1;
            printf("\nAdding Client To System\n");
            num_clients = read_clients(config_file, tmp_clients, KMF_CLIENT_COUNT);
            if(num_clients){
               send_add_client_to_server(&tmp_clients[0][0]);
            } else {
                printf("\nNo IP Set - Can't Add To System\n");
            }
        }
        if(user_option == 'r'){
            handled = 1;
            printf("\nRemoving Client From Server\n");
            num_clients = read_clients(config_file, tmp_clients, KMF_CLIENT_COUNT);
            if(num_clients > 0){
                printf("Number of Clients: %d\n", num_clients);
                get_client_number(config_file, &client_id[0], KMF_CLIENT_COUNT);
                if(client_id[0] > 0){
                   printf("Sending Remove Client\n");
                   send_rem_client_from_server(client_id[0], &client_ips[0][0]);
                } else {
                   printf("\nNo Client Id...Can't Remove...Connected??\n");
                }
            } else {
                printf("\nNot Added To Server, Can't Remove\n");
            }
        }
        #endif
        if(handled == 0){
            printf("\nUnknown Value! %c\n", user_option);
        }
    }*/
}

void print_config_menu(void){
    printf("\n------------------------------\n");
    printf("\n---Choose An Option To View---\n");
    printf("\n------------------------------\n");
    printf("\nq  Exit System Configuration  \n");
    printf("\np  Communication Port:        \n");
    printf("\ns  Server IP Address:         \n");
    #ifdef IS_SERVER
    printf("\nn  Server Name:               \n");
    printf("\nv  View Attached Clients      \n");
    printf("\na  Add New Client IPs         \n");
    printf("\nc  Change Client IP           \n");
    printf("\nr  Remove Attached Clients    \n");
    printf("\no  Re-Order Attached Clients  \n");
    printf("\nt  Set Movie Directory        \n");
    printf("\nb  Set Music Directory        \n");
    printf("\nl  Set Playlist Directory     \n");
    #endif
    #ifdef IS_CLIENT
    printf("\ni  Set/Change IP Address:     \n");
    printf("\nn  Set/Change Client Name:    \n");
    printf("\na  Add Client To Server       \n");
    printf("\nr  Remove Client From Server  \n");
    #endif
    printf("\n------------------------------\n");
    printf("\nType Input Selection: ");
}

// char read_config_data(FILE* config_file, char* config_data, long int offset, int size){
//     char data[TMP_DATA_SIZE] = {0};
//     int tmp_cnt = 0;
//     char tmp_data = 0;
    
//     if((offset < MAX_CONFIG_FILE) && (offset <= file_length)){
//         if(fseek(config_file, offset, SEEK_SET)) return 0;
//     } else {
//         return 0;
//     }
    
//     while((tmp_cnt < size) && (tmp_cnt < TMP_DATA_SIZE)){
//         tmp_data = getc(config_file);
//         if(tmp_data == EOF) return 0;
//         data[tmp_cnt] = tmp_data;
//         tmp_cnt = tmp_cnt + 1;
//     }
    
//     strncpy(config_data, &data[0], size);
//     return 1;
// }

// char write_config_data(FILE* config_file, char* config_data, long int offset, int size){
//     unsigned int tmp_cnt = 0;
//     char tmp_data = 0;
    
//     if((offset < MAX_CONFIG_FILE) && (offset <= file_length)){
//         if(fseek(config_file, offset, SEEK_SET)) return 0;
//     } else {
//         return 0;
//     }
    
//     while((tmp_cnt < size) && (tmp_cnt < TMP_DATA_SIZE)){
//         tmp_data = config_data[tmp_cnt];
//         fputc(tmp_data, config_file);
//         tmp_cnt = tmp_cnt + 1;
//     }
    
//     return 0;
// }

// char read_port(FILE* config_file, unsigned int* config_data, long int offset){
//     unsigned char tmp_data = 0;
//     unsigned int tmp_int = 0;
    
//     if((offset < MAX_CONFIG_FILE) && (offset <= file_length)){
//         if(fseek(config_file, offset, SEEK_SET)) return 0;
//     } else {
//         return 0;
//     }
    
//     tmp_data = getc(config_file);

//     if(tmp_data == EOF) return 0;
//     tmp_int = tmp_data;
//     tmp_int = tmp_int << 8;
//     tmp_data = getc(config_file);

//     if(tmp_data == EOF) return 0;
//     tmp_int = tmp_int | tmp_data;
//     *config_data = tmp_int;

//     return 1;
// }

// char write_port(FILE* config_file, unsigned int* config_data, long int offset){
//     unsigned char tmp_data = 0;
//     int tmp_int = 0;
    
//     tmp_int = *config_data;
//     printf("got write port: %d\n", tmp_int);
//     if((offset < MAX_CONFIG_FILE) && (offset <= file_length)){
//         if(fseek(config_file, offset, SEEK_SET)) return 0;
//     } else {
//         return 0;
//     }
//     tmp_data = (tmp_int >> 8) & 0xFF;

//     fputc(tmp_data, config_file);
//     tmp_data = tmp_int & 0xFF;

//     fputc(tmp_data, config_file);
    
//     return 0;
// }

// char read_ip_address(FILE* config_file, unsigned int* config_data, long int offset){
//     unsigned char tmp_data = 0;
//     unsigned char tmp_cnt = 0;
    
//     if((offset < MAX_CONFIG_FILE) && (offset <= file_length)){
//         if(fseek(config_file, offset, SEEK_SET)) return 0;
//     } else {
//         return 0;
//     }
    
//     while(tmp_cnt < 4){
//         tmp_data = getc(config_file);
//         if(tmp_data == EOF) return 0;
//         config_data[tmp_cnt] = tmp_data;
//         tmp_cnt = tmp_cnt + 1;
//     }
    
//     return 1;
// }

// char write_ip_address(FILE* config_file, unsigned int* config_data, long int offset){
//     unsigned char tmp_cnt = 0;
//     unsigned char tmp_data = 0;
    
//     if((offset < MAX_CONFIG_FILE) && (offset <= file_length)){
//         if(fseek(config_file, offset, SEEK_SET)) return 0;
//     } else {
//         return 0;
//     }
    
//     while(tmp_cnt < 4){
//         tmp_data = config_data[tmp_cnt];
//         fputc(tmp_data, config_file);
//         tmp_cnt = tmp_cnt + 1;
//     }
    
//     return 0;
// }

// char read_name(char* name_data, long int offset){
//     unsigned char tmp_data = 0;
//     unsigned char tmp_cnt = 0;
//     unsigned char name_length = 0;
//     FILE* name_fp = 0;
    
//     name_fp = fopen(NAME_PATH, "r");
//     if(name_fp == NULL){
//         printf("Couldn't Open Names File...");
//         return 0;
//     } else {
//         while(tmp_cnt < offset){
//             if(feof(name_file)) break;
//             fgets(name_data, MAX_NAME_LENGTH, name_fp);
//             tmp_cnt = tmp_cnt + 1;
//         }
//         if(feof(name_file)) return 0;
//         fgets(name_data, MAX_NAME_LENGTH, name_fp);
        
//         fclose(name_fp);
        
//         return 1;
//     }
// }

// char write_name(char* name_data, long int offset){
//     unsigned char tmp_cnt = 0;
//     unsigned char tmp_data = 0;
//     unsigned char name_length = 0;
//     FILE* name_fp = 0;

//     name_fp = fopen(NAME_PATH, "w");
//     if(name_fp == NULL){
//         printf("Couldn't Open Name File\n");
//         return 0;
//     } else {
//         while(tmp_cnt < offset){
//             fgets(name_data, MAX_NAME_LENGTH, name_fp);
//             tmp_cnt = tmp_cnt + 1;
//         }
        
//         fprintf(name_fp, "%s", name_data);
        
//         fclose(name_fp);
        
//         return 1;
//     }
// }

// char set_client_ip(FILE* config_file, unsigned int* config_data, long int offset){
//     #ifdef IS_CLIENT
//     unsigned char tmp_data = 0;
//     unsigned char tmp_cnt = 0;
//     unsigned char clients = 0;
//     unsigned int tmp_ip[4] = {0};
//     long int new_offset;
//     long int file_size;
    
//     if((offset < MAX_CONFIG_FILE) && (offset <= file_length)){
//         if(fseek(config_file, offset, SEEK_SET)) return 0;
//     } else {
//         return 0;
//     }
    
//     tmp_data = getc(config_file);
//     if(tmp_data == EOF) return 0;
//     clients = clients | tmp_data;
    
//      new_offset = offset + 1;
//      if((new_offset < MAX_CONFIG_FILE) && (new_offset <= file_length)){
//          if(fseek(config_file, new_offset, SEEK_SET)) return 0;
//      } else {
//          return 0;
//      }
    
//     if(clients){
//        tmp_cnt = 0;
//        while(tmp_cnt < 4){
//            tmp_ip[tmp_cnt] = client_ips[0][tmp_cnt];
//            tmp_cnt = tmp_cnt + 1;
//        }
//        send_new_ip_to_server(&tmp_ip[0], config_data);
//     }
    
//     tmp_cnt = 0;
//     while(tmp_cnt < 4){
//         tmp_data = config_data[tmp_cnt];
//         fputc(tmp_data, config_file);
//         tmp_cnt = tmp_cnt + 1;
//     }
    
//     if(!clients){
//        if((offset < MAX_CONFIG_FILE) && (offset <= file_length)){
//            if(fseek(config_file, offset, SEEK_SET)) return 0;
//        } else {
//            return 0;
//        }
       
//        clients = 1;
//        tmp_data = clients & 0xFF;
//        fputc(tmp_data, config_file);
       
//        read_file_size(config_file, &file_size, KMF_FILE_SIZE);
//        file_size = file_size + 5;
//        file_length = file_size;
//        write_file_size(config_file, &file_size, KMF_FILE_SIZE);
//     }
    
//     clients = 1;
//     client_count = clients;
//     tmp_data = (clients << 1);
//     fputc(tmp_data, config_file);
    
//     #endif
//     return 0;
// }

// char set_client_number(FILE* config_file, unsigned int client_number, long int offset){
//    #ifdef IS_CLIENT
//    long int new_offset = 0;
//    new_offset = offset + 1 + 4;
   
//    if((new_offset < MAX_CONFIG_FILE) && (new_offset <= file_length)){
//       if(fseek(config_file, new_offset, SEEK_SET)) return 0;
//       printf("opened KMF to update client number\n");
//     } else {
//       printf("Couldn't open KMF to update client number\n");
//       return 0;
//     }
//     printf("Updated client to number: %u\n", client_number);
//     fputc(client_number, config_file);
    
//    #endif
//    return 0;
// }

// char get_client_number(FILE* config_file, unsigned int* client_number, long int offset){
//    #ifdef IS_CLIENT
//    long int new_offset = 0;
//    new_offset = offset + 1 + 4;
//    char tmp_data = 0;
   
//    if((new_offset < MAX_CONFIG_FILE) && (new_offset <= file_length)){
//       if(fseek(config_file, new_offset, SEEK_SET)) return 0;
//     } else {
//       return 0;
//     }
    
//     tmp_data = fgetc(config_file);
//     *client_number = tmp_data;
    
//    #endif
//    return 0;
// }

// char add_client(FILE* config_file, unsigned int* config_data, long int offset){
//     unsigned char tmp_data = 0;
//     unsigned char tmp_cnt = 0;
//     unsigned char clients = 0;
//     unsigned char loop_count = 0;
//     long int new_offset;
//     long int file_size;
//     unsigned char match = 0;
    
//     if((offset < MAX_CONFIG_FILE) && (offset <= file_length)){
//         if(fseek(config_file, offset, SEEK_SET)) return 0;
//     } else {
//         return 0;
//     }
    
//     tmp_data = getc(config_file);
//     if(tmp_data == EOF) return 0;
//     clients = clients | tmp_data;
    
//     while(loop_count < clients){
//         new_offset = offset + 1 + (5*(loop_count));
//         if((new_offset < MAX_CONFIG_FILE) && (new_offset <= file_length)){
//             if(fseek(config_file, new_offset, SEEK_SET)) return 0;
//         } else {
//             return 0;
//         }
        
//         tmp_cnt = 0;
//         match = 1;
//         while(tmp_cnt < 4){
//             tmp_data = fgetc(config_file);
//             if(tmp_data != config_data[tmp_cnt]){
//                 match = 0;
//             }
//             tmp_cnt = tmp_cnt + 1;
//         }
//         if(match) return 0;
        
//         loop_count = loop_count + 1;
//     }
    
//     new_offset = offset + 1 + (5*(clients));
//     if((new_offset < MAX_CONFIG_FILE) && (new_offset <= file_length)){
//         if(fseek(config_file, new_offset, SEEK_SET)) return 0;
//     } else {
//         return 0;
//     }
    
//     tmp_cnt = 0;
//     while(tmp_cnt < 4){
//         tmp_data = config_data[tmp_cnt];
//         fputc(tmp_data, config_file);
//         tmp_cnt = tmp_cnt + 1;
//     }
//     clients = clients + 1;
//     tmp_data = (clients << 1);
//     fputc(tmp_data, config_file);
//     if((offset < MAX_CONFIG_FILE) && (offset <= file_length)){
//         if(fseek(config_file, offset, SEEK_SET)) return 0;
//     } else {
//         return 0;
//     }
    
//     tmp_data = clients & 0xFF;
//     fputc(tmp_data, config_file);
//     client_count = clients;
//     read_file_size(config_file, &file_size, KMF_FILE_SIZE);
//     file_size = file_size + 5;
//     file_length = file_size;
//     write_file_size(config_file, &file_size, KMF_FILE_SIZE);
    
//     return 0;
// }

// char change_client_ip(FILE* config_file, unsigned int client_num, unsigned int* new_ip, long int offset){
//     unsigned char tmp_data = 0;
//     unsigned char tmp_cnt = 0;
//     unsigned char clients = 0;
//     long int new_offset;
    
//     if((offset < MAX_CONFIG_FILE) && (offset <= file_length)){
//         if(fseek(config_file, offset, SEEK_SET)) return 0;
//     } else {
//         return 0;
//     }
    
//     tmp_data = getc(config_file);
//     if(tmp_data == EOF) return 0;
//     clients = clients | tmp_data;
    
//     new_offset = offset + 1 + (5*(client_num - 1));
//    if((new_offset < MAX_CONFIG_FILE) && (new_offset <= file_length)){
//       if(fseek(config_file, new_offset, SEEK_SET)) return 0;
//    } else {
//       return 0;
//    }
    
//     tmp_cnt = 0;
//     while(tmp_cnt < 4){
//         tmp_data = new_ip[tmp_cnt];
//         fputc(tmp_data, config_file);
//         tmp_cnt = tmp_cnt + 1;
//     }
    
//     return 0;
// }

// char remove_client(FILE* config_file, unsigned int config_data, long int offset){
//     unsigned char tmp_data = 0;
//     unsigned char loop_count = 0;
//     unsigned char clients = 0;
//     unsigned int tmp_ip[4];
//     long int new_offset;
//     long int file_size;
    
//     if((offset < MAX_CONFIG_FILE) && (offset <= file_length)){
//         if(fseek(config_file, offset, SEEK_SET)) return 0;
//     } else {
//         return 0;
//     }
//     //tmp_data = getc(config_file);
//     //if(tmp_data == EOF) return 0;
//     //client_count = tmp_data;
//     //client_count = (client_count << 8);
//     tmp_data = getc(config_file);
//     if(tmp_data == EOF) return 0;
//     clients = clients | tmp_data;
    
//     loop_count = (config_data - 1);
//     while(loop_count < clients){
//         new_offset = offset + 1 + (5*(loop_count+1));
//         if((new_offset < MAX_CONFIG_FILE) && (new_offset <= file_length)){
//             read_ip_address(config_file, &tmp_ip[0], new_offset);
//             write_ip_address(config_file, &tmp_ip[0], (new_offset - 4));
//         } else {
//             return 0;
//         }
//         loop_count = loop_count + 1;
//     }
    
//     clients = clients - 1;
//     if((offset < MAX_CONFIG_FILE) && (offset <= file_length)){
//         if(fseek(config_file, offset, SEEK_SET)) return 0;
//     } else {
//         return 0;
//     }
//     //tmp_data = (client_count >> 8) & 0xFF;
//     //fputc(tmp_data, config_file);
//     tmp_data = clients & 0xFF;
//     fputc(tmp_data, config_file);
//     client_count = clients;
//     read_file_size(config_file, &file_size, KMF_FILE_SIZE);
//     file_size = file_size - 5;
//     file_length = file_size;
//     write_file_size(config_file, &file_size, KMF_FILE_SIZE);
//     ftruncate(fileno(config_file), file_size);
    
//     return 0;
// }

// char reorder_clients(FILE* config_file, unsigned int config_data, unsigned int config_data_n, long int offset){
//     unsigned char tmp_data = 0;
//     unsigned char clients = 0;
//     unsigned int tmp_ip[4];
//     unsigned int tmp_ip_n[4];
//     long int new_offset;
    
//     if((offset < MAX_CONFIG_FILE) && (offset <= file_length)){
//         if(fseek(config_file, offset, SEEK_SET)) return 0;
//     } else {
//         return 0;
//     }
//     //tmp_data = getc(config_file);
//     //if(tmp_data == EOF) return 0;
//     //client_count = tmp_data;
//     //client_count = (client_count << 8);
//     tmp_data = getc(config_file);
//     if(tmp_data == EOF) return 0;
//     clients = clients | tmp_data;
    
//     new_offset = offset + 1 + (5*(config_data - 1));
//     if((new_offset < MAX_CONFIG_FILE) && (new_offset <= file_length)){
//         read_ip_address(config_file, &tmp_ip[0], new_offset);
//     } else {
//         return 0;
//     }
//     new_offset = offset + 1 + (5*(config_data_n - 1));
//     if((new_offset < MAX_CONFIG_FILE) && (new_offset <= file_length)){
//         read_ip_address(config_file, &tmp_ip_n[0], new_offset);
//     } else {
//         return 0;
//     }
//     new_offset = offset + 1 + (5*(config_data - 1));
//     if((new_offset < MAX_CONFIG_FILE) && (new_offset <= file_length)){
//         write_ip_address(config_file, &tmp_ip_n[0], new_offset);
//     } else {
//         return 0;
//     }
//     new_offset = offset + 1 + (5*(config_data_n - 1));
//     if((new_offset < MAX_CONFIG_FILE) && (new_offset <= file_length)){
//         write_ip_address(config_file, &tmp_ip[0], new_offset);
//     } else {
//         return 0;
//     }
    
//     return 0;
// }

// char read_file_size(FILE* config_file, long int* config_data, long int offset){
//     int tmp_data = 0;
//     int tmp_cnt = 4;
//     long int tmp_int= 0;
    
//     if((offset < MAX_CONFIG_FILE) && (offset <= file_length)){
//         if(fseek(config_file, offset, SEEK_SET)) return 0;
//     } else {
//         return 0;
//     }
    
//     while(tmp_cnt){
//         tmp_cnt = tmp_cnt - 1;
//         tmp_data = getc(config_file);
//         if(tmp_data == EOF) return 0;
//         tmp_int = tmp_int | (tmp_data << (8*tmp_cnt));
//     }
    
//     *config_data = tmp_int;
//     return 1;
// }

// unsigned int read_clients(FILE* config_file, unsigned int config_data[][4], long int offset){
//     unsigned char tmp_data = 0;
//     unsigned char tmp_cnt = 0;
//     unsigned char tmp_counter = 0;
//     unsigned char clients = 0;
    
//     if((offset < MAX_CONFIG_FILE) && (offset <= file_length)){
//         if(fseek(config_file, offset, SEEK_SET)) return 0;
//     } else {
//         return 0;
//     }
    
//     //tmp_data = getc(config_file);
//     //if(tmp_data == EOF) return 0;
//     //client_count = tmp_data;
//     //client_count = (client_count << 8);
//     tmp_data = getc(config_file);
//     if(tmp_data == EOF) return 0;
//     clients = clients | tmp_data;
    
//     while((tmp_cnt < clients) && (tmp_cnt < MAX_CLIENTS)){
//         tmp_counter = 0;
//         while(tmp_counter < 4){
//             tmp_data = getc(config_file);
//             if(tmp_data == EOF) return 0;
//             config_data[tmp_cnt][tmp_counter] = tmp_data;
//             tmp_counter = tmp_counter + 1;
//         }
//         tmp_cnt = tmp_cnt + 1;
//         tmp_data = getc(config_file);
//     }
    
//     return clients;
// }

// char check_file_size(FILE* config_file){
//    long int file_size = 0;
//    char cfg_cnt = 0;
//    char file_end = 0;
//    char data = 0;
   
//    rewind(config_file);
   
//    while(!file_end && (cfg_cnt < MAX_CONFIG_FILE)){
//         fgetc(config_file);
//         file_end = feof(config_file);
//         if(!file_end) cfg_cnt = cfg_cnt + 1;
//     }
//     printf("\nConfig File Was %d Bytes Long\n", cfg_cnt);
    
//     if(cfg_cnt < MIN_CONFIG_SIZE){
//         printf("\nConfig File Was Too Short! Adding Length!\n");
//         if(cfg_cnt < MAX_CONFIG_FILE){
//             if(!fseek(config_file, cfg_cnt, SEEK_SET)){
//                 data = 0;
//                 while(cfg_cnt < MIN_CONFIG_SIZE){
//                     fputc(data, config_file);
//                     cfg_cnt = cfg_cnt + 1;
//                 }
//                 printf("\nSet Config File to Minimum Length\n");
//             }
//         }
//     }
//     rewind(config_file);
    
//     printf("\nConfig File Is Now %d Bytes Long\n", cfg_cnt);
//     file_size = cfg_cnt;
//     file_length = cfg_cnt;
//     write_file_size(config_file, &file_size, KMF_FILE_SIZE);
//     return 0;
// }

// char write_file_size(FILE* config_file, long int* config_data, long int offset){
//     char tmp_cnt = 3;
//     char tmp_data = 0;
    
//     if((offset < MAX_CONFIG_FILE) && (offset <= file_length)){
//         if(fseek(config_file, offset, SEEK_SET)) return 0;
//     } else {
//         return 0;
//     }
    
//     while(tmp_cnt){
//         tmp_data = ((*config_data >> (8*tmp_cnt)) & 0xFF);
//         fputc(tmp_data, config_file);
//         tmp_cnt = tmp_cnt - 1;
//     }
//     tmp_data = *config_data & 0xFF;
//     fputc(tmp_data, config_file);
    
//     return 0;
// }

// void web_server_ip_update(char* config_data){
//    unsigned int web_ip[4] = {0};
//    unsigned char tmp_cnt = 0;
//    char bad_ip = 0;
    
//     sscanf(config_data, "%u.%u.%u.%u", &web_ip[0], &web_ip[1], &web_ip[2], &web_ip[3]);
//     while(tmp_cnt < 4){
//         if(config_data[tmp_cnt] == 0) bad_ip = 1;
//         tmp_cnt = tmp_cnt + 1;
//     }
//     if(!bad_ip){
//        write_ip_address(config_file, &web_ip[0], KMF_S_IP);
//        #ifdef IS_SERVER
//          send_new_ip_to_clients(&web_ip[0]);
//        #endif
//        check_config(config_file);
//     }
// }

// void web_port_update(char* config_data){
//     unsigned int web_port = 0;
//     sscanf(config_data, "%u", &web_port);
//      // put scanf in for port
//     if((web_port > 0) && (web_port < MAX_PORT)){
//        write_port(config_file, &web_port, KMF_COM_PORT);
//        #ifdef IS_SERVER
//        send_new_port_to_clients(web_port);
//        #endif
//        check_config(config_file);
//     }
// }

// void web_add_client(char* config_data){
//    #ifdef IS_SERVER
//    unsigned int web_ip[4] = {0};
//    unsigned char tmp_cnt = 0;
//    char bad_ip = 0;
   
//     sscanf(config_data, "%u.%u.%u.%u", &web_ip[0], &web_ip[1], &web_ip[2], &web_ip[3]);
//     while(tmp_cnt < 4){
//         if(web_ip[tmp_cnt] == 0) bad_ip = 1;
//         tmp_cnt = tmp_cnt + 1;
//     }
//     if(!bad_ip){
//        add_client(config_file, &web_ip[0], KMF_CLIENT_COUNT);
//        check_config(config_file);
//     }
//    #endif
//    #ifdef IS_CLIENT
//       if(client_count){
//          send_add_client_to_server(&client_ips[0][0]);
//       } else {
//          printf("Set Client IP Before Adding To Server\n");
//       }
//    #endif
// }

// void web_set_client_ip(char* config_data){
//    #ifdef IS_CLIENT
//    unsigned int web_ip[4] = {0};
//    unsigned char tmp_cnt = 0;
//    char bad_ip = 0;
   
//     sscanf(config_data, "%u.%u.%u.%u", &web_ip[0], &web_ip[1], &web_ip[2], &web_ip[3]);
//     while(tmp_cnt < 4){
//         if(web_ip[tmp_cnt] == 0) bad_ip = 1;
//         tmp_cnt = tmp_cnt + 1;
//     }
//     if(!bad_ip){
//        set_client_ip(config_file, &web_ip[0], KMF_CLIENT_COUNT);
//        check_config(config_file);
//     }
//     #endif
// }

// void web_update_client_ip(char* config_data){
//    #ifdef IS_SERVER
//    unsigned int new_ip[4] = {0};
//    unsigned char tmp_cnt = 0;
//    char bad_ip = 0;
//    unsigned int tmp_num = 0;
   
//     sscanf(config_data, "%u.%u.%u.%u.%u",
//       &tmp_num, &new_ip[0], &new_ip[1], &new_ip[2], &new_ip[3]);
//     while(tmp_cnt < 4){
//         if(new_ip[tmp_cnt] == 0) bad_ip = 1;
//         tmp_cnt = tmp_cnt + 1;
//     }
//     if(!bad_ip){
//        change_client_ip(config_file, tmp_num, &new_ip[0], KMF_CLIENT_COUNT);
//        check_config(config_file);
//     }
//     #endif
// }

// void web_remove_client(char* config_data){
//    unsigned int web_ip[4] = {0};
//    //unsigned char tmp_cnt = 0;
//    unsigned int tmp_num = 0;
//    //unsigned char loop = 0;
//    //char match = 0;
    
//     sscanf(config_data, "%u.%u.%u.%u.%u", &tmp_num, &web_ip[0], &web_ip[1], &web_ip[2], &web_ip[3]);
//     /*while(!match && (loop < client_count)){
//        tmp_cnt = 0;
//        match = 1;
//        while(match && (tmp_cnt < 4)){
//            if(web_ip[tmp_cnt] != client_ips[loop][tmp_cnt]) match = 0;
//            tmp_cnt = tmp_cnt + 1;
//        }
//        loop = loop + 1;
//     }*/
//     //if(match){
//     #ifdef IS_SERVER
//     if((tmp_num > 0) && ((tmp_num - 1) < client_count)){
//        remove_client(config_file, tmp_num/*(loop + 1)*/, KMF_CLIENT_COUNT);
//        check_config(config_file);
//     }
//     #endif
//     #ifdef IS_CLIENT
//       send_rem_client_from_server(tmp_num, &web_ip[0]);
//     #endif
// }

// void update_system(FILE* config_file){
//    // populate global variables for port and ip from config file
//    read_port(config_file, &ms_port, KMF_COM_PORT);
//    read_ip_address(config_file, &ms_ip[0], KMF_S_IP);
//    client_count = read_clients(config_file, client_ips, KMF_CLIENT_COUNT);
// }

// void send_new_ip_to_server(unsigned int* old_ip, unsigned int* new_ip){
//    #ifdef IS_CLIENT
//    char function[MAX_FUNCTION_STRING] = {0};
//    unsigned int tmp_num;
//    get_client_number(config_file, &tmp_num, KMF_CLIENT_COUNT);
//    printf("port: %d\n", ms_port);
//    sprintf(&function[0], "1%%kmfupdclient%u.%u.%u.%u.%u%%",
//       tmp_num, new_ip[0], new_ip[1], new_ip[2], new_ip[3]);
//    //   ms_ip[0], ms_ip[1], ms_ip[2], ms_ip[3], ms_port);
//    //system(&function[0]);
//    if(active_clients){
//       send(client_sockets[0],function,sizeof(function),0);
//    }
//    #endif
// }

// void send_add_client_to_server(unsigned int* new_ip){
//    #ifdef IS_CLIENT
//    char function[MAX_FUNCTION_STRING] = {0};
   
//    sprintf(&function[0], "1%%kmfaddclient%u.%u.%u.%u%%",
//       new_ip[0], new_ip[1], new_ip[2], new_ip[3]);
//    //   ms_ip[0], ms_ip[1], ms_ip[2], ms_ip[3], ms_port);
//    //system(&function[0]);
//    if(active_clients){
//        send(client_sockets[0],function,sizeof(function),0);
//    }
//    #endif
// }

// void send_rem_client_from_server(unsigned int client_id, unsigned int* client_ip){
//    #ifdef IS_CLIENT
//    char function[MAX_FUNCTION_STRING] = {0};
   
//    sprintf(&function[0], "1%%kmfremclient%u.%u.%u.%u.%u%%",
//       client_id, client_ip[0], client_ip[1],
//       client_ip[2], client_ip[3]);
//    //   ms_ip[0], ms_ip[1], ms_ip[2], ms_ip[3], ms_port);
//    //system(&function[0]);
//    if(active_clients){
//       send(client_sockets[0],function,sizeof(function),0);
//    }
//    #endif
// }

// void send_new_port_to_clients(unsigned int new_port){
//    #ifdef IS_SERVER
//    char function[MAX_FUNCTION_STRING] = {0};
//    unsigned int temp_cnt = 0;
   
//    printf("new port: %d, Client Count: %d\n", new_port, client_count);
//    while(temp_cnt < active_clients){
//       sprintf(&function[0], "1%%kmfsetport%u%%",
//          new_port);
//       send(client_sockets[temp_cnt],function,sizeof(function),0);
//       temp_cnt = temp_cnt + 1;
//       printf("Update Port: %s\n", &function[0]);
//       //system(&function[0]);
//    }
//    #endif
// }

// void send_heartbeat_to_clients(void){
//    #ifdef IS_SERVER
//    char function[MAX_FUNCTION_STRING] = {0};
//    unsigned int temp_cnt = 0;
   
//    restart_heartbeat = 1;
   
//    while(temp_cnt < active_clients){
//       sprintf(&function[0], "1%%hello%u:%s%%",
//          (temp_cnt + 1), ms_name);
//       send(client_sockets[temp_cnt],function,sizeof(function),0);
//       temp_cnt = temp_cnt + 1;
//       //printf("Running: %s\n", function);
//       printf("Sending Heartbeat To Client\n");
//       //system(&function[0]);
//    }
//    #endif
// }

// void receive_heartbeat_from_server(char* config_data){
//    unsigned int tmp_num = 0;
//    char tmp_name[MAX_NAME_LENGTH] = {0};
//    printf("got heartbeat from server\n");
//    sscanf(config_data, "%u:%s", &tmp_num, tmp_name);
//    strcpy(ms_name, tmp_name);
//    set_client_number(config_file, tmp_num, KMF_CLIENT_COUNT);
//    send_heartbeat_to_server();
// }

// void receive_heartbeat_from_client(char client, char* config_data){
//    #ifdef IS_SERVER
//    //char function[MAX_FUNCTION_STRING] = {0};
//    //unsigned int temp_cnt = 0;
   
//    printf("got heartbeat from Client\n");
//    strcpy(client_names[client], config_data);
//    /*while(temp_cnt < client_count){
//       sprintf(&function[0], "echo \"1%%hello%%\" | nc -q 0 %u.%u.%u.%u %u",
//          client_ips[temp_cnt][0], client_ips[temp_cnt][1],
//          client_ips[temp_cnt][2], client_ips[temp_cnt][3], ms_port);
//       temp_cnt = temp_cnt + 1;
//    }*/
//    #endif
// }

// void send_heartbeat_to_server(void){
//    #ifdef IS_CLIENT
//    char function[MAX_FUNCTION_STRING] = {0};
//    unsigned int tmp_num = 0;
//    if(active_clients){
//        //printf("Getting Client Number to send to server\n");
//        get_client_number(config_file, &tmp_num, KMF_CLIENT_COUNT);
//        //printf("Got Client Number %u to send to server\n", tmp_num);
//        sprintf(&function[0], "1%%imhere%s%%",
//           client_names[0], tmp_num);
//           //printf("Running: %s\n", function);
//           //system(&function[0]);
//        send(client_sockets[0],function,sizeof(function),0);
//        printf("Sent Heartbeat to Server\n");
//    }
//    #endif
// }

// void send_new_ip_to_clients(unsigned int* new_ip){
//    #ifdef IS_SERVER
//    char function[MAX_FUNCTION_STRING] = {0};
//    unsigned int temp_cnt = 0;
   
//    while(temp_cnt < client_count){
//       sprintf(&function[0], "kmfupdclient%u.%u.%u.%u",
//          new_ip[0], new_ip[1], new_ip[2], new_ip[3]);
//       send(client_sockets[temp_cnt],function,sizeof(function),0);
//       temp_cnt = temp_cnt + 1;
//       //system(&function[0]);
//    }
//    #endif
// }

// void initial_config(FILE* config_file){
//    char cfg_data[TMP_DATA_SIZE] = {0};
   
//    strncpy(&cfg_data[0], "kmf", 3);
//     write_config_data(config_file, &cfg_data[0], KMF_BASE, 3);
    
//     cfg_data[0] = 0;
//     cfg_data[1] = 0;
//     cfg_data[2] = 1;
//     write_config_data(config_file, &cfg_data[0], KMF_VERSION, 3);
    
//     #ifdef IS_SERVER
//       cfg_data[0] = 1;// server is 1, client is 0
//     #endif
//     #ifdef IS_CLIENT
//       cfg_data[0] = 0;// server is 1, client is 0
//     #endif
//     write_config_data(config_file, &cfg_data[0], KMF_SYS_ID, 1);
// }

// void send_to(unsigned char input_string[MAX_INPUT_STRING], unsigned int address[4]){
//    char send_string[MAX_FUNCTION_STRING] = {0};
//    //printf("In Send To: %s, %u.%u.%u.%u\n", input_string, address[0], address[1], address[2], address[3]);
//    //sprintf(send_string, "echo \"1%%%s%%\" | nc -q 0 %u.%u.%u.%u %u", input_string, address[0], address[1], address[2], address[3], ms_port);
//    //system(send_string);
// }

// void set_status(void){
//     int status_count = 0;
//     printf("Setting Status\n");
//     status_file = fopen(STATUS_PATH, "w");
//     if(status_file != NULL){
//         printf("\n\n----- Successfully Opened Status File -----\n\n");
//         fprintf(status_file, "s:%s\n",ms_name);
//         for(status_count=0;status_count<active_clients;status_count++){
//             fprintf(status_file, "c%d:%s\n",status_count,client_names[status_count]);
//         }
//         fclose(status_file);
//     } else {
//         printf("\n\n----- Failed To Open Status File, Exiting -----\n\n");
//     }
// }


