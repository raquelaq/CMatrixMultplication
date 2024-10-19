#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <psapi.h>
#include <time.h>

void generateMatrix(int** matrix, int rows, int cols, int min, int max) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            matrix[i][j] = rand() % (max - min + 1) + min;
        }
    }
}

void matrixMultiplication(int** A, int** B, int** C, int rowsA, int colsA, int colsB) {
    for (int i = 0; i < rowsA; i++) {
        for (int j = 0; j < colsB; j++) {
            C[i][j] = 0;
            for (int k = 0; k < colsA; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

double calculateCPUUsage(FILETIME* start, FILETIME* end) {
    ULARGE_INTEGER s, e;
    s.LowPart = start->dwLowDateTime;
    s.HighPart = start->dwHighDateTime;
    e.LowPart = end->dwLowDateTime;
    e.HighPart = end->dwHighDateTime;

    return (double)(e.QuadPart - s.QuadPart) / 10000.0;  
}

int main() {
    srand(time(NULL));

    int sizes[] = {10, 50, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1024};
    int numSizes = sizeof(sizes) / sizeof(sizes[0]);

    FILE *csvFile = fopen("../benchmark_results.csv", "a");
    if (csvFile == NULL) {
        fprintf(stderr, "Error opening file for writing.\n");
        return 1;
    }

    for (int idx = 0; idx < numSizes; idx++) {
        int size = sizes[idx];

        
        int** A = (int**)malloc(size * sizeof(int*));
        int** B = (int**)malloc(size * sizeof(int*));
        int** C = (int**)malloc(size * sizeof(int*));
        for (int i = 0; i < size; i++) {
            A[i] = (int*)malloc(size * sizeof(int));
            B[i] = (int*)malloc(size * sizeof(int));
            C[i] = (int*)malloc(size * sizeof(int));
        }

        
        generateMatrix(A, size, size, 1, 9);
        generateMatrix(B, size, size, 1, 9);

        LARGE_INTEGER frequency, start, end;
        QueryPerformanceFrequency(&frequency); 

        PROCESS_MEMORY_COUNTERS memCounter;
        GetProcessMemoryInfo(GetCurrentProcess(), &memCounter, sizeof(memCounter));
        SIZE_T memoryBefore = memCounter.WorkingSetSize;

        FILETIME creationTime, exitTime, kernelStartTime, userStartTime;
        FILETIME kernelEndTime, userEndTime;

        HANDLE processHandle = GetCurrentProcess();
        GetProcessTimes(processHandle, &creationTime, &exitTime, &kernelStartTime, &userStartTime);

        QueryPerformanceCounter(&start);  
        matrixMultiplication(A, B, C, size, size, size);
        QueryPerformanceCounter(&end);    

        GetProcessMemoryInfo(GetCurrentProcess(), &memCounter, sizeof(memCounter));
        SIZE_T memoryAfter = memCounter.WorkingSetSize;
        GetProcessTimes(processHandle, &creationTime, &exitTime, &kernelEndTime, &userEndTime);

        double kernelTimeUsed = calculateCPUUsage(&kernelStartTime, &kernelEndTime);
        double userTimeUsed = calculateCPUUsage(&userStartTime, &userEndTime);

        double executionTime = (double)(end.QuadPart - start.QuadPart) * 1000.0 / frequency.QuadPart;

        double totalCPUTime = kernelTimeUsed + userTimeUsed;
        double cpuUsage = 0;
        if (executionTime > 0) {
            cpuUsage = totalCPUTime / executionTime * 100;
        }

        printf("\n\nMatrix size: %dx%d\n", size, size);
        printf("Execution time: %.3f milliseconds\n", executionTime);
        printf("Memory used: %zu bytes\n", memoryAfter - memoryBefore);
        printf("CPU Usage: %.3f%%\n", cpuUsage);
        printf("----------------------------------");

        fprintf(csvFile, "C,%dx%d,%.3f,%.2f,%.3f\n", size, size, executionTime, (double)(memoryAfter - memoryBefore) / (1024 * 1024), cpuUsage);

        for (int i = 0; i < size; i++) {
            free(A[i]);
            free(B[i]);
            free(C[i]);
        }
        free(A);
        free(B);
        free(C);
    }

    fclose(csvFile);

    return 0;
}
