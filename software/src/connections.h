
#ifndef SRC_CONNECTIONS_H_
#define SRC_CONNECTIONS_H_

#include <stdint.h>
#include "config.h"

#define CONTINUE_HEARTBEAT 1
#define HEARTBEAT_PERIOD_SEC 10
#define MAX_BROADCAST_PACKET 512
#define UDP_BROADCAST_PORT 1900

void heartbeat(struct system_config_struct* system_config);
int send_broadcast_packet(uint16_t broadcast_port, char* packet_data, uint16_t data_length);
uint16_t receive_broadcast_packet(uint16_t broadcast_port, char* packet_data);

// #include <stdio.h>
// #include <stdlib.h>
// #include <fcntl.h>
// #include <sys/stat.h>
// #include <sys/types.h>
// #include <sys/wait.h>
// #include <signal.h>
// #include <unistd.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <sys/un.h>
// #include <sys/time.h>
// #include <string.h>
// //#include <wiringPi.h>

// #define PI_IS_ON 1
// #define ERROR_HALT 1
// #define FILE_DEBUG 0
// #define USE_WPI 1
// #define OPTION_COUNT 13
// #define FUNCTION_COUNT 16
// #define MAX_STRING 400
// #define MAX_INPUT_STRING 100
// #define MAX_FUNCTION_STRING 400
// #define MAX_CONFIG_FILE 4096
// #define MIN_CONFIG_SIZE 20
// #define MAX_NAME_LENGTH 200
// #define TMP_DATA_SIZE 40
// #define MAX_PORT 65535

// #define USE_TIMEOUT 1
// #define NO_TIMEOUT 0
// #define USE_HEARTBEAT 1

#endif //SRC_CONNECTIONS_H_