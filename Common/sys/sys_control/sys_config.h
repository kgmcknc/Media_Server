
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
char add_client(FILE* config_file, unsigned int* config_data, long int offset);
unsigned int read_clients(FILE* config_file, unsigned int config_data[][4], long int offset);
char remove_client(FILE* config_file, unsigned int config_data, long int offset);
char reorder_clients(FILE* config_file, unsigned int config_data, unsigned int config_data_n, long int offset);
char read_config_data(FILE* config_file, char* config_data, long int offset, int size);
char write_config_data(FILE* config_file, char* config_data, long int offset, int size);
void update_system(FILE* config_file);

void web_port_update(char* config_data);
void web_server_ip_update(char* config_data);
void web_add_client(char* config_data);
void web_remove_client(char* config_data);

#define KMF_BASE 0
#define KMF_VERSION 3
#define KMF_FILE_SIZE 6
#define KMF_SYS_ID 10
#define KMF_COM_PORT 11
#define KMF_S_IP 13
#define KMF_CLIENT_COUNT 18

