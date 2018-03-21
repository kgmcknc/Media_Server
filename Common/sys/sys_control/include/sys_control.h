
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <sys/time.h>
#include <string.h>
#include "configuration.h"
//#include <wiringPi.h>

#define PI_IS_ON 1
#define ERROR_HALT 1
#define FILE_DEBUG 0
#define USE_WPI 1
#define OPTION_COUNT 13
#define FUNCTION_COUNT 16
#define MAX_STRING 400
#define MAX_INPUT_STRING 100
#define MAX_FUNCTION_STRING 400
#define MAX_CONFIG_FILE 4096
#define MIN_CONFIG_SIZE 20
#define TMP_DATA_SIZE 40
#define MAX_PORT 50000
#ifdef IS_SERVER
   #define MAX_CLIENTS 8
#endif
#ifdef IS_CLIENT
   #define MAX_CLIENTS 1
#endif
#define USE_TIMEOUT 1
#define NO_TIMEOUT 0
#define USE_HEARTBEAT 1

#define CONFIG_PATH "/usr/share/media_server/sys_control/sys_config.kmf"
#define MOVIEDIR_PATH "/usr/share/media_server/sys_control/moviectrl/moviedir.kmf"
#define MUSICDIR_PATH "/usr/share/media_server/sys_control/musicctrl/musicdir.kmf"
#define RX_PATH "/var/www/html/media_server/control/web_control/rxwebpipe"
//#define WEB_PATH "/var/www/html/media_server/control/web_control/txwebpipe"

#define HEARTBEAT_PATH "\0/usr/share/media_server/sys_control/hb_socket"
#define COM_PATH "\0/usr/share/media_server/sys_control/com_socket"
#define LOCAL_PATH "\0/usr/share/media_server/sys_control/main_socket"
#define WEB_PATH "\0/usr/share/media_server/sys_control/web_socket"

#define RUNNING 1
#define STOPPED 0
#define HB_PERIOD 15
#define MAX_FILE_STRING 400
#define MAX_LOCAL 8
#define MAX_SYS_SOCKETS 1 // master sockets for unix or inet
#define MAX_UNIX_SOCKETS 1 // Used for Local Cross Process Communication
#define MAX_LOCAL_SOCKETS MAX_LOCAL // Used for Webpage Communication
#define MAX_INET_SOCKETS MAX_CLIENTS // these would be attached remote clients

#define TIMEOUT_TIME 20

/*

: System Sockets -- listeners for new connections (unix or inet)
: Unix Sockets -- local process communication
: Local Sockets -- local web sockets
: Client Sockets -- attached decoder clients

*/

struct system_function{
    char string[MAX_FUNCTION_STRING];
};

void server_listener(void);
void get_rx(int com_socket);
void process(int client, char f_string[MAX_FUNCTION_STRING]);
void server_system(void);
void heartbeat(int hb_socket);
int create_unix_socket(char path[MAX_FILE_STRING]);
int connect_unix_socket(char path[MAX_FILE_STRING]);
int create_main_socket(unsigned int port);
int connect_client_socket(char ip[4], unsigned int port);
int socket_handler(void);
void system_setup(void);
void function_setup(void);
void set_function(struct system_function* sf, char f_string[MAX_FUNCTION_STRING]);

extern FILE* config_file;

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
extern char valid_config;
extern char client_count;
extern unsigned int client_ips[MAX_CLIENTS][4];
extern char client_state[MAX_CLIENTS];
extern unsigned int client_id[MAX_CLIENTS];
extern char send_heartbeat;
extern char restart_heartbeat;

