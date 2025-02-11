#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
    TOKEN_SELECT,
    TOKEN_FROM,
    TOKEN_WHERE,
    TOKEN_INSERT,
    TOKEN_UPDATE,
    TOKEN_DELETE,
    TOKEN_STAR,
    TOKEN_EQ,
    TOKEN_COMMA,
    TOKEN_IDENTIFIER,
    TOKEN_INTEGER,
    TOKEN_TEXT,
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

#endif // Token.h
