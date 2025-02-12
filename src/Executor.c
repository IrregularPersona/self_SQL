// executor
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Token.h"
#include "Executor.h"
#include "Parser.h"
#include "Database.h"

void execute_create_table(Database *db, CreateTableStatement *stmt) {
    if (create_table(db, stmt->table_name, stmt->columns, stmt->column_count)) {
        printf("Table %s created successfully\n", stmt->table_name);
        db_save(db);
    } else {
        fprintf(stderr, "Failed to create table %s\n", stmt->table_name);
    }
}

void execute_insert(Database *db, InsertStatement *stmt) {
    if (insert_into_table(db, stmt->table_name, (const char**)stmt->values, stmt->count)) {
        printf("Inserted data into %s successfully\n", stmt->table_name);
        db_save(db);
    } else {
        fprintf(stderr, "Failed to insert data into %s\n", stmt->table_name);
    }
}

void execute_select(Database *db, SelectStatement *stmt) {
    for (size_t i = 0; i < db->table_count; i++) {
        if (strcmp(db->tables[i].schema.name, stmt->table) == 0) {
            Table *table = &db->tables[i];
            
            // Print column headers
            for (size_t j = 0; j < table->schema.column_count; j++) {
                printf("%-15s", table->schema.columns[j].name);
            }
            printf("\n");

            // Print rows
            for (size_t j = 0; j < table->row_count; j++) {
                Row *row = &table->rows[j];
                for (size_t k = 0; k < row->value_count; k++) {
                    switch (table->schema.columns[k].type) {
                        case TYPE_INT:
                            printf("%-15d", *(int32_t*)row->values[k]);
                            break;
                        case TYPE_TEXT:
                            printf("%-15s", (char*)row->values[k] ? (char*)row->values[k] : "NULL");
                            break;
                        case TYPE_FLOAT:
                            printf("%-15.2f", *(float*)row->values[k]);
                            break;
                        case TYPE_BOOL:
                            printf("%-15s", *(bool*)row->values[k] ? "true" : "false");
                            break;
                        case TYPE_DATE:
                            printf("%-15s", (char*)row->values[k] ? (char*)row->values[k] : "NULL");
                            break;
                    }
                }
                printf("\n");
            }
            return;
        }
    }
    fprintf(stderr, "Table %s not found\n", stmt->table);
}
