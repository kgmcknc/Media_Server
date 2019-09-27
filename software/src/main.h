
#ifndef SRC_MAIN_H_
#define SRC_MAIN_H_

#include <stdint.h>
#include "connections.h"
#include "config.h"
#include "database.h"
#include <mysql/mysql.h>

struct system_struct {
    struct system_config_struct config;
    struct network_struct network;
    struct database_struct database;
    struct device_table_struct active_devices;
    struct local_connection_table_struct local_connections;
    struct linked_device_table_struct linked_devices; // gets loaded from database
};

uint8_t main_process(struct system_struct* system);
void safe_shutdown(int sig);
void system_shutdown(void);


//void receive_broadcast_packet(void);
void receive_get_request(void);
/*s
void server_listener(void);
void get_rx(int com_socket);
void process(char type, char client, char f_string[MAX_FUNCTION_STRING]);
void process_function(char client);
void server_system(void);
int create_unix_socket(const char path[MAX_FILE_STRING]);
int connect_unix_socket(const char path[MAX_FILE_STRING]);
int create_main_socket(unsigned int port);
int connect_client_socket(unsigned int ip[4], unsigned int port);
int socket_handler(void);
void system_setup(void);
void function_setup(void);
void set_function(struct system_function* sf, const char f_string[MAX_FUNCTION_STRING]);

extern FILE* config_file;
extern FILE* status_file;
extern FILE* name_file;
extern FILE* grep_fp;

extern int sys_sockets[MAX_SYS_SOCKETS];
extern int unix_sockets[MAX_UNIX_SOCKETS];
extern int local_sockets[MAX_LOCAL_SOCKETS];
extern int client_sockets[MAX_INET_SOCKETS];
extern char active_clients;
extern char active_unix;
extern char active_local;
extern char active_system;

extern char username[MAX_STRING];
extern char filestring[MAX_STRING];
extern char funcstring[MAX_STRING];
extern char fvalid;
extern int flength;
extern long int file_length;
extern unsigned int ms_port;
extern char restart_listener;
extern char user_option;
extern unsigned int ms_ip[4];
extern char ms_name[MAX_NAME_LENGTH];
extern char valid_config;
extern char client_count;
extern unsigned int client_ips[MAX_CLIENTS][4];
extern char client_state[MAX_CLIENTS];
extern char client_names[MAX_CLIENTS][MAX_NAME_LENGTH];
extern unsigned int client_id[MAX_CLIENTS];
extern char restart_heartbeat;
*/

#endif /* SRC_MAIN_H_ */