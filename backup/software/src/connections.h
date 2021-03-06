
#ifndef SRC_CONNECTIONS_H_
#define SRC_CONNECTIONS_H_

#include <stdint.h>
#include "config.h"
#include <unistd.h>
#include <sys/types.h>

#define CONTINUE_HEARTBEAT      1
#define HEARTBEAT_PERIOD_SEC    4
#define TIMEOUT_TIME            (HEARTBEAT_PERIOD_SEC*4)
#define MAX_BROADCAST_PACKET    512
#define MAX_PACKET_LENGTH       1440
#define UDP_BROADCAST_PORT      1900
#define UDP_TRANSFER_LENGTH     128
#define MAX_ACTIVE_DEVICES      64
#define LISTEN_DEPTH            1024

#define MAX_CONNECT_READ_WAIT   10
#define READ_USLEEP_COUNT       500

#define NOT_CONNECTED           0
#define IS_CONNECTED            1

struct device_info_struct {
    struct system_config_struct config;
    int device_socket;
    uint8_t is_active;
    uint8_t is_connected;
    uint8_t is_linked;
    uint8_t timeout_set;
};

struct device_table_struct {
    uint8_t device_count;
    struct device_info_struct device[MAX_ACTIVE_DEVICES];
};

struct local_connection_struct {
    int device_socket;
    uint8_t is_connected;
};

struct local_connection_table_struct {
    uint8_t device_count;
    struct local_connection_struct device[MAX_ACTIVE_DEVICES];
};

struct linked_device_struct {
    int32_t device_id;
    uint8_t is_valid;
    char device_name[MAX_DEVICE_NAME];
};

struct linked_device_table_struct {
    uint8_t device_count;
    struct linked_device_struct device[MAX_ACTIVE_DEVICES];
};

struct network_struct {
    int network_socket_fd;
    fd_set network_set;
    int max_network_set;
};

void heartbeat(struct system_config_struct* system_config);
int send_broadcast_packet(uint16_t broadcast_port, char* packet_data, uint16_t data_length);
uint16_t receive_broadcast_packet(uint16_t broadcast_port, char* packet_data);
void listen_for_devices(struct system_config_struct* system_config, struct system_config_struct* new_device);
uint8_t validate_packet(char* packet_data, uint16_t packet_length);
void add_device_to_table(struct system_struct* system, struct system_config_struct* new_device);
void remove_inactive_devices(struct device_table_struct* active_devices);
void clean_up_table_order(struct device_table_struct* device_table);
void clean_up_local_table(struct local_connection_table_struct* device_table);
void clean_up_linked_table(struct linked_device_table_struct* device_table);
void set_device_timeout_flags(struct device_table_struct* active_devices);

int create_network_socket(struct system_config_struct* system_config);
void connect_linked_devices(struct system_struct* system);
uint8_t connect_device(struct network_struct* network, struct system_config_struct* system_config, struct device_info_struct* device);
void set_new_connections(struct network_struct* network);
void wait_for_new_connections(struct network_struct* network); // receives server and network connections
void receive_connections(struct system_struct* system);
void check_connections(struct system_struct* system);
void close_device_connection(struct device_info_struct* device);
void link_device_to_server(struct system_struct* system, struct linked_device_struct* new_linked_device);
void remove_device_from_server(struct system_struct* system, int32_t device_id);
void init_device_table_struct(struct device_table_struct* device_table);
void init_device_info_struct(struct device_info_struct* device_info);
void init_local_table_struct(struct local_connection_table_struct* device_info);
void init_linked_table_struct(struct linked_device_table_struct* device_table);

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