
// #include "config.h"
// #include "database.h"
// #include "http.h"

// #include <stdio.h>
// #include <stdlib.h>
// #include <stdint.h>

// #include <string.h>
// #include <fcntl.h>
// #include <errno.h>
// //#include <sys/stat.h>
// #include <sys/types.h>
// //#include <sys/wait.h>
 
// #include <unistd.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <sys/select.h>
// // #include <sys/un.h>
// // #include <sys/time.h>
// //#include <wiringPi.h>

// uint8_t connect_database(MYSQL* conn){
//     //char server[] = "localhost";
//     //char user[] = "root";
//     //char *password = "root"; /* set me first */
//     char database[] = "media_server";

//     mysql_init(conn);
    
//     /* Connect to database */
//     if (!mysql_real_connect(conn, NULL, NULL, NULL, 
//                                         database, 0, NULL, 0)) {
//         fprintf(stderr, "%s\n", mysql_error(conn));
//         return 1;
//     }
//     printf("Connected Database!\n");
    
//     return 0;
// }

// void close_database(MYSQL* conn){
//     mysql_close(conn);
// }

// void add_int_to_table(MYSQL* conn, const char* table_name, const char* column_name, int32_t* data){

// }

// void add_string_to_table(MYSQL* conn, const char* table_name, const char* column_name, char* data){
    
// }

// void change_string_in_table(MYSQL* conn, const char* table_name, const char* column_name, char* data){
    
// }

// void remove_int_from_table(MYSQL* conn, const char* table_name, const char* column_name, int32_t* data){
//     char mysql_string[MAX_SQL_STRING];
//     sprintf(mysql_string, "DELETE FROM %s WHERE (%s=%d)", table_name, column_name, *data);
//     mysql_query(conn, &mysql_string[0]);
// }

// void remove_string_from_table(MYSQL* conn, const char* table_name, const char* column_name, char* data){
    
// }

// void show_database_tables(MYSQL* conn){
//     MYSQL_RES *res;
//     MYSQL_ROW row;
//     /* send SQL query */
//     if (mysql_query(conn, "show tables")) {
//         fprintf(stderr, "%s\n", mysql_error(conn));
//         exit(1);
//     }

//     res = mysql_use_result(conn);

//     /* output table name */
//     printf("MySQL Tables in mysql database:\n");

//     while ((row = mysql_fetch_row(res)) != NULL)
//         printf("%s \n", row[0]);

//     /* close connection */
//     mysql_free_result(res);
//     mysql_close(conn);
// }

// uint8_t load_linked_device_list(struct system_struct* system){
//     MYSQL_RES* result;
//     MYSQL_ROW row;
//     uint32_t count;
//     unsigned long* length;
    
//     // remove any null device id's from table... there shouldn't be, but just to be safe
//     if(mysql_query(&system->database.conn, "DELETE FROM linked_devices WHERE device_id IS NULL")){
//         return 1;
//     }
//     // list device id's and save to linked device list
//     if(mysql_query(&system->database.conn, "SELECT device_id FROM linked_devices")){
//         return 1;
//     }
//     system->linked_devices.device_count = 0;
//     result = mysql_use_result(&system->database.conn);
//     row = mysql_fetch_row(result);
//     while(row != NULL){
//         sscanf((char*) row[0], "%d", &system->linked_devices.device[system->linked_devices.device_count].device_id);
//         system->linked_devices.device[system->linked_devices.device_count].is_valid = 1;
//         system->linked_devices.device_count = system->linked_devices.device_count + 1;
//         row = mysql_fetch_row(result);
//     }
//     mysql_query(&system->database.conn, "SELECT device_name FROM linked_devices");
//     result = mysql_use_result(&system->database.conn);
//     row = mysql_fetch_row(result);
//     count = 0;
//     while(row != NULL){
//         length = mysql_fetch_lengths(result);
//         if(*length){
//             strcpy(&system->linked_devices.device[count].device_name[0], (char*) row[0]);
//         } else {
//             strcpy(&system->linked_devices.device[count].device_name[0], "UnNamed Device");
//         }
//         count++;
//         row = mysql_fetch_row(result);
//     }
//     return 0;
// }

// void add_device_to_db(MYSQL* conn, struct linked_device_struct* device){
//     char mysql_string[MAX_SQL_STRING];
//     sprintf(mysql_string, "INSERT INTO linked_devices (device_name, device_id) VALUES ('%s', %d)", device->device_name, device->device_id);
//     mysql_query(conn, &mysql_string[0]);
// }

// void update_device_in_db(MYSQL* conn, struct linked_device_struct* device){
//     char mysql_string[MAX_SQL_STRING];
//     sprintf(mysql_string, "UPDATE linked_devices SET device_name='%s' WHERE device_id=%d", device->device_name, device->device_id);
//     mysql_query(conn, &mysql_string[0]);
// }

// void remove_device_from_db(MYSQL* conn, struct linked_device_struct* device){
//     char mysql_string[MAX_SQL_STRING];
//     sprintf(mysql_string, "DELETE FROM linked_devices WHERE (device_id=%d)", device->device_id);
//     mysql_query(conn, &mysql_string[0]);
// }