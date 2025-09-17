#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>

typedef struct {
    double minTime;
    double maxTime;
    double totalTime;
    int runs;
    int fails;
    struct timespec startTime;
} BenchStats;

long handleArguments(const char* optarg, const char* errorMessage, int max) {
    char* endPtr;
    long result = strtol(optarg, &endPtr, 10);
    if (result < 0 || result > max) {
        fprintf(stderr, "%s\n", errorMessage);
        exit(EXIT_FAILURE);
    }

    if (*endPtr != '\0') {
        fprintf(stderr, "Invalid Count\n");
        exit(EXIT_FAILURE);
    }

    return result;
}

void warmupRuns(int warmup, char* argv[], int optind) {
    for (int i = 0; i < warmup; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            execvp(argv[optind], &argv[optind]);
            perror("execvp");
            _exit(EXIT_FAILURE);
        }
        int status;
        if (waitpid(pid, &status, 0) < 0) {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }
    }
}

void setup(BenchStats* stats) {
    stats->minTime = 1e9;
    stats->maxTime = 0;
    stats->totalTime = 0;
    stats->runs = 0;
    stats->fails = 0;
    clock_gettime(CLOCK_MONOTONIC, &stats->startTime);
}

void execute(BenchStats* stats, long duration, char* argv[], int optind) {
    while (1) { 
        struct timespec timeNow;
        clock_gettime(CLOCK_MONOTONIC, &timeNow);
        double elapsed = (timeNow.tv_sec - stats->startTime.tv_sec) + (timeNow.tv_nsec - stats->startTime.tv_nsec) / 1e9; // <= converting nanoseconds to seconds
        if (elapsed >= duration) break;

        // Measure run
        struct timespec runStart, runEnd;
        clock_gettime(CLOCK_MONOTONIC, &runStart);

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {
            execvp(argv[optind], &argv[optind]);
            perror("execvp");
            _exit(EXIT_FAILURE);
        }

        int status;
        if (waitpid(pid, &status, 0) < 0) {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }

        clock_gettime(CLOCK_MONOTONIC, &runEnd);
        double runTime = (runEnd.tv_sec - runStart.tv_sec) + (runEnd.tv_nsec - runStart.tv_nsec) / 1e9;

        if (WIFEXITED(status)) {
            int code = WEXITSTATUS(status);
            if (code != 0) {
                stats->fails++;
                fprintf(stderr, "Child exited with status: %d\n", code);
            } else {
                if (runTime < stats->minTime) stats->minTime = runTime;
                if (runTime > stats->maxTime) stats->maxTime = runTime;

                stats->totalTime += runTime;
                stats->runs++;
            }
        } else if (WIFSIGNALED(status)) {
            fprintf(stderr, "Child killed by signal: %d - [%s]\n",
                    WTERMSIG(status), strsignal(WTERMSIG(status)));
            stats->fails++;
        } else {
            fprintf(stderr, "Oops, child killed itself unexpectedly\n");
            stats->fails++;
        }
    } 
}

void makeReport(BenchStats* stats, long warmup) {
    printf("Min:    %.6f seconds   Warmups: %ld\n", stats->minTime, warmup);
    printf("Avg:    %.6f seconds   Runs: %d\n", stats->totalTime / stats->runs, stats->runs);
    printf("Max:    %.6f seconds   Fails: %d\n", stats->maxTime, stats->fails);
    printf("Total:  %.6f seconds\n", stats->totalTime);
}

int main(int argc, char* argv[]) {
    int option;

    long warmup = 0, duration = 0;
    while ((option = getopt(argc, argv, "w:d:")) != -1) {
        switch(option) {
            case 'w': {
                if (optarg == NULL) {
                    fprintf(stderr, "-w requires an argument\n");
                    exit(EXIT_FAILURE);
                }

                const char* errorMessage = "The number of warmup runs out of range";
                warmup = handleArguments(optarg, errorMessage, 10); 

                break;
            }
            case 'd': {
                if (optarg == NULL) {
                    fprintf(stderr, "-d requires an argument\n");
                    exit(EXIT_FAILURE);
                }

                const char* errorMessage = "Duration time out of range";
                duration = handleArguments(optarg, errorMessage, 1000);

                break;
            }
                
            default:
                fprintf(stderr, "Unknown option\n");;
                exit(EXIT_FAILURE);
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "No command specified to benchmark\n");
        exit(EXIT_FAILURE);
    }

    // first run warmups
    warmupRuns(warmup, argv, optind);

    // setup variables and stats
    BenchStats stats;
    setup(&stats);
    execute(&stats, duration, argv, optind); 

    makeReport(&stats, warmup);
    return 0;
}
