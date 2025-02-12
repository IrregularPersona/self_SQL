#include "Database.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_TABLE_CAP 10;
#define INITIAL_ROW_CAP 100;

Database* db_create(const char* filename) {
    Database* db = (Database *)malloc(sizeof(Database));

    if (db == NULL) {
        fprintf(stderr, "ERROR: Failure to create Database!\n");
        return NULL;
    }

    db->tables = NULL;
    db->table_count = 0;
    db->db_filename = strdup(filename);

    return db;
}

bool db_save(Database* db) {
    FILE* file = fopen(db->db_filename, "wb");
    if (!file) return false;
    
    fwrite(&db->table_count, sizeof(size_t), 1, file);
    for (size_t i = 0; i < db->table_count; i++) {
        Table* table = &db->tables[i];

        size_t name_len = strlen(table->schema.name);
        fwrite(&name_len, sizeof(size_t), 1, file);
        fwrite(table->schema.name, 1, name_len, file);

        fwrite(&table->schema.column_count, sizeof(size_t), 1, file);
        for (size_t j = 0; j < table->schema.column_count; j++) {
            Column* col = &table->schema.columns[j];
            name_len = strlen(col->name);
            fwrite(&name_len, sizeof(size_t), 1, file);
            fwrite(col->name, 1, name_len, file);
            fwrite(&col->type, sizeof(ColumnType), 1, file);
            fwrite(&col->is_prim_key, sizeof(bool), 1, file);
            fwrite(&col->is_nullable, sizeof(bool), 1, file);
        }

        fwrite(&table->row_count, sizeof(size_t), 1, file);
        for (size_t j = 0; j < table->row_count; j++) {
            Row* row = &table->rows[j];
            for (size_t k = 0; k < row->value_count; k++) {
                switch (table->schema.columns[k].type) {
                    case TYPE_INT: {
                        int32_t val = *(int32_t *)row->values[k];
                        fwrite(&val, sizeof(int32_t), 1, file);
                        break;        
                    }
                    case TYPE_TEXT: {
                        char* str = (char*)row->values[k];
                        size_t str_len = str ? strlen : 0;
                        fwrite(&str_len, sizeof(size_t), 1, file);
                        if (str_len > 0) {
                            fwrite(str, 1, str_len, file);
                        }

                        break;
                    }
                    case TYPE_FLOAT: {
                        float val = *(float *)row->values[k];
                        fwrite(&val, sizeof(float), 1, file);
                        break;        
                    }
                    case TYPE_BOOL: {
                        bool val = (bool*)row->values[k];
                        fwrite(&val, sizeof(bool), 1, file);       
                    }
                    // todo: add date type
                }
            }
        }
    }

    fclose(file);
    return true;
}

Database* db_load(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) return NULL;

    Database* db = db_create(filename);

    fread(&db->table_count, sizeof(size_t), 1, file);
    db->tables = malloc(sizeof(Table) * db->table_count);

    for (size_t i = 0; i < db->table_count; i++) {
        Table* table = &db->tables[i];

        size_t name_len;
        fread(&name_len, sizeof(size_t), 1, file);
        table->schema.name = malloc(name_len + 1);
        fread(table->schema.name, 1, name_len, file);
        table->schema.name[name_len] = '\0';

        fread(&table->schema.column_count, sizeof(size_t), 1, file);
        table->schema.columns = malloc(sizeof(Column) * table->schema.column_count);
        
        for (size_t j = 0; j < table->schema.column_count; j++) {
            Column* col = &table->schema.columns[j];
            
            // Read column name
            fread(&name_len, sizeof(size_t), 1, file);
            col->name = malloc(name_len + 1);
            fread(col->name, 1, name_len, file);
            col->name[name_len] = '\0';

            // Read column properties
            fread(&col->type, sizeof(ColumnType), 1, file);
            fread(&col->is_prim_key, sizeof(bool), 1, file);
            fread(&col->is_nullable, sizeof(bool), 1, file);
        }

        // Read rows
        fread(&table->row_count, sizeof(size_t), 1, file);
        table->capacity = table->row_count > 0 ? table->row_count : INITIAL_ROW_CAP;
        table->rows = malloc(sizeof(Row) * table->capacity);

        // Read row data
        for (size_t j = 0; j < table->row_count; j++) {
            Row* row = &table->rows[j];
            row->value_count = table->schema.column_count;
            row->values = malloc(sizeof(void*) * row->value_count);

            for (size_t k = 0; k < row->value_count; k++) {
                // Read value based on column type
                switch (table->schema.columns[k].type) {
                    case TYPE_INT: {
                        int32_t* val = malloc(sizeof(int32_t));
                        fread(val, sizeof(int32_t), 1, file);
                        row->values[k] = val;
                        break;
                    }
                    case TYPE_TEXT: {
                        size_t str_len;
                        fread(&str_len, sizeof(size_t), 1, file);
                        if (str_len > 0) {
                            char* str = malloc(str_len + 1);
                            fread(str, 1, str_len, file);
                            str[str_len] = '\0';
                            row->values[k] = str;
                        } else {
                            row->values[k] = NULL;
                        }
                        break;
                    }
                    case TYPE_FLOAT: {
                        float* val = malloc(sizeof(float));
                        fread(val, sizeof(float), 1, file);
                        row->values[k] = val;
                        break;        
                    }
                    case TYPE_BOOL: {
                        bool* val = malloc(sizeof(bool));
                        fread(val, sizeof(bool), 1, file);
                        row->values[k] = val;
                        break;
                    }
                }
            }
        }
    }

    fclose(file);
    return db;
}

