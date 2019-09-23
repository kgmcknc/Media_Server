#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "string.h"
#include "http.h"
#include "main.h"
#include "connections.h"

// char header_okay[] = "HTTP/1.1 200 OK\r\nAccept-Ranges: bytes\r\n";
// char header_error[] = "HTTP/1.1 200 OK\r\nAccept-Ranges: bytes\r\n";
// char header_length[] = "Content-Length: ";
// char header_end[] = "Connection: close\r\nContent-Type: text/html\r\nX-Pad: avoid browser bug\r\n\r\n";

char command_string[NUMBER_COMMANDS][MAX_COMMAND_STRING] = {
    "get_active_devices",
    "add_device_to_server",
    "rem_device_from_server"
};

void handle_http_message(struct system_struct* system, char* packet_data, struct device_info_struct* device){
    struct http_message_struct http_message;
    if(strstr(packet_data, "GET") > 0){
        http_message.is_get = 1;
        printf("Http Get Message: %s\n", packet_data);
        process_message(system, device, packet_data, &http_message);
    } else {
        if(strstr(packet_data, "POST") > 0){
            http_message.is_get = 0;
            printf("Http Post Message: %s\n", packet_data);
            process_message(system, device, packet_data, &http_message);
            char response[] = "<html><body><h1>POST works!</h1></body></html>";
            send_http_okay(device, response, strlen(response));
        }
    }
}

void process_message(struct system_struct* system, struct device_info_struct* device, char* message, struct http_message_struct* http){
    char* command_pointer;
    char* message_pointer;
    char response[MAX_PACKET_LENGTH] = {'\0'};
    char* response_pointer = &response[0];
    int write_count;

    command_pointer = strstr(message, "q={");
    command_pointer = command_pointer + 2; // got open brace
    if(*command_pointer == '{'){
        // valid json object
        command_pointer = command_pointer + 1;
        memset(&http->command[0], 0, MAX_COMMAND_STRING);
        message_pointer = get_json_string(command_pointer, &http->command[0]);
    } else {
        // unknown data... close connection
    }
    
    http->command_number = 0;

    for(int i=0;i<NUMBER_COMMANDS;i++){
        if(strcmp(http->command, command_string[i]) == 0){
            // found command
            break;
        }
        http->command_number++;
    }
    printf("command: %s, num: %d\n", http->command, http->command_number);
    switch(http->command_number){
        case 0 : {
            if(http->is_get){
                write_count = sprintf(response_pointer, "{\"active_device_count\":%d", system->active_devices.device_count);
                response_pointer = response_pointer + write_count;
                if(system->active_devices.device_count){
                    write_count = sprintf(response_pointer, ",\"active_devices\":[");
                    response_pointer = response_pointer + write_count;
                    for(int i=0;i<system->active_devices.device_count;i++){
                        if(i > 0){
                            write_count = sprintf(response_pointer, ",%u", system->active_devices.device[i].config.device_id);
                        } else {
                            write_count = sprintf(response_pointer, "%u", system->active_devices.device[i].config.device_id);
                        }
                        response_pointer = response_pointer + write_count;
                    }
                    *response_pointer = ']';
                    response_pointer = response_pointer + 1;
                }
                *response_pointer = '}';
                response_pointer = response_pointer + 1;
                *response_pointer = '\0';
            } else {
                // send empty response
            }
            send_http_okay(device, response, strlen(response));
            break;
        }
        case 1 : {
            if(http->is_get){
                // can't add from get
            } else {
                int32_t new_device_id;
                struct system_config_struct new_device;
                message_pointer = message_pointer + 1;
                sscanf(message_pointer, "%d", &new_device_id);
                link_device_to_server(system, new_device_id);
            }
            send_http_okay(device, command_pointer, strlen(command_pointer));
            break;
        }
        case 2 : {
            if(http->is_get){
                // can't remove from get
            } else {
                int32_t new_device_id;
                struct system_config_struct new_device;
                message_pointer = message_pointer + 1;
                sscanf(message_pointer, "%d", &new_device_id);
                remove_device_from_server(system, new_device_id);
            }
            send_http_okay(device, command_pointer, strlen(command_pointer));
            break;
        }
        default : {
            printf("Message Switch Statement Error\n");
            // close connection
        }
    }
}

void send_http_okay(struct device_info_struct* device, char* packet_data, uint32_t packet_length){
    char packet[MAX_PACKET_LENGTH];
    memset(packet, 0, MAX_PACKET_LENGTH);
    sprintf(packet, "%s%s%d\r\n%s%s", HEADER_OKAY, HEADER_LENGTH, packet_length, HEADER_END, packet_data);
    send(device->device_socket, packet, MAX_PACKET_LENGTH, 0);
}

void send_http_error(struct device_info_struct* device, uint32_t error_number){
    
}

void send_http_not_found(struct device_info_struct* device){

}

char* get_json_string(char* command_pointer, char* command){
    char* string_pointer = command_pointer;
    int max_string_counter = 0;
    if(*string_pointer == '\"'){
        string_pointer = string_pointer + 1;
        while((*string_pointer != '\"') && (max_string_counter < MAX_VALUE_STRING)){
            command[max_string_counter] = string_pointer[0];
            string_pointer = string_pointer + 1;
            max_string_counter = max_string_counter + 1;
        }
        string_pointer = string_pointer + 1;
    } else {
        printf("didn't find valid string at pointer location\n");
    }
    return string_pointer;
}
