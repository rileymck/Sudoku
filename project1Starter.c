#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

typedef int bool;
#define TRUE 1
#define FALSE 0

// Global shared data structures
int sudokuPuzzle[9][9];
bool colArray[9], rowArray[9], subgridArray[9];
pthread_t tid[27]; // Array to store all thread IDs

// Function prototypes for thread functions
void *colChecker(void *param);
void *rowChecker(void *param);
void *subgridChecker(void *param);

// Structure to define index range for each thread
typedef struct {
    int topRow, bottomRow, leftColumn, rightColumn;
} parameters;

int main() {
    // Open the Sudoku puzzle file
    FILE *file = fopen("SudokuPuzzle.txt", "r");
    if (!file) {
        printf("Could not open file\n");
        return 1;
    }
    
    // Read puzzle into 2D array
    char line[50];
    for (int i = 0; i < 9; i++) {
        fgets(line, sizeof(line), file);
        char *token = strtok(line, "\t");
        int j = 0;
        while (token) {
            sudokuPuzzle[i][j++] = atoi(token);
            token = strtok(NULL, "\t");
        }
    }
    fclose(file);

    // Print the Sudoku puzzle
    printf("\nSudoku Puzzle:\n");
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            printf("%d ", sudokuPuzzle[i][j]);
        }
        printf("\n");
    }

    // Initialize validation arrays to TRUE
    for (int i = 0; i < 9; i++)
        colArray[i] = rowArray[i] = subgridArray[i] = TRUE;

    parameters *params;
    int threadIndex = 0;

    // Create threads for column checking
    for (int i = 0; i < 9; i++, threadIndex++) {
        params = (parameters *)malloc(sizeof(parameters));
        params->topRow = 0; params->bottomRow = 8;
        params->leftColumn = i; params->rightColumn = i;
        pthread_create(&tid[threadIndex], NULL, colChecker, params);
    }
    
    // Create threads for row checking
    for (int i = 0; i < 9; i++, threadIndex++) {
        params = (parameters *)malloc(sizeof(parameters));
        params->topRow = i; params->bottomRow = i;
        params->leftColumn = 0; params->rightColumn = 8;
        pthread_create(&tid[threadIndex], NULL, rowChecker, params);
    }

    // Create threads for subgrid checking
    for (int i = 0; i < 9; i++, threadIndex++) {
        params = (parameters *)malloc(sizeof(parameters));
        params->topRow = (i / 3) * 3;
        params->bottomRow = params->topRow + 2;
        params->leftColumn = (i % 3) * 3;
        params->rightColumn = params->leftColumn + 2;
        pthread_create(&tid[threadIndex], NULL, subgridChecker, params);
    }

    // Wait for all threads to finish
    for (int i = 0; i < 27; i++)
        pthread_join(tid[i], NULL);

    // Display results
    printf("\nColumn results:\n");
    for (int i = 0; i < 9; i++)
        printf("Column %d: %s\n", i + 1, colArray[i] ? "valid" : "invalid");

    printf("\nRow results:\n");
    for (int i = 0; i < 9; i++)
        printf("Row %d: %s\n", i + 1, rowArray[i] ? "valid" : "invalid");

    printf("\nSubgrid results:\n");
    for (int i = 0; i < 9; i++)
        printf("Subgrid %d: %s\n", i + 1, subgridArray[i] ? "valid" : "invalid");

    // Determine overall puzzle validity
    bool validArray = TRUE;
    for (int i = 0; i < 9; i++) {
        if (!colArray[i] || !rowArray[i] || !subgridArray[i]) {
            validArray = FALSE;
            break;
        }
    }

    printf("\nSudoku Puzzle: %s\n", validArray ? "valid" : "invalid");
    return validArray ? 0 : 1;
}

// Function to validate a single column
void *colChecker(void *param) {
    parameters *p = (parameters *)param;
    bool seen[9] = {FALSE};
    for (int i = p->topRow; i <= p->bottomRow; i++) {
        int num = sudokuPuzzle[i][p->leftColumn] - 1;
        if (num < 0 || num > 8 || seen[num]) {
            colArray[p->leftColumn] = FALSE;
            printf("TRow: %d, BRow: %d, LCol: %d, RCol: %d invalid!\n",
                p->topRow, p->bottomRow, p->leftColumn, p->rightColumn);
            free(param);
            return NULL;
        }
        seen[num] = TRUE;
    }
    printf("TRow: %d, BRow: %d, LCol: %d, RCol: %d valid!\n",
        p->topRow, p->bottomRow, p->leftColumn, p->rightColumn);
    free(param);
    return NULL;
}

// Function to validate a single row
void *rowChecker(void *param) {
    parameters *p = (parameters *)param;
    bool seen[9] = {FALSE};
    for (int j = p->leftColumn; j <= p->rightColumn; j++) {
        int num = sudokuPuzzle[p->topRow][j] - 1;
        if (num < 0 || num > 8 || seen[num]) {
            rowArray[p->topRow] = FALSE;
            printf("TRow: %d, BRow: %d, LCol: %d, RCol: %d invalid!\n",
                p->topRow, p->bottomRow, p->leftColumn, p->rightColumn);
            free(param);
            return NULL;
        }
        seen[num] = TRUE;
    }
    printf("TRow: %d, BRow: %d, LCol: %d, RCol: %d valid!\n",
        p->topRow, p->bottomRow, p->leftColumn, p->rightColumn);
    free(param);
    return NULL;
}

// Function to validate a 3x3 subgrid
void *subgridChecker(void *param) {
    parameters *p = (parameters *)param;
    bool seen[9] = {FALSE};
    int subgridIndex = (p->topRow / 3) * 3 + (p->leftColumn / 3);
    for (int i = p->topRow; i <= p->bottomRow; i++) {
        for (int j = p->leftColumn; j <= p->rightColumn; j++) {
            int num = sudokuPuzzle[i][j] - 1;
            if (num < 0 || num > 8 || seen[num]) {
                subgridArray[subgridIndex] = FALSE;
                printf("TRow: %d, BRow: %d, LCol: %d, RCol: %d invalid!\n",
                    p->topRow, p->bottomRow, p->leftColumn, p->rightColumn);
                free(param);
                return NULL;
            }
            seen[num] = TRUE;
        }
    }
    printf("TRow: %d, BRow: %d, LCol: %d, RCol: %d valid!\n",
        p->topRow, p->bottomRow, p->leftColumn, p->rightColumn);
    free(param);
    return NULL;
}
