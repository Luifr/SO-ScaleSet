#include <stdio.h>
#include <stdlib.h>

#define FILE_NAME "inputRand.in"
#define FILE_LENGTH 1024
#define MAX_INPUT 10000

int GenerateInput () {

    FILE* input;
    input = fopen(FILE_NAME, "w");

    for (int i = 0; i < FILE_LENGTH; ++i) {
       fprintf(input, "%d\n", rand() % MAX_INPUT);
    }

    fclose(input);

}

int main() {
    GenerateInput();
}