bool create_table(Database* db, const char* table_name, Column* columns, size_t column_count) {
    // Expand tables array
    size_t new_idx = db->table_count;
    db->tables = realloc(db->tables, sizeof(Table) * (db->table_count + 1));
    if (!db->tables) return false;
    
    // Initialize new table
    Table* table = &db->tables[new_idx];
    table->schema.name = strdup(table_name);
    table->schema.columns = malloc(sizeof(Column) * column_count);
    table->schema.column_count = column_count;
    
    // Copy column definitions
    for (size_t i = 0; i < column_count; i++) {
        table->schema.columns[i] = columns[i];
        table->schema.columns[i].name = strdup(columns[i].name);
    }
    
    // Initialize rows
    table->rows = NULL;
    table->row_count = 0;
    table->capacity = INITIAL_ROW_CAP;
    table->rows = malloc(sizeof(Row) * table->capacity);
    
    if (!table->rows) {
        // Cleanup on failure
        for (size_t i = 0; i < column_count; i++) {
            free(table->schema.columns[i].name);
        }
        free(table->schema.columns);
        free(table->schema.name);
        return false;
    }
    
    db->table_count++;
    return true;
}

bool insert_into_table(Database* db, const char* table_name, const char** values, size_t value_count) {
    // Find the table
    Table* table = NULL;
    for (size_t i = 0; i < db->table_count; i++) {
        if (strcmp(db->tables[i].schema.name, table_name) == 0) {
            table = &db->tables[i];
            break;
        }
    }
    
    if (!table) return false;
    
    // Validate column count
    if (value_count != table->schema.column_count) return false;
    
    // Check if we need to expand the rows array
    if (table->row_count >= table->capacity) {
        size_t new_capacity = table->capacity * 2;
        Row* new_rows = realloc(table->rows, sizeof(Row) * new_capacity);
        if (!new_rows) return false;
        
        table->rows = new_rows;
        table->capacity = new_capacity;
    }
    
    // Create new row
    Row* row = &table->rows[table->row_count];
    row->values = malloc(sizeof(void*) * value_count);
    row->value_count = value_count;
    
    // Parse and store values
    for (size_t i = 0; i < value_count; i++) {
        ColumnType type = table->schema.columns[i].type;
        row->values[i] = parse_value(values[i], type);
        
        if (!row->values[i] && !table->schema.columns[i].is_nullable) {
            // Cleanup on failure
            for (size_t j = 0; j < i; j++) {
                free_value(row->values[j], table->schema.columns[j].type);
            }
            free(row->values);
            return false;
        }
    }
    
    table->row_count++;
    return true;
}

void db_close(Database* db) {
    if (!db) return;
    
    // Free all tables
    for (size_t i = 0; i < db->table_count; i++) {
        Table* table = &db->tables[i];
        
        // Free schema
        for (size_t j = 0; j < table->schema.column_count; j++) {
            free(table->schema.columns[j].name);
        }
        free(table->schema.columns);
        free(table->schema.name);
        
        // Free rows
        for (size_t j = 0; j < table->row_count; j++) {
            Row* row = &table->rows[j];
            for (size_t k = 0; k < row->value_count; k++) {
                free_value(row->values[k], table->schema.columns[k].type);
            }
            free(row->values);
        }
        free(table->rows);
    }
    
    free(db->tables);
    free(db->db_filename);
    free(db);
}

ColumnType parse_column_type(const char* type_str) {
    if (strcasecmp(type_str, "INT") == 0) return TYPE_INT;
    if (strcasecmp(type_str, "TEXT") == 0) return TYPE_TEXT;
    if (strcasecmp(type_str, "FLOAT") == 0) return TYPE_FLOAT;
    if (strcasecmp(type_str, "BOOL") == 0) return TYPE_BOOL;
    if (strcasecmp(type_str, "DATE") == 0) return TYPE_DATE;
    return TYPE_TEXT; // Default to TEXT
}

void* parse_value(const char* value, ColumnType type) {
    if (!value) return NULL;
    
    switch (type) {
        case TYPE_INT: {
            int32_t* val = malloc(sizeof(int32_t));
            *val = atoi(value);
            return val;
        }
        case TYPE_TEXT: {
            return strdup(value);
        }
        case TYPE_FLOAT: {
            float* val = malloc(sizeof(float));
            *val = atof(value);
            return val;
        }
        case TYPE_BOOL: {
            bool* val = malloc(sizeof(bool));
            *val = (strcasecmp(value, "true") == 0 || strcmp(value, "1") == 0);
            return val;
        }
        case TYPE_DATE: {
            // Simple date storage as string for now
            return strdup(value);
        }
        default:
            return NULL;
    }
}

void free_value(void* value, ColumnType type) {
    if (!value) return;
    
    switch (type) {
        case TYPE_TEXT:
            free(value);
            break;
        default:
            free(value);
            break;
    }
}
