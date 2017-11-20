/*
 * sys_config.c
 * 
 * Copyright 2017 kyle <kyle@linux-main>
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

#include "sys_control.h"
#include "sys_config.h"
#include "sys_functions.h"
#include "/usr/share/media_server/sys_control/configuration.h"

char check_config(FILE* config_file){
    char cfg_data[TMP_DATA_SIZE] = {0}; // max temp data - 40 bytes
    int cfg_cnt = 0;
    char file_end = 0;
    unsigned int port = 0;
    unsigned int ip[4] = {0};
    long int size = 0;
    char tmp_valid = 1;
    
    rewind(config_file);
    while(!file_end && (cfg_cnt < MAX_CONFIG_FILE)){
        fgetc(config_file);
        file_end = feof(config_file);
        cfg_cnt = cfg_cnt + 1;
    }
    file_length = cfg_cnt;
    rewind(config_file);
    
    if(cfg_cnt < MIN_CONFIG_SIZE){
        printf("\nConfig File Was Too Short! Need to Re-Configure!\n");
        check_file_size(config_file);
        initial_config(config_file);
        valid_config = 0;
        return 1;
    } else {
        if(!read_file_size(config_file, &size, KMF_FILE_SIZE)){
           printf("Failed File Size\n");
        }
        if(size){
           file_length = size;
        } else {
           printf("File Size: %u\n", ((int) size));
           tmp_valid = 0;
           //return 1;
        }        
        if(!read_config_data(config_file, &cfg_data[0], KMF_BASE, 3)){
           printf("config data: %s\n", cfg_data);
           tmp_valid = 0;
           //return 1;
        }
        if(strcmp(&cfg_data[0], "kmf")){
           printf("failed on kmf\n");
           //return 1;//printf("config data: %s\n", cfg_data);
           tmp_valid = 0;
        }
        if(!read_port(config_file, &port, KMF_COM_PORT)){
           printf("config port: %d\n", port);
           //return 1;
           tmp_valid = 0;
        }
        if(!port){
           printf("failed on port\n");
           //return 1;
           tmp_valid = 0;
        }
        if(!read_ip_address(config_file, &ip[0], KMF_S_IP)){
           printf("ip: %u.%u.%u.%u\n", ip[0], ip[1], ip[2], ip[3]);
           //return 1;
           tmp_valid = 0;
        }
        if(!ip[0] || !ip[1] || !ip[2] || !ip[3]){
           printf("ip: %u.%u.%u.%u\n", ip[0], ip[1], ip[2], ip[3]);
           printf("failed on ip\n");
           tmp_valid = 0;
           //return 1;
        }
        
        valid_config = tmp_valid;
        if(valid_config){
           update_system(config_file);
           restart_listener = 1;
           return 0;
        } else {
           restart_listener = 0;
           return 1;
        }
    }
}

void configure_system(void){
    #ifdef IS_SERVER
    int tmp_count = 0;
    unsigned int tmp_data = 0;
    unsigned int tmp_data_n = 0;
    #endif
    unsigned int tmp_ip[4] = {0};
    unsigned int tmp_port = 0;
    char handled = 0;
    char empty;
    unsigned int num_clients;
    unsigned int tmp_clients[MAX_CLIENTS][4];

    rewind(config_file);
    user_option = 0;
    
    check_file_size(config_file);
    
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
        #endif
        #ifdef IS_CLIENT
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
        #endif
        if(handled == 0){
            printf("\nUnknown Value! %c\n", user_option);
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
    #ifdef IS_SERVER
    printf("\nv  View Attached Clients      \n");
    printf("\na  Add New Client IPs         \n");
    printf("\nc  Change Client IP           \n");
    printf("\nr  Remove Attached Clients    \n");
    printf("\no  Re-Order Attached Clients  \n");
    #endif
    #ifdef IS_CLIENT
    printf("\ni  Set/Change IP Address:     \n");
    printf("\na  Add Client To Server       \n");
    #endif
    printf("\n------------------------------\n");
    printf("\nType Input Selection: ");
}

char read_config_data(FILE* config_file, char* config_data, long int offset, int size){
    char data[TMP_DATA_SIZE] = {0};
    int tmp_cnt = 0;
    char tmp_data = 0;
    
    if((offset < MAX_CONFIG_FILE) && (offset <= file_length)){
        if(fseek(config_file, offset, SEEK_SET)) return 0;
    } else {
        return 0;
    }
    
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
    
    if((offset < MAX_CONFIG_FILE) && (offset <= file_length)){
        if(fseek(config_file, offset, SEEK_SET)) return 0;
    } else {
        return 0;
    }
    
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

char write_port(FILE* config_file, unsigned int* config_data, long int offset){
    unsigned char tmp_data = 0;
    int tmp_int = 0;
    
    tmp_int = *config_data;
    printf("got write port: %d\n", tmp_int);
    if((offset < MAX_CONFIG_FILE) && (offset <= file_length)){
        if(fseek(config_file, offset, SEEK_SET)) return 0;
    } else {
        return 0;
    }
    tmp_data = (tmp_int >> 8) & 0xFF;

    fputc(tmp_data, config_file);
    tmp_data = tmp_int & 0xFF;

    fputc(tmp_data, config_file);
    
    return 0;
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

char write_ip_address(FILE* config_file, unsigned int* config_data, long int offset){
    unsigned char tmp_cnt = 0;
    unsigned char tmp_data = 0;
    
    if((offset < MAX_CONFIG_FILE) && (offset <= file_length)){
        if(fseek(config_file, offset, SEEK_SET)) return 0;
    } else {
        return 0;
    }
    
    while(tmp_cnt < 4){
        tmp_data = config_data[tmp_cnt];
        fputc(tmp_data, config_file);
        tmp_cnt = tmp_cnt + 1;
    }
    
    return 0;
}

char set_client_ip(FILE* config_file, unsigned int* config_data, long int offset){
    #ifdef IS_CLIENT
    unsigned char tmp_data = 0;
    unsigned char tmp_cnt = 0;
    unsigned char client_count = 0;
    unsigned int tmp_ip[4] = {0};
    long int new_offset;
    long int file_size;
    
    if((offset < MAX_CONFIG_FILE) && (offset <= file_length)){
        if(fseek(config_file, offset, SEEK_SET)) return 0;
    } else {
        return 0;
    }
    
    tmp_data = getc(config_file);
    if(tmp_data == EOF) return 0;
    client_count = client_count | tmp_data;
    
     new_offset = offset + 1;
     if((new_offset < MAX_CONFIG_FILE) && (new_offset <= file_length)){
         if(fseek(config_file, new_offset, SEEK_SET)) return 0;
     } else {
         return 0;
     }
    
    if(client_count){
       tmp_cnt = 0;
       while(tmp_cnt < 4){
           tmp_ip[tmp_cnt] = client_ips[0][tmp_cnt];
           tmp_cnt = tmp_cnt + 1;
       }
       send_new_ip_to_server(&tmp_ip[0], config_data);
    }
    
    tmp_cnt = 0;
    while(tmp_cnt < 4){
        tmp_data = config_data[tmp_cnt];
        fputc(tmp_data, config_file);
        tmp_cnt = tmp_cnt + 1;
    }
    
    if(!client_count){
       if((offset < MAX_CONFIG_FILE) && (offset <= file_length)){
           if(fseek(config_file, offset, SEEK_SET)) return 0;
       } else {
           return 0;
       }
       
       client_count = 1;
       tmp_data = client_count & 0xFF;
       fputc(tmp_data, config_file);
       
       read_file_size(config_file, &file_size, KMF_FILE_SIZE);
       file_size = file_size + 5;
       file_length = file_size;
       write_file_size(config_file, &file_size, KMF_FILE_SIZE);
    }
    
    client_count = 1;
    tmp_data = (client_count << 1);
    fputc(tmp_data, config_file);
    
    #endif
    return 0;
}

char set_client_number(FILE* config_file, unsigned int client_number, long int offset){
   #ifdef IS_CLIENT
   long int new_offset = 0;
   new_offset = offset + 4;
   
   if((new_offset < MAX_CONFIG_FILE) && (new_offset <= file_length)){
      if(fseek(config_file, new_offset, SEEK_SET)) return 0;
    } else {
      return 0;
    }
    
    fputc(client_number, config_file);
    
   #endif
   return 0;
}

char get_client_number(FILE* config_file, unsigned int* client_number, long int offset){
   #ifdef IS_CLIENT
   long int new_offset = 0;
   new_offset = offset + 4;
   char tmp_data = 0;
   
   if((new_offset < MAX_CONFIG_FILE) && (new_offset <= file_length)){
      if(fseek(config_file, new_offset, SEEK_SET)) return 0;
    } else {
      return 0;
    }
    
    tmp_data = fgetc(config_file);
    *client_number = tmp_data;
    
   #endif
   return 0;
}

char add_client(FILE* config_file, unsigned int* config_data, long int offset){
    unsigned char tmp_data = 0;
    unsigned char tmp_cnt = 0;
    unsigned char client_count = 0;
    unsigned char loop_count = 0;
    long int new_offset;
    long int file_size;
    unsigned char match = 0;
    
    if((offset < MAX_CONFIG_FILE) && (offset <= file_length)){
        if(fseek(config_file, offset, SEEK_SET)) return 0;
    } else {
        return 0;
    }
    
    tmp_data = getc(config_file);
    if(tmp_data == EOF) return 0;
    client_count = client_count | tmp_data;
    
    while(loop_count < client_count){
        new_offset = offset + 1 + (5*(loop_count));
        if((new_offset < MAX_CONFIG_FILE) && (new_offset <= file_length)){
            if(fseek(config_file, new_offset, SEEK_SET)) return 0;
        } else {
            return 0;
        }
        
        tmp_cnt = 0;
        match = 1;
        while(tmp_cnt < 4){
            tmp_data = fgetc(config_file);
            if(tmp_data != config_data[tmp_cnt]){
                match = 0;
            }
            tmp_cnt = tmp_cnt + 1;
        }
        if(match) return 0;
        
        loop_count = loop_count + 1;
    }
    
    new_offset = offset + 1 + (5*(client_count));
    if((new_offset < MAX_CONFIG_FILE) && (new_offset <= file_length)){
        if(fseek(config_file, new_offset, SEEK_SET)) return 0;
    } else {
        return 0;
    }
    
    tmp_cnt = 0;
    while(tmp_cnt < 4){
        tmp_data = config_data[tmp_cnt];
        fputc(tmp_data, config_file);
        tmp_cnt = tmp_cnt + 1;
    }
    client_count = client_count + 1;
    tmp_data = (client_count << 1);
    fputc(tmp_data, config_file);
    if((offset < MAX_CONFIG_FILE) && (offset <= file_length)){
        if(fseek(config_file, offset, SEEK_SET)) return 0;
    } else {
        return 0;
    }
    
    tmp_data = client_count & 0xFF;
    fputc(tmp_data, config_file);
    
    read_file_size(config_file, &file_size, KMF_FILE_SIZE);
    file_size = file_size + 5;
    file_length = file_size;
    write_file_size(config_file, &file_size, KMF_FILE_SIZE);
    
    return 0;
}

char change_client_ip(FILE* config_file, unsigned int client_num, unsigned int* new_ip, long int offset){
    unsigned char tmp_data = 0;
    unsigned char tmp_cnt = 0;
    unsigned char client_count = 0;
    long int new_offset;
    
    if((offset < MAX_CONFIG_FILE) && (offset <= file_length)){
        if(fseek(config_file, offset, SEEK_SET)) return 0;
    } else {
        return 0;
    }
    
    tmp_data = getc(config_file);
    if(tmp_data == EOF) return 0;
    client_count = client_count | tmp_data;
    
    new_offset = offset + 1 + (5*(client_num - 1));
	if((new_offset < MAX_CONFIG_FILE) && (new_offset <= file_length)){
		if(fseek(config_file, new_offset, SEEK_SET)) return 0;
	} else {
		return 0;
	}
    
    tmp_cnt = 0;
    while(tmp_cnt < 4){
        tmp_data = new_ip[tmp_cnt];
        fputc(tmp_data, config_file);
        tmp_cnt = tmp_cnt + 1;
    }
    
    return 0;
}

char remove_client(FILE* config_file, unsigned int config_data, long int offset){
    unsigned char tmp_data = 0;
    unsigned char loop_count = 0;
    unsigned char client_count = 0;
    unsigned int tmp_ip[4];
    long int new_offset;
    long int file_size;
    
    if((offset < MAX_CONFIG_FILE) && (offset <= file_length)){
        if(fseek(config_file, offset, SEEK_SET)) return 0;
    } else {
        return 0;
    }
    //tmp_data = getc(config_file);
    //if(tmp_data == EOF) return 0;
    //client_count = tmp_data;
    //client_count = (client_count << 8);
    tmp_data = getc(config_file);
    if(tmp_data == EOF) return 0;
    client_count = client_count | tmp_data;
    
    loop_count = (config_data - 1);
    while(loop_count < client_count){
        new_offset = offset + 1 + (5*(loop_count+1));
        if((new_offset < MAX_CONFIG_FILE) && (new_offset <= file_length)){
            read_ip_address(config_file, &tmp_ip[0], new_offset);
            write_ip_address(config_file, &tmp_ip[0], (new_offset - 4));
        } else {
            return 0;
        }
        loop_count = loop_count + 1;
    }
    
    client_count = client_count - 1;
    if((offset < MAX_CONFIG_FILE) && (offset <= file_length)){
        if(fseek(config_file, offset, SEEK_SET)) return 0;
    } else {
        return 0;
    }
    //tmp_data = (client_count >> 8) & 0xFF;
    //fputc(tmp_data, config_file);
    tmp_data = client_count & 0xFF;
    fputc(tmp_data, config_file);
    
    read_file_size(config_file, &file_size, KMF_FILE_SIZE);
    file_size = file_size - 5;
    file_length = file_size;
    write_file_size(config_file, &file_size, KMF_FILE_SIZE);
    ftruncate(fileno(config_file), file_size);
    
    return 0;
}

char reorder_clients(FILE* config_file, unsigned int config_data, unsigned int config_data_n, long int offset){
    unsigned char tmp_data = 0;
    unsigned char client_count = 0;
    unsigned int tmp_ip[4];
    unsigned int tmp_ip_n[4];
    long int new_offset;
    
    if((offset < MAX_CONFIG_FILE) && (offset <= file_length)){
        if(fseek(config_file, offset, SEEK_SET)) return 0;
    } else {
        return 0;
    }
    //tmp_data = getc(config_file);
    //if(tmp_data == EOF) return 0;
    //client_count = tmp_data;
    //client_count = (client_count << 8);
    tmp_data = getc(config_file);
    if(tmp_data == EOF) return 0;
    client_count = client_count | tmp_data;
    
    new_offset = offset + 1 + (5*(config_data - 1));
    if((new_offset < MAX_CONFIG_FILE) && (new_offset <= file_length)){
        read_ip_address(config_file, &tmp_ip[0], new_offset);
    } else {
        return 0;
    }
    new_offset = offset + 1 + (5*(config_data_n - 1));
    if((new_offset < MAX_CONFIG_FILE) && (new_offset <= file_length)){
        read_ip_address(config_file, &tmp_ip_n[0], new_offset);
    } else {
        return 0;
    }
    new_offset = offset + 1 + (5*(config_data - 1));
    if((new_offset < MAX_CONFIG_FILE) && (new_offset <= file_length)){
        write_ip_address(config_file, &tmp_ip_n[0], new_offset);
    } else {
        return 0;
    }
    new_offset = offset + 1 + (5*(config_data_n - 1));
    if((new_offset < MAX_CONFIG_FILE) && (new_offset <= file_length)){
        write_ip_address(config_file, &tmp_ip[0], new_offset);
    } else {
        return 0;
    }
    
    return 0;
}

char read_file_size(FILE* config_file, long int* config_data, long int offset){
    int tmp_data = 0;
    int tmp_cnt = 4;
    long int tmp_int= 0;
    
    if((offset < MAX_CONFIG_FILE) && (offset <= file_length)){
        if(fseek(config_file, offset, SEEK_SET)) return 0;
    } else {
        return 0;
    }
    
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
    
    if((offset < MAX_CONFIG_FILE) && (offset <= file_length)){
        if(fseek(config_file, offset, SEEK_SET)) return 0;
    } else {
        return 0;
    }
    
    //tmp_data = getc(config_file);
    //if(tmp_data == EOF) return 0;
    //client_count = tmp_data;
    //client_count = (client_count << 8);
    tmp_data = getc(config_file);
    if(tmp_data == EOF) return 0;
    client_count = client_count | tmp_data;
    
    while((tmp_cnt < client_count) && (tmp_cnt < MAX_CLIENTS)){
        tmp_counter = 0;
        while(tmp_counter < 4){
            tmp_data = getc(config_file);
            if(tmp_data == EOF) return 0;
            config_data[tmp_cnt][tmp_counter] = tmp_data;
            tmp_counter = tmp_counter + 1;
        }
        tmp_cnt = tmp_cnt + 1;
        tmp_data = getc(config_file);
    }
    
    return client_count;
}

char check_file_size(FILE* config_file){
   long int file_size = 0;
   char cfg_cnt = 0;
   char file_end = 0;
   char data = 0;
   
   rewind(config_file);
   
   while(!file_end && (cfg_cnt < MAX_CONFIG_FILE)){
        fgetc(config_file);
        file_end = feof(config_file);
        if(!file_end) cfg_cnt = cfg_cnt + 1;
    }
    printf("\nConfig File Was %d Bytes Long\n", cfg_cnt);
    
    if(cfg_cnt < MIN_CONFIG_SIZE){
        printf("\nConfig File Was Too Short! Adding Length!\n");
        if(cfg_cnt < MAX_CONFIG_FILE){
            if(!fseek(config_file, cfg_cnt, SEEK_SET)){
                data = 0;
                while(cfg_cnt < MIN_CONFIG_SIZE){
                    fputc(data, config_file);
                    cfg_cnt = cfg_cnt + 1;
                }
                printf("\nSet Config File to Minimum Length\n");
            }
        }
    }
    rewind(config_file);
    
    printf("\nConfig File Is Now %d Bytes Long\n", cfg_cnt);
    file_size = cfg_cnt;
    file_length = cfg_cnt;
    write_file_size(config_file, &file_size, KMF_FILE_SIZE);
    return 0;
}

char write_file_size(FILE* config_file, long int* config_data, long int offset){
    char tmp_cnt = 3;
    char tmp_data = 0;
    
    if((offset < MAX_CONFIG_FILE) && (offset <= file_length)){
        if(fseek(config_file, offset, SEEK_SET)) return 0;
    } else {
        return 0;
    }
    
    while(tmp_cnt){
        tmp_data = ((*config_data >> (8*tmp_cnt)) & 0xFF);
        fputc(tmp_data, config_file);
        tmp_cnt = tmp_cnt - 1;
    }
    tmp_data = *config_data & 0xFF;
    fputc(tmp_data, config_file);
    
    return 0;
}

void web_server_ip_update(char* config_data){
   unsigned int web_ip[4] = {0};
   unsigned char tmp_cnt = 0;
   char bad_ip = 0;
    
    sscanf(config_data, "%u.%u.%u.%u", &web_ip[0], &web_ip[1], &web_ip[2], &web_ip[3]);
    while(tmp_cnt < 4){
        if(config_data[tmp_cnt] == 0) bad_ip = 1;
        tmp_cnt = tmp_cnt + 1;
    }
    if(!bad_ip){
       write_ip_address(config_file, &web_ip[0], KMF_S_IP);
       #ifdef IS_SERVER
         send_new_ip_to_clients(&web_ip[0]);
       #endif
       check_config(config_file);
    }
}

void web_port_update(char* config_data){
   #ifdef IS_SERVER
    unsigned int web_port = 0;
    sscanf(config_data, "%u", &web_port);
     // put scanf in for port
    if((web_port > 0) && (web_port < MAX_PORT)){
       write_port(config_file, &web_port, KMF_COM_PORT);
       send_new_port_to_clients(web_port);
       check_config(config_file);
    }
   #endif
}

void web_add_client(char* config_data){
   #ifdef IS_SERVER
   unsigned int web_ip[4] = {0};
   unsigned char tmp_cnt = 0;
   char bad_ip = 0;
   
    sscanf(config_data, "%u.%u.%u.%u", &web_ip[0], &web_ip[1], &web_ip[2], &web_ip[3]);
    while(tmp_cnt < 4){
        if(web_ip[tmp_cnt] == 0) bad_ip = 1;
        tmp_cnt = tmp_cnt + 1;
    }
    if(!bad_ip){
       add_client(config_file, &web_ip[0], KMF_CLIENT_COUNT);
       check_config(config_file);
    }
   #endif
   #ifdef IS_CLIENT
      if(client_count){
         send_add_client_to_server(&client_ips[0][0]);
      } else {
         printf("Set Client IP Before Adding To Server\n");
      }
   #endif
}

void web_set_client_ip(char* config_data){
   #ifdef IS_CLIENT
   unsigned int web_ip[4] = {0};
   unsigned char tmp_cnt = 0;
   char bad_ip = 0;
   
    sscanf(config_data, "%u.%u.%u.%u", &web_ip[0], &web_ip[1], &web_ip[2], &web_ip[3]);
    while(tmp_cnt < 4){
        if(web_ip[tmp_cnt] == 0) bad_ip = 1;
        tmp_cnt = tmp_cnt + 1;
    }
    if(!bad_ip){
       set_client_ip(config_file, &web_ip[0], KMF_CLIENT_COUNT);
       check_config(config_file);
    }
    #endif
}

void web_update_client_ip(char* config_data){
   #ifdef IS_SERVER
   unsigned int new_ip[4] = {0};
   unsigned char tmp_cnt = 0;
   char bad_ip = 0;
   unsigned int tmp_num = 0;
   
    sscanf(config_data, "%u.%u.%u.%u.%u",
      &tmp_num, &new_ip[0], &new_ip[1], &new_ip[2], &new_ip[3]);
    while(tmp_cnt < 4){
        if(new_ip[tmp_cnt] == 0) bad_ip = 1;
        tmp_cnt = tmp_cnt + 1;
    }
    if(!bad_ip){
       change_client_ip(config_file, tmp_num, &new_ip[0], KMF_CLIENT_COUNT);
       check_config(config_file);
    }
    #endif
}

void web_remove_client(char* config_data){
   #ifdef IS_SERVER
   unsigned int web_ip[4] = {0};
   unsigned char tmp_cnt = 0;
   unsigned char loop = 0;
   char match = 0;
    
    sscanf(config_data, "%u.%u.%u.%u", &web_ip[0], &web_ip[1], &web_ip[2], &web_ip[3]);
    while(!match && (loop < client_count)){
       tmp_cnt = 0;
       match = 1;
       while(match && (tmp_cnt < 4)){
           if(web_ip[tmp_cnt] != client_ips[loop][tmp_cnt]) match = 0;
           tmp_cnt = tmp_cnt + 1;
       }
       loop = loop + 1;
    }
    if(match){
       remove_client(config_file, (loop + 1), KMF_CLIENT_COUNT);
       check_config(config_file);
    }
    #endif
}

void update_system(FILE* config_file){
   // populate global variables for port and ip from config file
   read_port(config_file, &ms_port, KMF_COM_PORT);
   read_ip_address(config_file, &ms_ip[0], KMF_S_IP);
   client_count = read_clients(config_file, client_ips, KMF_CLIENT_COUNT);
}

void send_new_ip_to_server(unsigned int* old_ip, unsigned int* new_ip){
   #ifdef IS_CLIENT
   char function[MAX_FUNCTION_STRING] = {0};
   unsigned int tmp_num;
   get_client_number(config_file, &tmp_num, KMF_CLIENT_COUNT);
   printf("port: %d\n", ms_port);
   sprintf(&function[0], "echo \"1%%kmfcip%u.%u.%u.%u.%u%%\" | nc %u.%u.%u.%u %u",
      tmp_num, new_ip[0], new_ip[1], new_ip[2], new_ip[3],
      ms_ip[0], ms_ip[1], ms_ip[2], ms_ip[3], ms_port);
   system(&function[0]);
   #endif
}

void send_add_client_to_server(unsigned int* new_ip){
   #ifdef IS_CLIENT
   char function[MAX_FUNCTION_STRING] = {0};
   
   sprintf(&function[0], "echo \"1%%kmfaddclient%u.%u.%u.%u%%\" | nc %u.%u.%u.%u %u",
      new_ip[0], new_ip[1], new_ip[2], new_ip[3],
      ms_ip[0], ms_ip[1], ms_ip[2], ms_ip[3], ms_port);
   system(&function[0]);
   #endif
}

void send_new_port_to_clients(unsigned int new_port){
   #ifdef IS_SERVER
   char function[MAX_FUNCTION_STRING] = {0};
   unsigned int temp_cnt = 0;
   
   while(temp_cnt < client_count){
      sprintf(&function[0], "echo \"1%%kmfsetport%u%%\" | nc %u.%u.%u.%u %u",
         new_port, client_ips[temp_cnt][0], client_ips[temp_cnt][1],
         client_ips[temp_cnt][2], client_ips[temp_cnt][3], ms_port);
      temp_cnt = temp_cnt + 1;
      system(&function[0]);
   }
   #endif
}

void send_heartbeat_to_clients(void){
   #ifdef IS_SERVER
   char function[MAX_FUNCTION_STRING] = {0};
   unsigned int temp_cnt = 0;
   
   send_heartbeat = 0;
   restart_heartbeat = 1;
   
   while(temp_cnt < client_count){
      sprintf(&function[0], "echo \"1%%hello%u%%\" | nc %u.%u.%u.%u %u",
         (temp_cnt + 1), client_ips[temp_cnt][0], client_ips[temp_cnt][1],
         client_ips[temp_cnt][2], client_ips[temp_cnt][3], ms_port);
      temp_cnt = temp_cnt + 1;
      system(&function[0]);
   }
   #endif
}

void receive_heartbeat_from_server(char* config_data){
   unsigned int tmp_num = 0;
   printf("got heartbeat from server\n");
   sscanf(config_data, "%u", &tmp_num);
   set_client_number(config_file, tmp_num, KMF_CLIENT_COUNT);
   send_heartbeat_to_server();
}

void receive_heartbeat_from_client(char* config_data){
   #ifdef IS_SERVER
   //char function[MAX_FUNCTION_STRING] = {0};
   //unsigned int temp_cnt = 0;
   
   printf("got heartbeat\n");
   /*while(temp_cnt < client_count){
      sprintf(&function[0], "echo \"1%%hello%%\" | nc %u.%u.%u.%u %u",
         client_ips[temp_cnt][0], client_ips[temp_cnt][1],
         client_ips[temp_cnt][2], client_ips[temp_cnt][3], ms_port);
      temp_cnt = temp_cnt + 1;
   }*/
   #endif
}

