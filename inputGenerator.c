#include <stdio.h>
#include <stdlib.h>

#define FILE_NAME "input.in"
#define FILE_LENGTH 256
#define MAX_INPUT 1024

int GenerateInput () {

    FILE* input;
    input = fopen(FILE_NAME, "w");

    for (int i = 0; i < FILE_LENGTH; ++i) {
       fprintf(input, "%d\n", rand() % MAX_INPUT);
    }

    fclose(input);

}