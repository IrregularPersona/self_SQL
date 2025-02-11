#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "Token.h"
   
int main() {
    char input[1024];

    for (;;) {
        write(1, "> ", 2);
        ssize_t char_count = read(0, input, 1024);

        if (char_count == -1) {
            perror("read");
            return 1;
        }

        if (char_count == 0) {
            printf("\nExiting Shell\n");
            return 0;
        }
        
        input[char_count - 1] = '\0';
        printf("Input: %s\n", input);
        printf("Length: %ld\n", strlen(input)); 

        Token **tokens = tokenize(input);
        printf("Detected Tokens are:\n");
        for (int i = 0; tokens[i] != NULL; i++) {
            printf("Token: %s\t|| Type: %s\n", tokens[i]->value, token_type_to_string(tokens[i]->type));
        }

        free_tokens(tokens);
    }
        
    return 0;
}
