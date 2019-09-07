
#define MAX_FUNCTION_STRING 128

void check_function(char client, char f_string[MAX_FUNCTION_STRING]);
void transmit(int tx_port, char* tx_ip, char* tx_data);
void system_kill(char* proc_name);
