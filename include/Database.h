#ifndef DATABASE_H
#define DATABASE_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef enum {
    TYPE_INT,
    TYPE_TEXT,
    TYPE_FLOAT,
    TYPE_BOOL,
    TYPE_DATE
} ColumnType;

typedef struct {
    char* name;
    ColumnType type;
    bool is_prim_key;
    bool is_nullable;
} Column;

typedef struct {
    char* name;
    Column* columns;
    size_t column_count;
} TableSchema;

typedef struct {
    void** values; // for holding values
    size_t value_count;
} Row;

typedef struct {
    TableSchema schema;
    Row* rows;
    size_t row_count;
    size_t capacity;
} Table;

typedef struct {
    Table* tables;
    size_t table_count;
    char* db_filename;
} Database;

// db functions
Database* db_create(const char* filename);
void db_close(Database* db);
bool db_save(Database* db);
Database* db_load(const char* filename);

// table functions
bool create_table(Database* db, const char* table_name, Column* columns, size_t value_count);
bool insert_into_table(Database* db, const char* table_name, const char** values, size_t value_count);
bool delete_from_table(Database* db, const char* table_name, const char** values, size_t value_count);
void print_table(Database* db, const char* table_name);

// column functions
ColumnType parse_column_type(const char* type_str);
void* parse_value(const char* value, ColumnType type);
void free_value(void* value, ColumnType type);

#endif // Database.h
