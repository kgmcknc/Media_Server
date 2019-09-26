#ifndef SRC_HTTP_H_
#define SRC_HTTP_H_

#include <stdint.h>
#include "main.h"

#define HEADER_OKAY "HTTP/1.1 200 OK\r\nAccept-Ranges: bytes\r\n"
#define HEADER_ERROR "HTTP/1.1 200 OK\r\nAccept-Ranges: bytes\r\n"
#define HEADER_LENGTH "Content-Length: "
#define HEADER_END "Connection: close\r\nContent-Type: text/html\r\nX-Pad: avoid browser bug\r\n\r\n"

//"200 OK", "404 Not Found", "403 Forbidden", "500 Internal Server Error".

#define NUMBER_COMMANDS 3
#define MAX_COMMAND_STRING 256
#define MAX_VALUE_STRING 512

struct http_message_struct {
    char is_get;
    char command[MAX_COMMAND_STRING];
    uint16_t command_number;
    char value_string[MAX_VALUE_STRING];
    int64_t value_number;
};

extern char command_string[NUMBER_COMMANDS][MAX_COMMAND_STRING];

void handle_http_message(struct system_struct* system, char* packet_data, struct local_connection_struct* device);
void process_message(struct system_struct* system, struct local_connection_struct* device, char* message, struct http_message_struct* http);
void send_http_okay(struct local_connection_struct* device, char* packet_data, uint32_t packet_length);
void send_http_error(struct local_connection_struct* device, uint32_t error_number);
void send_http_not_found(struct local_connection_struct* device);

uint8_t get_json_string(char* json_pointer, const char* search_string, char* result_string);
uint8_t get_json_int(char* json_pointer, const char* search_string, int32_t* value);

#endif // SRC_HTTP_H_