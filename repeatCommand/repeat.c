#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int args, char* argv[]) {
    size_t bufferSize = 0;
    for (int i = 1; i < args; i++) {
        bufferSize += strlen(argv[i]) + 1; // +1 for spaces and newlines
    }

    char* buffer = malloc(bufferSize);
    if (!buffer) {
        perror("Allocation Failed.");
        return 1;
    }

    buffer[0] = '\0';

    for (int i = 1; i < args; i++) {
        strncat(buffer, argv[i], bufferSize - strlen(buffer) - 1);
        if (i < args - 1) {
            // basically add spaces between arguments
            strncat(buffer, " ", bufferSize - strlen(buffer) - 1);
        }
    }

    char* savePtr;
    char* token = strtok_r(buffer, " ", &savePtr);
    char* text = NULL;

    char* commandAmount = "-n";
    char* commandText = "-t";

    int count = 0;
    while (token) {
        if (!strncmp(token, commandAmount, strlen(commandAmount))) {
            token = strtok_r(NULL, " ", &savePtr);
            if (token) {
                // count = atoi(token);
                char* endPtr;
                long long tempCount = strtoll(token, &endPtr, 10);
                
                if (*endPtr != '\0') {
                    fprintf(stderr, "Invalid Count %s\n", token);
                    free(buffer);
                    return 1;
                }

                if (tempCount <= 0 || tempCount >= 100) {
                    fprintf(stderr, "Cannot output %s %d times\n", text, count);
                    free(buffer);
                    return 1;
                }

                count = (int)tempCount;
            }
        } else if (!strncmp(token, commandText, strlen(commandText))) {
            text = savePtr; // everything after -t is output text
            break;
        }

        token = strtok_r(NULL, " ", &savePtr);
    }

    if (!text) {
        fprintf(stderr, "Usage: %s -n <count> -t <text>\n", argv[0]);
        free(buffer);
        free(text);
        return 1;
    } 

    for (int i = 0; i < count; i++) {
        printf("%s\n", text);
    }

    free(buffer);
    return 0;
}
