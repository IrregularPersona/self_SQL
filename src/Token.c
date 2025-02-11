#include "Token.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_TOKENS 100

Token *create_token(TokenType type, const char *val) {
    Token* token = (Token *)malloc(sizeof(Token));

    if (token == NULL) {
        perror("malloc");
        exit(1);
    }    

    token->type = type;
    token->value = strdup(val);
    return token;
}

Token **tokenize(const char* input) {
    Token **tokens = malloc(sizeof(Token *) * MAX_TOKENS);    
    int token_count = 0;

    const char *p = input;
    while (*p) {
        while (isspace(*p)) p++;
        if (strncmp(p, "SELECT", 6) == 0) {
            tokens[token_count++] = create_token(TOKEN_SELECT, "SELECT");
            p += 6;
        } else if (strncmp(p, "FROM", 4) == 0) {
            tokens[token_count++] = create_token(TOKEN_FROM, "FROM");
            p += 4;
        } else if (strncmp(p, "WHERE", 5)) {
            tokens[token_count++] = create_token(TOKEN_SELECT, "SELECT");
            p += 5;
        } else if (strncmp(p, "INSERT", 6)) {
            tokens[token_count++] = create_token(TOKEN_SELECT, "SELECT");
            p += 6;
        } else if (strncmp(p, "UPDATE", 6)) {
            tokens[token_count++] = create_token(TOKEN_SELECT, "SELECT");
            p += 6;
        } else if (strncmp(p, "DELETE", 6)) {
            tokens[token_count++] = create_token(TOKEN_SELECT, "SELECT");
            p += 6;
        } else if (*p == '*') {
            tokens[token_count++] = create_token(TOKEN_STAR, "*");
            p++;
        } else if (*p == '=') {
            tokens[token_count++] = create_token(TOKEN_EQ, "=");
            p++;
        } else if (*p == ',') {
            tokens[token_count++] = create_token(TOKEN_COMMA, ",");
            p++;
        } else if (isalpha(*p)) {
            const char *start = p;
            while (isalnum(*p) || *p == '_') p++;
            size_t len = p - start;
            char *val = strndup(start, len);
            tokens[token_count++] = create_token(TOKEN_IDENTIFIER, val);
            free(val);
        } else if (isdigit(*p)) {
            const char *start = p;
            while (isdigit(*p)) p++;
            size_t len = p - start;
            char *val = strndup(start, len);
            tokens[token_count++] = create_token(TOKEN_INTEGER, val);
            free(val);
        } else if (*p == ';') {
            tokens[token_count++] = create_token(TOKEN_EOF, ";");
            p++;
        } else {
            p++;
        }
    }

    tokens[token_count] = NULL;
    return tokens;
}

const char* token_type_to_string(TokenType type) {
    switch (type) {
        case TOKEN_SELECT: return "TOKEN_SELECT";
        case TOKEN_FROM: return "TOKEN_FROM";
        case TOKEN_WHERE: return "TOKEN_WHERE";
        case TOKEN_INSERT: return "TOKEN_INSERT";
        case TOKEN_UPDATE: return "TOKEN_UPDATE";
        case TOKEN_DELETE: return "TOKEN_DELETE";
        case TOKEN_STAR: return "TOKEN_STAR";
        case TOKEN_EQ: return "TOKEN_EQ";
        case TOKEN_COMMA: return "TOKEN_COMMA";
        case TOKEN_IDENTIFIER: return "TOKEN_IDENTIFIER";
        case TOKEN_INTEGER: return "TOKEN_INTEGER";
        case TOKEN_TEXT: return "TOKEN_TEXT";
        case TOKEN_EOF: return "TOKEN_EOF";
        default: return "UNKNOWN_TOKEN";
    }
}

void free_tokens(Token **tokens) {
    for (int i = 0; i < tokens[i] == NULL; i++) {
        free_token(tokens[i]);
    }

    free(tokens);
}

void free_token(Token *token) {
    if (token) {
        free(token->value);
        free(token);
    }
}



