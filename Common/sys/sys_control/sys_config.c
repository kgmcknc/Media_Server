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

#define KMF_BASE 0
#define KMF_VERSION 3
#define KMF_FILE_SIZE 6
#define KMF_COM_PORT 10
#define KMF_S_IP 12
#define KMF_S_STATUS 16
#define KMF_CLIENT_COUNT 17
#define KMF_CLIENT_STATUS 18

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
    write_file_size(config_file, &file_size, 6);
    
    strncpy(&cfg_data[0], "kmf", 3);
    write_config_data(config_file, &cfg_data[0], KMF_BASE, 3);
    
    cfg_data[0] = 0;
    cfg_data[1] = 0;
    cfg_data[2] = 1;
    write_config_data(config_file, &cfg_data[0], KMF_VERSION, 3);
    
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
                } else {
                    printf("\nIP Was Out of Range, Try Setting Again\n");
                }
            }
        }
        if(user_option == 'c'){
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
        if(!read_config_data(config_file, &cfg_data[0], 12, 4)) return 1;
        printf("config data: %s\n", cfg_data);
        if(!read_config_data(config_file, &cfg_data[0], 16, 4)) return 1;
        printf("config data: %s\n", cfg_data);
        return 0;
    }
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
    
    //tmp_data = getc(config_file);
    //if(tmp_data == EOF) return 0;
    //client_count = tmp_data;
    //client_count = (client_count << 8);
    tmp_data = getc(config_file);
    if(tmp_data == EOF) return 0;
    client_count = client_count | tmp_data;
    
    while(loop_count < client_count){
        new_offset = offset + 3 + (4*(loop_count));
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
    
    new_offset = offset + 3 + (4*(client_count));
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
    file_size = file_size + 4;
    file_length = file_size;
    write_file_size(config_file, &file_size, KMF_FILE_SIZE);
    
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
        new_offset = offset + 3 + (4*(loop_count+1));
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
    file_size = file_size - 4;
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
    
    new_offset = offset + 3 + (4*(config_data - 1));
    if((new_offset < MAX_CONFIG_FILE) && (new_offset <= file_length)){
        read_ip_address(config_file, &tmp_ip[0], new_offset);
    } else {
        return 0;
    }
    new_offset = offset + 3 + (4*(config_data_n - 1));
    if((new_offset < MAX_CONFIG_FILE) && (new_offset <= file_length)){
        read_ip_address(config_file, &tmp_ip_n[0], new_offset);
    } else {
        return 0;
    }
    new_offset = offset + 3 + (4*(config_data - 1));
    if((new_offset < MAX_CONFIG_FILE) && (new_offset <= file_length)){
        write_ip_address(config_file, &tmp_ip_n[0], new_offset);
    } else {
        return 0;
    }
    new_offset = offset + 3 + (4*(config_data_n - 1));
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

