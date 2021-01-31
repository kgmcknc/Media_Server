
// #ifndef SRC_DATABASE_H_
// #define SRC_DATABASE_H_

// #include <stdint.h>
// #include "config.h"
// #include <unistd.h>
// #include <sys/types.h>
// #include <mysql/mysql.h>

// #define MAX_SQL_STRING 512

// struct database_struct {
//     MYSQL conn;
// };

// uint8_t connect_database(MYSQL* conn);
// void close_database(MYSQL* conn);
// void show_database_tables(MYSQL* conn);

// void add_int_to_table(MYSQL* conn, const char* table_name, const char* column_name, int32_t* data);
// void add_string_to_table(MYSQL* conn, const char* table_name, const char* column_name, char* data);
// void change_string_in_table(MYSQL* conn, const char* table_name, const char* column_name, char* data);
// void remove_int_from_table(MYSQL* conn, const char* table_name, const char* column_name, int32_t* data);
// void remove_string_from_table(MYSQL* conn, const char* table_name, const char* column_name, char* data);
// uint8_t load_linked_device_list(struct system_struct* system);
// void add_device_to_db(MYSQL* conn, struct linked_device_struct* device);
// void update_device_in_db(MYSQL* conn, struct linked_device_struct* device);
// void remove_device_from_db(MYSQL* conn, struct linked_device_struct* device);


// #endif //SRC_DATABASE_H_