#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
    TOKEN_CREATE,
    TOKEN_TABLE,
    TOKEN_SELECT,
    TOKEN_FROM,
    TOKEN_WHERE,
    TOKEN_INSERT,
    TOKEN_INTO,
    TOKEN_VALUES,
    TOKEN_UPDATE,
    TOKEN_DELETE,
    TOKEN_STAR,
    TOKEN_EQ,
    TOKEN_COMMA,
    TOKEN_IDENTIFIER,
    TOKEN_INTEGER,
    TOKEN_TEXT,
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_PRIMARY,
    TOKEN_KEY,
    TOKEN_NOT,
    TOKEN_NULL,
    TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    char* value;
} Token;

Token* create_token(TokenType type, const char *value);
Token **tokenize(const char* input);
const char* token_type_to_string(TokenType type);
void free_tokens(Token **token);
void free_token(Token *token); 

#endif // Token.h
