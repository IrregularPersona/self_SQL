#ifndef PARSER_H
#define PARSER_H

#include "Token.h"
#include "Database.h"
#include <stdlib.h>

typedef struct {
    char **columns;
    size_t column_count;
    char *table;
    char *where_column;
    char *where_value;
} SelectStatement;

typedef struct {
    char *table_name;
    Column *columns;
    size_t column_count;
} CreateTableStatement;

typedef struct {
    char *table_name;
    char **column_names;
    char **values;
    size_t count;
} InsertStatement;

SelectStatement *parse_select(Token **tokens);
CreateTableStatement *parse_create_table(Token **tokens);
InsertStatement *parse_insert(Database* db, Token **tokens);

void free_select_statement(SelectStatement *stmt);
void free_create_table_statement(CreateTableStatement *stmt);
void free_insert_statement(InsertStatement *stmt);

#endif // Parser.h
