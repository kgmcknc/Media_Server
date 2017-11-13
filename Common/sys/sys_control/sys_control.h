
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
//#include <wiringPi.h>

#define PI_IS_ON 1
#define ERROR_HALT 1
#define FILE_DEBUG 1
#define USE_WPI 1
#define OPTION_COUNT 3
#define FUNCTION_COUNT 12
#define MAX_STRING 400
#define MAX_FUNCTION_STRING 400
#define MAX_CONFIG_FILE 4096
#define MIN_CONFIG_SIZE 20
#define TMP_DATA_SIZE 40
#define MAX_PORT 5000
#define MAX_CLIENTS 8

#define CONFIG_PATH "/usr/share/media_server/sys_control/sys_config.kmf"
#define RX_PATH "/var/www/html/media_server/control/web_control/rxwebpipe"
#define WEB_PATH "/var/www/html/media_server/control/web_control/txwebpipe"

#define KMF_BASE 0
#define KMF_VERSION 3
#define KMF_FILE_SIZE 6
#define KMF_SYS_ID 10
#define KMF_COM_PORT 11
#define KMF_S_IP 13
#define KMF_CLIENT_COUNT 18

extern char filestring[MAX_STRING];
extern char funcstring[MAX_STRING];
extern char fvalid;
extern int flength;
extern long int file_length;
