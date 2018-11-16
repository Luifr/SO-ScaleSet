#include <stdio.h>
#include <stdlib.h>

#define FILE_NAME "inputRand.in"
#define FILE_LENGTH 24 * 10
#define RAND_PERCENT 15
#define MAX_POWER 4000
#define HOUR_MULTIPLIER {0.19, 0.14, 0.10, 0.07, 0.05, 0.11, 0.19, 0.27, 0.33, 0.48, 0.59, 0.70,\
			 0.74, 0.88, 0.96, 1.00, 1.00, 0.90, 0.94, 0.92, 0.86, 0.70, 0.52, 0.28}
			//0,    1,    2,    3,    4,    5,    6,    7,    8,    9,    10,   11
//#define MAX_INPUT 10000

#define MAX_VALUE(a, b) (((a) > (b)) ? (a) : (b)) // Retuns the max value between the numbers 'a' and 'b'

int GenerateInput () {

	FILE* input;
	input = fopen(FILE_NAME, "w");

	float hourMultiplier[24] = HOUR_MULTIPLIER;
	float maxPercent = 1 - RAND_PERCENT/100;

	float randNumber;
	float totalMultiplier;
	int finalNumber;

	for (int i = 0; i < FILE_LENGTH; ++i) {
		randNumber = (rand() % (RAND_PERCENT * 2) - RAND_PERCENT) / 100.0;
		totalMultiplier = MAX_VALUE(hourMultiplier[(i+13)%24] + randNumber, 0);
		finalNumber = MAX_POWER * totalMultiplier;
		fprintf(input, "%d\n", finalNumber);
	}

	/*for (int i = 0; i < FILE_LENGTH; ++i) {
		fprintf(input, "%d\n", rand() % MAX_INPUT);
	}*/

	fclose(input);
}

int main() {
	GenerateInput();
}
