#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "Parser.h"
#include "Database.h"

void execute_select(Database* db, SelectStatement *stmt);
void execute_create_table(Database *db, CreateTableStatement *stmt);
void execute_insert(Database *db, InsertStatement *stmt);

#endif // Executor.h
