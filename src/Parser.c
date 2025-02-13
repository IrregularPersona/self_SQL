// parser
#include "Parser.h"
#include "Token.h"
#include "debug.h"

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SelectStatement *create_select_statement() {
    SelectStatement *stmt = (SelectStatement *)malloc(sizeof(SelectStatement));
    if (stmt == NULL) {
        fprintf(stderr, "Error: Failed creating select statement!\n");
        return NULL;
    }

    DEBUG_PRINT("Created new SelectStatement\n");
    stmt->columns = NULL;
    stmt->column_count = 0;
    stmt->table = NULL;
    stmt->where_column = NULL;
    stmt->where_value = NULL;
    return stmt;
}

void free_select_statement(SelectStatement *stmt) {
    if (stmt) {
        DEBUG_PRINT("Freeing SelectStatement\n");
        free(stmt->table);
        free(stmt->where_column);
        free(stmt->where_value);
        for (size_t i = 0; i < stmt->column_count; i++) {
            free(stmt->columns[i]);
        }
        free(stmt->columns);
        free(stmt);
    }
}

SelectStatement *parse_select(Token **tokens) {
    DEBUG_PRINT("Starting SELECT statement parsing\n");
    DEBUG_TOKENS(tokens);

    if (!tokens || !tokens[0]) {
        fprintf(stderr, "ERROR: Token list is empty or NULL!\n");
        return NULL;
    }

    SelectStatement *stmt = create_select_statement();
    if (!stmt) {
        return NULL;  // Memory allocation failed
    }

    size_t i = 0;

    // Ensure the first token is SELECT
    if (!tokens[i] || tokens[i]->type != TOKEN_SELECT) {
        fprintf(stderr, "ERROR: Expects SELECT!\n");
        free_select_statement(stmt);
        return NULL;
    }
    i++;

    // Process column names or *
    while (tokens[i] && (tokens[i]->type == TOKEN_IDENTIFIER || tokens[i]->type == TOKEN_STAR)) {
        DEBUG_PRINT("Adding column: %s\n", tokens[i]->value);
        stmt->columns = realloc(stmt->columns, sizeof(char *) * (stmt->column_count + 1));
        if (!stmt->columns) {
            fprintf(stderr, "ERROR: Memory allocation failed for columns\n");
            free_select_statement(stmt);
            return NULL;
        }
        stmt->columns[stmt->column_count++] = strdup(tokens[i]->value);
        i++;
    }

    // Expect FROM keyword
    if (!tokens[i] || tokens[i]->type != TOKEN_FROM) {
        fprintf(stderr, "ERROR: Expected FROM!\n");
        free_select_statement(stmt);
        return NULL;
    }
    i++;

    // Expect table name
    if (!tokens[i] || tokens[i]->type != TOKEN_IDENTIFIER) {
        fprintf(stderr, "ERROR: Expected Table Identifier!\n");
        free_select_statement(stmt);
        return NULL;
    }
    stmt->table = strdup(tokens[i]->value);
    DEBUG_PRINT("Setting table name: %s\n", stmt->table);
    i++;

    // Handle optional WHERE clause
    if (tokens[i] && tokens[i]->type == TOKEN_WHERE) {
        DEBUG_PRINT("Processing WHERE clause\n");
        i++;

        if (!tokens[i] || tokens[i]->type != TOKEN_IDENTIFIER) {
            fprintf(stderr, "ERROR: Expected column name after WHERE!\n");
            free_select_statement(stmt);
            return NULL;
        }
        stmt->where_column = strdup(tokens[i]->value);
        DEBUG_PRINT("WHERE column: %s\n", stmt->where_column);
        i++;

        if (!tokens[i] || tokens[i]->type != TOKEN_EQ) {
            fprintf(stderr, "ERROR: Expected '=' in WHERE clause!\n");
            free_select_statement(stmt);
            return NULL;
        }
        i++;

        if (!tokens[i] || (tokens[i]->type != TOKEN_INTEGER && tokens[i]->type != TOKEN_TEXT)) {
            fprintf(stderr, "ERROR: Expected value in WHERE clause!\n");
            free_select_statement(stmt);
            return NULL;
        }
        stmt->where_value = strdup(tokens[i]->value);
        DEBUG_PRINT("WHERE value: %s\n", stmt->where_value);
        i++;
    }

    // Ensure no extra tokens at the end
    if (!tokens[i] || tokens[i]->type != TOKEN_EOF) {
        fprintf(stderr, "ERROR: Unexpected tokens at the end!\n");
        free_select_statement(stmt);
        return NULL;
    }

    DEBUG_PRINT("Successfully parsed SELECT statement\n");
    return stmt;
}

