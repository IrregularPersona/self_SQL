// debug.h
#ifndef DEBUG_H
#define DEBUG_H

#ifdef ENABLE_DEBUG
    #define DEBUG_PRINT(...) fprintf(stderr, "DEBUG: " __VA_ARGS__)
    #define DEBUG_TOKEN(token) printf("Token: %s (%s)\n", token->value, token_type_to_string(token->type))
    #define DEBUG_TOKENS(tokens) \
        do { \
            printf("Tokens:\n"); \
            for (size_t j = 0; tokens[j] != NULL; j++) { \
                printf("  [%zu]: %s (%s)\n", j, tokens[j]->value, token_type_to_string(tokens[j]->type)); \
            } \
        } while(0)
#else
    #define DEBUG_PRINT(...) ((void)0)
    #define DEBUG_TOKEN(token) ((void)0)
    #define DEBUG_TOKENS(tokens) ((void)0)
#endif

#endif // DEBUG_H
