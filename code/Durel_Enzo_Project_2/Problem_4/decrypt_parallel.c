#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define MAX_LINE_LENGTH 1000000
#define DEBUG 0

#define MAX(a, b) (a > b) ? a : b

/* ------------ Project 2 - Problem 3 - Encryption ------------
   This file encrypts a text file serially.
   Most of the work is done for you.
   Just find what in the program can be parallelized and how to encrypt a character.
   Don't forget to log the output time of your modified code!
*/ // ------------------------------------------------------ //

int count_substring(const char *str, const char *sub) {
    int count = 0;
    int len_sub = strlen(sub);
    
    if (len_sub == 0) return 0; // Edge case: Empty substring

    while ((str = strstr(str, sub)) != NULL) {
        count++;
        str += len_sub;  // Move pointer to avoid counting overlapping occurrences
    }
    
    return count;
}

int main (int argc, char *argv[])
{
    // Catch console errors
    //  Make sure you include the # of threads and your output time file.
    if (argc != 5) {
        printf("USE LIKE THIS: decrypt_parallel input_text.txt key.txt time.txt num_threads\n");
        return EXIT_FAILURE;
    }

    // Read in the encryption key
    int thread_count = strtol(argv[4], NULL, 10);

    // Open the time file
    FILE* timeFile = fopen(argv[3], "w");

    // Open the input, unencrypted text file
    FILE* inputFile = fopen(argv[1], "r");

    FILE* keyFile = fopen(argv[2], "w");

    // Allocate and open a buffer to read in the input
    fseek(inputFile, 0L, SEEK_END);
    long lSize = ftell(inputFile);
    rewind(inputFile);
    char* buffer = calloc(1, lSize + 1);
    if (!buffer)
        fclose(inputFile),
	    fclose(keyFile),
	    fclose(timeFile),
	    free(buffer),
	    fputs("Memory alloc for inputFile1 failed!\n", stderr), 
	    exit(1);

    // Read the input into the buffer
    if(1 != fread(buffer, lSize, 1, inputFile))
        fclose(inputFile),
	    fclose(keyFile),
	    fclose(timeFile),
	    free(buffer),
	    fputs("Failed reading into the input buffer!\n", stderr),
	    exit(2);

    // Allocate a buffer for the encrypted data
    char* encrypted_buffer = NULL;

    // We are interesting in timing the matrix-matrix multiplication only
    // Record the start time
    double start = omp_get_wtime();
    
    // ----> Begin Encryption <----- //
    // Encrypt the buffer into the encrypted_buffer
    long int maximum = 0;
    long int key = -1;
    long int count = 0;

#pragma omp parallel num_threads(thread_count) default(none) shared(thread_count, maximum, lSize, buffer, key) private(encrypted_buffer, count)
    {
	encrypted_buffer =calloc(1, lSize+1);

#pragma omp for
	for (int j = 0; j < 255; j++)
	{

#pragma omp parallel for num_threads(thread_count) default(none) shared(encrypted_buffer, buffer, j, lSize)
	    for (int i = 0; i<lSize; i++) {
		encrypted_buffer[i] = (buffer[i] - j) % 255; // Decrypt a character from the input buffer.
	    }

	    count = count_substring(encrypted_buffer, "the");
#pragma omp critical	    
	    if (count > maximum)
	    {
		maximum = count;
		key = j;
		printf("%ld\n", key);
	    }
	}
    }

    // Record the finish time        
    double end = omp_get_wtime();
    
    // Time calculation (in seconds)
    double time_passed = end - start;

    // Save time to file
    fprintf(timeFile, "%f", time_passed);
    
    if (DEBUG) printf("Values encypted! \n");

    // Print to the output file
    fprintf(keyFile, "%ld", key);

    // Cleanup
    fclose(inputFile);
    fclose(keyFile);
    fclose(timeFile);

    free(encrypted_buffer);
    free(buffer);

    return 0;
} // End main //

/* ---------------------------------------------------------------------------------------------------
   Sources used:
   https://stackoverflow.com/questions/3747086/reading-the-whole-text-file-into-a-char-array-in-c
*/
