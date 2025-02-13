// main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "Token.h"
#include "Parser.h"
#include "Executor.h"
#include "Database.h"
#include "debug.h"

int main() {
    char input[1024];
    Database *db = db_load("mydb.sdb");
    if (!db) {
        db = db_create("mydb.sdb");
    }

    for (;;) {
        write(1, "sql> ", 5);
        ssize_t char_count = read(0, input, sizeof(input) - 1);
        
        if (char_count == -1) {
            perror("read");
            break;
        }
        
        if (char_count == 0) {
            printf("\nExiting SQL Shell\n");
            break;
        }
        
        input[char_count - 1] = '\0';

        Token **tokens = tokenize(input);
        if (tokens == NULL) {
            fprintf(stderr, "Error tokenizing input\n");
            continue;
        }
        for (size_t j = 0; tokens[j] != NULL; j++) {
            printf("Token[%zu]: %s (%s)\n", j, tokens[j]->value, token_type_to_string(tokens[j]->type));
        }

        if (tokens[0]->type == TOKEN_SELECT) {
            SelectStatement *stmt = parse_select(tokens);
            if (stmt) {
                execute_select(db, stmt);
                free_select_statement(stmt);
            }
        } else if (tokens[0]->type == TOKEN_CREATE) {
            CreateTableStatement *stmt = parse_create_table(tokens);
            if (stmt) {
                execute_create_table(db, stmt);
                free_create_table_statement(stmt);
            }
        } else if (tokens[0]->type == TOKEN_INSERT) {
            InsertStatement *stmt = parse_insert(db, tokens);
            if (stmt) {
                execute_insert(db, stmt);
                free_insert_statement(stmt);
            }
        }

        // Free the tokens
        for (int i = 0; tokens[i] != NULL; i++) {
            free_token(tokens[i]);
        }
        free(tokens);
    }

    db_close(db);
    return 0;
}
