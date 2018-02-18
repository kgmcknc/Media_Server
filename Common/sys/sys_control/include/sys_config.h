
void configure_system(void);
void print_config_menu(void);
char check_config(FILE* config_file);
void initial_config(FILE* config_file);
char check_file_size(FILE* config_file);
char read_file_size(FILE* config_file, long int* config_data, long int offset);
char write_file_size(FILE* config_file, long int* config_data, long int offset);
char read_port(FILE* config_file, unsigned int* config_data, long int offset);
char write_port(FILE* config_file, unsigned int* config_data, long int offset);
char read_ip_address(FILE* config_file, unsigned int* config_data, long int offset);
char write_ip_address(FILE* config_file, unsigned int* config_data, long int offset);
char set_client_ip(FILE* config_file, unsigned int* config_data, long int offset);
char set_client_number(FILE* config_file, unsigned int client_number, long int offset);
char get_client_number(FILE* config_file, unsigned int* client_number, long int offset);
char add_client(FILE* config_file, unsigned int* config_data, long int offset);
unsigned int read_clients(FILE* config_file, unsigned int config_data[][4], long int offset);
char remove_client(FILE* config_file, unsigned int config_data, long int offset);
char reorder_clients(FILE* config_file, unsigned int config_data, unsigned int config_data_n, long int offset);
char read_config_data(FILE* config_file, char* config_data, long int offset, int size);
char write_config_data(FILE* config_file, char* config_data, long int offset, int size);
char change_client_ip(FILE* config_file, unsigned int old_ip, unsigned int* new_ip, long int offset);
void update_system(FILE* config_file);

void send_new_ip_to_server(unsigned int* old_ip, unsigned int* new_ip);
void send_new_ip_to_clients(unsigned int* new_ip);
void send_new_port_to_clients(unsigned int new_port);
void send_add_client_to_server(unsigned int* new_ip);
void send_rem_client_from_server(unsigned int client_id, unsigned int* client_ip);
void send_heartbeat_to_clients(void);
void send_heartbeat_to_server(void);
void receive_heartbeat_from_client(char* config_data);
void receive_heartbeat_from_server(char* config_data);

void web_port_update(char* config_data);
void web_server_ip_update(char* config_data);
void web_add_client(char* config_data);
void web_remove_client(char* config_data);
void web_set_client_ip(char* config_data);
void web_update_client_ip(char* config_data);

void send_to(unsigned char input_string[MAX_INPUT_STRING], unsigned int address[4]);

#define KMF_BASE 0
#define KMF_VERSION 3
#define KMF_FILE_SIZE 6
#define KMF_SYS_ID 10
#define KMF_COM_PORT 11
#define KMF_S_IP 13
#define KMF_CLIENT_COUNT 18