CreateTableStatement *parse_create_table(Token **tokens) {
    DEBUG_PRINT("Starting CREATE TABLE statement parsing\n");
    // DEBUG_TOKENS(tokens);
    
    CreateTableStatement *stmt = malloc(sizeof(CreateTableStatement));
    size_t i = 0;
    
    // Skip CREATE TABLE
    if (tokens[i]->type != TOKEN_CREATE || tokens[i + 1]->type != TOKEN_TABLE) {
        fprintf(stderr, "ERROR: Expected CREATE TABLE!\n");
        free(stmt);
        return NULL;
    }
    i += 2;

    // Get table name
    if (tokens[i]->type != TOKEN_IDENTIFIER) {
        fprintf(stderr, "ERROR: Expected table name!\n");
        free(stmt);
        return NULL;
    }
    stmt->table_name = strdup(tokens[i]->value);
    DEBUG_PRINT("Creating table: %s\n", stmt->table_name);
    i++;

    // Expect left parenthesis
    if (tokens[i]->type != TOKEN_LEFT_PAREN) {
        fprintf(stderr, "ERROR: Expected '(' after table name!\n");
        free(stmt->table_name);
        free(stmt);
        return NULL;
    }
    i++;

    // Parse columns
    stmt->columns = NULL;
    stmt->column_count = 0;

    while (tokens[i]->type != TOKEN_RIGHT_PAREN) {
        stmt->columns = realloc(stmt->columns, sizeof(Column) * (stmt->column_count + 1));
        Column *col = &stmt->columns[stmt->column_count];

        // Get column name
        if (tokens[i]->type != TOKEN_IDENTIFIER) {
            fprintf(stderr, "ERROR: Expected column name!\n");
            goto error;
        }
        col->name = strdup(tokens[i]->value);
        DEBUG_PRINT("Adding column: %s\n", col->name);
        i++;

        // Get column type
        if (tokens[i]->type != TOKEN_IDENTIFIER) {
            fprintf(stderr, "ERROR: Expected column type!\n");
            goto error;
        }
        col->type = parse_column_type(tokens[i]->value);
        DEBUG_PRINT("Column type: %d\n", col->type);
        i++;

        // Set default constraints
        col->is_prim_key = false;
        col->is_nullable = true;

        // Check for optional constraints
        while (tokens[i]->type != TOKEN_COMMA && tokens[i]->type != TOKEN_RIGHT_PAREN) {
            if (tokens[i]->type == TOKEN_PRIMARY) {
                if (tokens[i + 1] && tokens[i + 1]->type == TOKEN_KEY) {
                    col->is_prim_key = true;
                    DEBUG_PRINT("Column %s set as PRIMARY KEY\n", col->name);
                    i += 2;
                } else {
                    fprintf(stderr, "ERROR: PRIMARY must be followed by KEY!\n");
                    goto error;
                }
            } else if (tokens[i]->type == TOKEN_NOT) {
                if (tokens[i + 1] && tokens[i + 1]->type == TOKEN_NULL) {
                    col->is_nullable = false;
                    DEBUG_PRINT("Column %s set as NOT NULL\n", col->name);
                    i += 2;
                } else {
                    fprintf(stderr, "ERROR: NOT must be followed by NULL!\n");
                    goto error;
                }
            } else {
                fprintf(stderr, "ERROR: Unexpected token in column definition!\n");
                goto error;
            }
        }

        stmt->column_count++;

        if (tokens[i]->type == TOKEN_COMMA) {
            i++;
            if (tokens[i]->type == TOKEN_RIGHT_PAREN) {
                fprintf(stderr, "ERROR: Trailing comma in column definition!\n");
                goto error;
            }
        }
    }

    if (tokens[i]->type == TOKEN_RIGHT_PAREN) {
        i++;
    } else {
        fprintf(stderr, "ERROR: Expected ')' at the end of column definitions!\n");
        goto error;
    }

    DEBUG_PRINT("Successfully parsed CREATE TABLE statement with %zu columns\n", stmt->column_count);
    return stmt;

error:
    DEBUG_PRINT("Error occurred while parsing CREATE TABLE statement\n");
    for (size_t j = 0; j < stmt->column_count; j++) {
        free(stmt->columns[j].name);
    }
    free(stmt->columns);
    free(stmt->table_name);
    free(stmt);
    return NULL;
}

