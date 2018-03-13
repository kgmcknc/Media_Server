
#include "sys_control.h"
#include "sys_config.h"
#include "sys_functions.h"
#include <stdio.h>
#include <stdlib.h>

void updatewebstate(FILE* out_file);
void init_webstate(FILE* out_file);

char filestring[MAX_STRING] = {0};
char funcstring[MAX_STRING] = {0};
char fvalid = 0;
int flength = 0;
char valid_config = 0;
long int file_length = 0;

char usertmp[MAX_STRING] = {0};
char username[MAX_STRING] = {0};
unsigned int ms_port = 0;
unsigned int ms_ip[4] = {0};
char client_count = 0;
unsigned int client_ips[MAX_CLIENTS][4] = {{0}};
char client_state[MAX_CLIENTS] = {0};
unsigned int client_id[MAX_CLIENTS] = {0};
char user_option = 0;
char send_heartbeat = 0;
char restart_heartbeat = 1;

char restart_listener = 0;
pid_t listener;
pid_t heartbeat;

FILE* user_fp;
FILE* config_file;

int main(int argc, char **argv) {

    printf("\n\n----- Starting Kyle's System -----\n\n");
    
    user_fp = popen("ls /home/", "r");
    if(user_fp == NULL){
        printf("Failed to get User...\n");
        exit(1);
    } else {
        fgets(usertmp, sizeof(usertmp)-1, user_fp);
        strncpy(username, usertmp, (strlen(usertmp)-1));
        printf("\n\n----- Got %s As User -----\n\n", username);
    }

    config_file = fopen(CONFIG_PATH, "r+b");
    if(config_file != NULL){
        printf("\n\n----- Successfully Opened Config File -----\n\n");
    } else {
        printf("\n\n----- Failed To Open Config File, Exiting -----\n\n");
        exit(EXIT_FAILURE);
    }
    rewind(config_file);

    if(argc > 1) {
        printf("\n\n----- Checking Input Argument -----\n\n");
        if(strcmp(argv[1], "Config") || strcmp(argv[1], "config")) {
                 printf("\n\n----- Running System Config -----\n\n");
        configure_system();
        } else {
            printf("\n\n----- Unknown Argument... Exiting -----\n\n");
            exit(EXIT_FAILURE);
        }
    }

    printf("\n\n----- Checking Configuration Settings -----\n\n");
    check_config(config_file);
    #ifdef IS_SERVER
        printf("\n\n----- Running as Server -----\n\n");
    #endif
    #ifdef IS_CLIENT
        printf("\n\n----- Running as Client -----\n\n");
    #endif
    
    printf("\n\n----- Resetting Web State -----\n\n");
    //init_webstate(webfile);

    printf("\n\n----- StartingListener -----\n\n");
    // call fork and start listener

    while(PI_IS_ON){
        restart_listener = 0;
        while(waitpid(-1, NULL, WNOHANG) > 0);
        listener = fork();
        if(listener == 0){
            char nclistener[MAX_STRING] = {0};
            char check[MAX_STRING] = {0};
            char rx_fpath[MAX_STRING] = RX_PATH;
            unsigned int ps_id = 0;
            sprintf(&nclistener[0], "nc -k -l %u > %s", ms_port, &rx_fpath[0]);
            // child id -- timeout catch
            while(1){
                printf("Child calling listener!\n");
                // make this be the listener system call
                printf("Function: %s\n", &nclistener[0]);
                system_kill(nclistener);
                printf("Starting Listener\n");
                system(&nclistener[0]);
                printf("Exitting Listener\n");
            }
            exit(EXIT_SUCCESS);
        } else {
            printf("\n\n----- Starting Main Loop -----\n\n");
            char rx_fpath[MAX_STRING] = RX_PATH;
            while(!restart_listener){
                // wait, so pi only checks function file every second or so...
                while(waitpid(-1, NULL, WNOHANG) > 0);
                #ifdef IS_SERVER
                if(restart_heartbeat) {
                    restart_heartbeat = 0;
                    heartbeat = fork();
                    if(heartbeat == 0) {
                        char set_heartbeat[MAX_STRING] = {0};
                        sleep(10);
                        sprintf(&set_heartbeat[0], "echo \"1%%sendheartbeat%%\" > %s", &rx_fpath[0]);
                        system(&set_heartbeat[0]);
                        exit(EXIT_SUCCESS);
                    }
                }
                if(send_heartbeat) {
                    send_heartbeat_to_clients();
                }
                #endif
                //check to see if file changed
                checkfunctionfile(USE_TIMEOUT);

                if(restart_listener){
                    printf("\n\n----- Breaking Out of Main -----\n\n");
                    char killstring[MAX_STRING] = {0};
                    kill(listener, SIGKILL);
                    while(waitpid(-1, NULL, WNOHANG) > 0);
                    sprintf(killstring, "nc -k -l %u > %s", ms_port, &rx_fpath[0]);
                    system_kill(killstring);
                }
            }
        }
    }
    fclose(config_file);
    return 0;
}
