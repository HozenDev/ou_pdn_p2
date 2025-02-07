#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <omp.h>

#define DEBUG 0

#define MAX(a, b) (a > b) ? a : b

/* ----------- Project 2 - Problem 1 - Matrix Mult -----------

    This file will multiply two matricies.
    Complete the TODOs in order to complete this program.
    Remember to make it parallelized!
*/ // ------------------------------------------------------ //

void print_array(long int * array, int height, int width)
{
    for(int row = 0; row < height; row++) {
        for(int column = 0; column < width; column++) {
            printf("\t%ld ", array[row * width + column]);
        }
        printf("\n");
    }
}

void write_csv(FILE *file, long int max)
{
    fprintf(file, "%ld\n", max);
}

void read_csv(FILE *file, long int* array, int n_row, int n_col)
{
    // Read the CSV file and store the values in the 1D array
	// The values should be stored in row-major order
	// Example: 
	// 1 2 3
	// 4 5 6
	// Should be stored as: 1 2 3 4 5 6
    int i = 0; // line counter
    int j = 0; // column counter
    int readed = 0;
    long int input = 0;

    for (i = 0; i < n_row; i++)
    {
	for (j = 0; j < n_col; j++)
	{
	    readed = fscanf(file, "%ld,", &input);
	    if (readed == EOF)
	    {
		printf("Error: not enough values in the file\n");
		printf("i: %d, j: %d\n", i, j);
		exit(-1);
	    }
	    array[i * n_col + j] = input;
	}
    }
}

void matrix_multiply_second_largest(long int * matrix, long int * vector, int n_row, int n_col, long int * max, long int * sl)
{
    for (int i = 0; i < n_row; i++)
    {
	long int x = 0;
	for (int j = 0; j < n_col; j++)
	{
	    x += matrix[i * n_col + j] * vector[j];
	}

#pragma omp critical
	{
	    if (x > *max) {
		*sl = *max;  // Save the old max as second largest
		*max = x;
	    } else if (x > *sl) {  // Ensure `x` is distinct
		*sl = x;
	    }
	}
    }
}

void array_allocation(long int** array, int n_row, int n_col)
{
	// Allocate a 1D array to store all the elements in the 2D array
	*array = (long int*)malloc((n_row * n_col) * sizeof(long int));
}


int main(int argc, char* argv[])
{
    // Catch console errors
    if (argc != 10)
    {
        printf("USE LIKE THIS: parallel_mult_mat_mat file_A.csv n_row_A n_col_A file_B.csv n_row_B n_col_B result_matrix.csv time.csv num_threads \n");
        return EXIT_FAILURE;
    }

    // Get the input files
    FILE* inputMatrix1 = fopen(argv[1], "r");
    FILE* inputMatrix2 = fopen(argv[4], "r");

    char* p1;
    char* p2;

    // Get matrix 1's dims
    int n_row1 = strtol(argv[2], &p1, 10);
    int n_col1 = strtol(argv[3], &p2, 10);

    // Get matrix 2's dims
    int n_row2 = strtol(argv[5], &p1, 10);
    int n_col2 = strtol(argv[6], &p2, 10);

    // Get num threads
    int thread_count = strtol(argv[9], NULL, 10);

    // Get output files
    FILE* outputFile = fopen(argv[7], "w");
    FILE* outputTime = fopen(argv[8], "w");

    // TODO: malloc the two input matrices and the output matrix
    // Please use long int as the variable type
    long int * mat_1 = NULL;
    long int * mat_2 = NULL;
    array_allocation(&mat_1, n_row1, n_col1);
    array_allocation(&mat_2, n_row2, n_col2);

    // TODO: Parse the input csv files and fill in the input matrices
    read_csv(inputMatrix1, mat_1, n_row1, n_col1);
    read_csv(inputMatrix2, mat_2, n_row2, n_col2);
    
    // We are interesting in timing the matrix-matrix multiplication only
    // Record the start time
    double start = omp_get_wtime();
    
    // TODO: Parallelize the matrix-matrix multiplication
    long int * input_col = NULL;
    long int maximum = 0;
    long int sl = 0;

    printf("row1: %d, col1: %d, row2: %d, col2: %d\n", n_row1, n_col1, n_row2, n_col2);
    
#pragma omp parallel num_threads(thread_count) default(none) shared(sl, maximum, mat_1, mat_2, n_row1, n_row2, n_col1, n_col2) private(input_col)
    {
	input_col = (long int*)malloc(n_row2 * sizeof(long int));
	
#pragma omp for
	for (int i = 0; i < n_col2; i++)
	{
	    for (int j = 0; j < n_row2; j++)
	    {
		input_col[j] = mat_2[j * n_col2 + i];
	    }

	    matrix_multiply_second_largest(mat_1, input_col, n_row1, n_col1, &maximum, &sl);

	    // print_array(mat_out, n_row1, n_col2);
	}

	free(input_col);
	input_col = NULL;
    }
    

    // Record the finish time        
    double end = omp_get_wtime();
    
    // Time calculation (in seconds)
    double time_passed = end - start;

    // Save time to file
    fprintf(outputTime, "%f", time_passed);

    // TODO: save the output matrix to the output csv file
    write_csv(outputFile, sl);

    free(mat_1);
    mat_1 = NULL;
    free(mat_2);
    mat_2 = NULL;

    // Cleanup
    fclose(inputMatrix1);
    fclose(inputMatrix2);
    fclose(outputFile);
    fclose(outputTime);

    return 0;
}

