
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
#define OPTION_COUNT 4
#define FUNCTION_COUNT 18
#define MAX_STRING 400
#define MAX_FUNCTION_STRING 400
#define MAX_CONFIG_FILE 4096
#define MIN_CONFIG_SIZE 20
#define TMP_DATA_SIZE 40
#define MAX_PORT 5000
#define MAX_CLIENTS 8
#define USE_TIMEOUT 1
#define NO_TIMEOUT 0

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

void configure_system(void);
void print_config_menu(void);
char check_config(FILE* config_file);
char read_file_size(FILE* config_file, long int* config_data, long int offset);
char write_file_size(FILE* config_file, long int* config_data, long int offset);
char read_port(FILE* config_file, unsigned int* config_data, long int offset);
char write_port(FILE* config_file, unsigned int* config_data, long int offset);
char read_ip_address(FILE* config_file, unsigned int* config_data, long int offset);
char write_ip_address(FILE* config_file, unsigned int* config_data, long int offset);
char add_client(FILE* config_file, unsigned int* config_data, long int offset);
unsigned int read_clients(FILE* config_file, unsigned int config_data[][4], long int offset);
char remove_client(FILE* config_file, unsigned int config_data, long int offset);
char reorder_clients(FILE* config_file, unsigned int config_data, unsigned int config_data_n, long int offset);
char read_config_data(FILE* config_file, char* config_data, long int offset, int size);
char write_config_data(FILE* config_file, char* config_data, long int offset, int size);
void update_system(FILE* config_file);

extern FILE* config_file;


extern char filestring[MAX_STRING];
extern char funcstring[MAX_STRING];
extern char fvalid;
extern int flength;
extern long int file_length;
extern unsigned int ms_port;
extern char restart_listener;
extern char user_option;
