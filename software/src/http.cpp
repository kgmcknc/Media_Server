#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "string.h"
#include "http.h"
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
        process_message(packet_data, &http_message);
        char response[] = "<html><body><h1>GET works!</h1></body></html>";
        send_http_okay(device, response, strlen(response));
    } else {
        if(strstr(packet_data, "POST") > 0){
            http_message.is_get = 0;
            printf("Http Post Message: %s\n", packet_data);
            process_message(packet_data, &http_message);
            char response[] = "<html><body><h1>POST works!</h1></body></html>";
            send_http_okay(device, response, strlen(response));
        }
    }
}

void process_message(char* message, struct http_message_struct* http){
    char* command_pointer;
    sscanf(message, "{\"%s\":", http->command);
    
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
            printf("get active devices\n");
            break;
        }
        case 1 : {
            printf("add device\n");
            break;
        }
        case 2 : {
            printf("remove device\n");
            break;
        }
        default : {
            printf("Message Switch Statement Error\n");
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