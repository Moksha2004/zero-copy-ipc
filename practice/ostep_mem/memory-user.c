#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    printf("PID: %d\n", getpid());
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <memory_in_mb> [duration_in_seconds]\n", argv[0]);
        return 1;
    }

    int memory_mb = atoi(argv[1]);
    if (memory_mb <= 0) {
        fprintf(stderr, "Error: Memory must be a positive integer.\n");
        return 1;
    }

    long long memory_bytes = (long long)memory_mb * 1024 * 1024;
    long long length = memory_bytes / sizeof(int);

    int *arr = (int *)malloc(memory_bytes);
    if (arr == NULL) {
        perror("Malloc failed");
        return 1;
    }

    printf("Allocated %d MB. Beginning memory stream...\n", memory_mb);

    int duration_sec = 0;
    if (argc == 3) {
        duration_sec = atoi(argv[2]);
        printf("Running for %d seconds.\n", duration_sec);
    } else {
        printf("Running indefinitely (Ctrl+C to stop).\n");
    }

    time_t start_time = time(NULL);
    long long ints_per_gb = (1024 * 1024 * 1024) / sizeof(int);
    while (1) {
        if (duration_sec > 0 && (time(NULL) - start_time) >= duration_sec) {
            break;
        }
        for (long long i = 0; i < length; i++) {
            arr[i] += 1; 
            if (i % ints_per_gb == 0 && i > 0) {
                 printf("Touched %lld GB\n", (i * sizeof(int)) / (1024*1024*1024));
            }
        }
    }

    printf("Finished.\n");
    free(arr);
    return 0;
}