InsertStatement *parse_insert(Database* db, Token **tokens) {
    DEBUG_PRINT("Starting INSERT statement parsing\n");
    DEBUG_TOKENS(tokens);
    
    InsertStatement *stmt = malloc(sizeof(InsertStatement));
    size_t i = 0;
    
    if (tokens[i]->type != TOKEN_INSERT || tokens[i + 1]->type != TOKEN_INTO) {
        fprintf(stderr, "ERROR: Expected INSERT INTO!\n");
        free(stmt);
        return NULL;
    }
    i += 2;

    if (tokens[i]->type != TOKEN_IDENTIFIER) {
        fprintf(stderr, "ERROR: Expected table name!\n");
        free(stmt);
        return NULL;
    }
    stmt->table_name = strdup(tokens[i]->value);
    DEBUG_PRINT("Inserting into table: %s\n", stmt->table_name);
    i++;

    stmt->column_names = NULL;
    stmt->values = NULL;
    stmt->count = 0;

    if (tokens[i]->type == TOKEN_LEFT_PAREN) {
        DEBUG_PRINT("Processing column list\n");
        i++;
        if (tokens[i]->type == TOKEN_IDENTIFIER) {
            while (tokens[i]->type != TOKEN_RIGHT_PAREN) {
                stmt->column_names = realloc(stmt->column_names, sizeof(char*) * (stmt->count + 1));
                if (tokens[i]->type != TOKEN_IDENTIFIER) {
                    fprintf(stderr, "ERROR: Expected column name!\n");
                    goto error;
                }
                stmt->column_names[stmt->count] = strdup(tokens[i]->value);
                DEBUG_PRINT("Column %zu: %s\n", stmt->count, stmt->column_names[stmt->count]);
                stmt->count++;
                i++;

                if (tokens[i]->type == TOKEN_COMMA) {
                    i++;
                }
            }
            i++;
        }
    }

    if (tokens[i]->type != TOKEN_VALUES) {
        fprintf(stderr, "ERROR: Expected VALUES!\n");
        goto error;
    }
    i++;

    if (tokens[i]->type != TOKEN_LEFT_PAREN) {
        fprintf(stderr, "ERROR: Expected '(' after VALUES!\n");
        goto error;
    }
    i++;

    size_t value_count = 0;
    if (stmt->count == 0) {
        Table *table = NULL;
        for (size_t j = 0; j < db->table_count; j++) {
            if (strcmp(db->tables[j].schema.name, stmt->table_name) == 0) {
                table = &db->tables[j];
                break;
            }
        }

        if (!table) {
            fprintf(stderr, "ERROR: Table %s not found!\n", stmt->table_name);
            goto error;
        }

        stmt->count = table->schema.column_count;
        DEBUG_PRINT("Using all columns from table (count: %zu)\n", stmt->count);
    }
    
    stmt->values = malloc(sizeof(char*) * stmt->count);
    DEBUG_PRINT("Processing values\n");
    
    while (tokens[i]->type != TOKEN_RIGHT_PAREN) {
        if (value_count >= stmt->count) {
            fprintf(stderr, "ERROR: Too many values!\n");
            goto error;
        }

        if (tokens[i]->type != TOKEN_INTEGER && tokens[i]->type != TOKEN_TEXT) {
            fprintf(stderr, "ERROR: Expected value!\n");
            goto error;
        }
        stmt->values[value_count] = strdup(tokens[i]->value);
        DEBUG_PRINT("Value %zu: %s\n", value_count, stmt->values[value_count]);
        value_count++;
        i++;

        if (tokens[i]->type == TOKEN_COMMA) {
            i++;
        }
    }

    if (stmt->count == 0) {
        stmt->count = value_count;
    }

    DEBUG_PRINT("Expected columns: %zu, Provided values: %zu\n", stmt->count, value_count);

    if (value_count != stmt->count) {
        fprintf(stderr, "ERROR: Value count doesn't match column count!\n");
        goto error;
    }

    DEBUG_PRINT("Successfully parsed INSERT statement\n");
    return stmt;

error:
    DEBUG_PRINT("Error occurred while parsing INSERT statement\n");
    for (size_t j = 0; j < stmt->count; j++) {
        free(stmt->column_names[j]);
        if (stmt->values && stmt->values[j]) {
            free(stmt->values[j]);
        }
    }
    free(stmt->column_names);
    free(stmt->values);
    free(stmt->table_name);
    free(stmt);
    return NULL;
}

void free_create_table_statement(CreateTableStatement *stmt) {
    if (!stmt) return;
    
    DEBUG_PRINT("Freeing CreateTableStatement\n");
    free(stmt->table_name);
    for (size_t i = 0; i < stmt->column_count; i++) {
        free(stmt->columns[i].name);
    }
    free(stmt->columns);
    free(stmt);
}

void free_insert_statement(InsertStatement *stmt) {
    if (!stmt) return;
    
    DEBUG_PRINT("Freeing InsertStatement\n");
    free(stmt->table_name);
    for (size_t i = 0; i < stmt->count; i++) {
        if (stmt->column_names) free(stmt->column_names[i]);
        if (stmt->values) free(stmt->values[i]);
    }
    free(stmt->column_names);
    free(stmt->values);
    free(stmt);
}