void send_heartbeat_to_server(void){
   #ifdef IS_CLIENT
   char function[MAX_FUNCTION_STRING] = {0};
   unsigned int tmp_num = 0;
   get_client_number(config_file, &tmp_num, KMF_CLIENT_COUNT);
   sprintf(&function[0], "echo \"1%%imhere%u.%u.%u.%u.%u%%\" | nc %u.%u.%u.%u %u",
      client_ips[0][0], client_ips[0][1],
      client_ips[0][2], client_ips[0][3], tmp_num,
      ms_ip[0], ms_ip[1], ms_ip[2], ms_ip[3], ms_port);
      system(&function[0]);
   #endif
}

void send_new_ip_to_clients(unsigned int* new_ip){
   #ifdef IS_SERVER
   char function[MAX_FUNCTION_STRING] = {0};
   unsigned int temp_cnt = 0;
   
   while(temp_cnt < client_count){
      sprintf(&function[0], "echo \"kmfsip%u.%u.%u.%u\" | nc %u.%u.%u.%u %u",
         new_ip[0], new_ip[1], new_ip[2], new_ip[3],
         client_ips[temp_cnt][0], client_ips[temp_cnt][1],
         client_ips[temp_cnt][2], client_ips[temp_cnt][3], ms_port);
      temp_cnt = temp_cnt + 1;
      system(&function[0]);
   }
   #endif
}

void initial_config(FILE* config_file){
   char cfg_data[TMP_DATA_SIZE] = {0};
   
   strncpy(&cfg_data[0], "kmf", 3);
    write_config_data(config_file, &cfg_data[0], KMF_BASE, 3);
    
    cfg_data[0] = 0;
    cfg_data[1] = 0;
    cfg_data[2] = 1;
    write_config_data(config_file, &cfg_data[0], KMF_VERSION, 3);
    
    #ifdef IS_SERVER
      cfg_data[0] = 1;// server is 1, client is 0
    #endif
    #ifdef IS_CLIENT
      cfg_data[0] = 0;// server is 1, client is 0
    #endif
    write_config_data(config_file, &cfg_data[0], KMF_SYS_ID, 1);
}

