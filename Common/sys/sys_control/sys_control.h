
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include "/usr/share/media_server/sys_control/configuration.h"
//#include <wiringPi.h>

#define PI_IS_ON 1
#define ERROR_HALT 1
#define FILE_DEBUG 1
#define USE_WPI 1
#define OPTION_COUNT 12
#define FUNCTION_COUNT 18
#define MAX_STRING 400
#define MAX_FUNCTION_STRING 400
#define MAX_CONFIG_FILE 4096
#define MIN_CONFIG_SIZE 20
#define TMP_DATA_SIZE 40
#define MAX_PORT 5000
#ifdef IS_SERVER
   #define MAX_CLIENTS 8
#endif
#ifdef IS_CLIENT
   #define MAX_CLIENTS 1
#endif
#define USE_TIMEOUT 1
#define NO_TIMEOUT 0

#define CONFIG_PATH "/usr/share/media_server/sys_control/sys_config.kmf"
#define RX_PATH "/var/www/html/media_server/control/web_control/rxwebpipe"
#define WEB_PATH "/var/www/html/media_server/control/web_control/txwebpipe"

extern FILE* config_file;

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

