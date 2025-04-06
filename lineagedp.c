#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <libgen.h>

#define MAX_FILENAME_LENGTH 256
enum filetypes { FT_REG, FT_DIR, FT_CHR, FT_BLK, FT_FIFO, FT_LINK, FT_SOCK, FT_ERR };
char *filetypenames[] =  {"regular", "directory", "character special",  "block special", "fifo", "symbolic link", "socket", "** unknown mode **" };

pthread_mutex_t lock;
int globalFileCounts[8] = {0};
int totalBytes = 0;

bool verbose = false; // Add a variable for verbose flag

void printTotal()
{
    printf("\ttotal:\n");
    printf("\t\tbytes used:%d\n", totalBytes);
    printf("\t\ttypecount:\n");
    for (int i = 0; i < 8; i++)  
    {
        if (globalFileCounts[i] > 0)
        {
            printf("\t\t\t%s %d\n", filetypenames[i], globalFileCounts[i]);
        }
    }
}

void report(int bytes, int typecount[], char *name)
{
    printf("\t%s:\n", name);
    printf("\t\tbytes used:%d\n", bytes);
    printf("\t\ttypecount:\n");
    for (int i = 0; i < 8; i++)  
    {
        if(typecount[i] > 0)
        {
            printf("\t\t\t%s %d\n", filetypenames[i], typecount[i]);
        }
    }
    if(verbose)
    {

    }
}

bool isVowel(char ch)
{
    return (ch == 'A' || ch == 'a' || ch == 'E' || ch == 'e' || ch == 'I' || ch == 'i' || ch == 'O' || ch == 'o' || ch == 'U' || ch == 'u');
}

void processFileType(struct stat *file_stat, int *localFileCounts)
{
    pthread_mutex_lock(&lock);

    if (S_ISREG(file_stat->st_mode)) 
    {
        localFileCounts[FT_REG]++;
        globalFileCounts[FT_REG]++;
    }
    else if (S_ISDIR(file_stat->st_mode)) 
    {
        localFileCounts[FT_DIR]++;
        globalFileCounts[FT_DIR]++;
    }
    else if (S_ISLNK(file_stat->st_mode)) 
    {
        localFileCounts[FT_LINK]++;
        globalFileCounts[FT_LINK]++;
    }
    else if (S_ISBLK(file_stat->st_mode)) 
    {
        localFileCounts[FT_BLK]++;
        globalFileCounts[FT_BLK]++;
    }
    else if (S_ISCHR(file_stat->st_mode)) 
    {
        localFileCounts[FT_CHR]++;
        globalFileCounts[FT_CHR]++;
    }
    else if (S_ISFIFO(file_stat->st_mode)) 
    {
        localFileCounts[FT_FIFO]++;
        globalFileCounts[FT_FIFO]++;
    }
    else if (S_ISSOCK(file_stat->st_mode)) 
    {
        localFileCounts[FT_SOCK]++;
        globalFileCounts[FT_SOCK]++;
    }
    else
    {
        localFileCounts[FT_ERR]++;
        globalFileCounts[FT_ERR]++;
    }

    pthread_mutex_unlock(&lock);
}

void printVerboseInfo(char *filename, int byteSize, int *localFileCounts, char *threadType)
{
    pid_t pid = getpid();  // Get the process ID
    pthread_t tid = pthread_self();  // Get the thread ID

    // Print the verbose information
    printf("%s PID:%d tid:%lu (0x%lx) bytesthisfile:%d bytessofar:%d\n", 
           threadType, pid, tid, (unsigned long)tid, byteSize, totalBytes);
}

void *vowels(void *arg)
{
    struct stat file_stat;
    char *filepath = (char *)arg;
    if (stat(filepath, &file_stat) == -1)
    {
        perror("stat failed");
        pthread_exit(NULL);
    }

    char *filename = basename(filepath);
    int vowelCount = 0;
    for (int i = 0; i < strlen(filename); i++)
    {
        if (isVowel(filename[i]))
            vowelCount++;
    }

    int *byteSize = malloc(sizeof(int));
    *byteSize = vowelCount * sizeof(char);

    int localFileCounts[8] = {0};
    processFileType(&file_stat, localFileCounts);

    pthread_mutex_lock(&lock);
    totalBytes += *byteSize;
    pthread_mutex_unlock(&lock);

    if (verbose)  // Check if verbose flag is set
    {
        printVerboseInfo(filename, *byteSize, localFileCounts, "vowel thread");
    }

    int *result = malloc(sizeof(int) * 9);  // 8 file types + byte size
    result[0] = *byteSize;
    memcpy(&result[1], localFileCounts, sizeof(int) * 8);

    pthread_exit((void *)result);
}

void *digits(void *arg)
{
    struct stat file_stat;
    char *filepath = (char *)arg;
    if (stat(filepath, &file_stat) == -1)
    {
        perror("stat failed");
        pthread_exit(NULL);
    }

    char *filename = basename(filepath);
    int digitCount = 0;
    for (int i = 0; i < strlen(filename); i++)
    {
        if (isdigit(filename[i]))
            digitCount++;
    }

    int *byteSize = malloc(sizeof(int));
    *byteSize = digitCount * sizeof(int);

    int localFileCounts[8] = {0};
    processFileType(&file_stat, localFileCounts);

    pthread_mutex_lock(&lock);
    totalBytes += *byteSize;
    pthread_mutex_unlock(&lock);

    if (verbose)  // Check if verbose flag is set
    {
        printVerboseInfo(filename, *byteSize, localFileCounts, "digit thread");
    }

    int *result = malloc(sizeof(int) * 9);
    result[0] = *byteSize;
    memcpy(&result[1], localFileCounts, sizeof(int) * 8);

    pthread_exit((void *)result);
}

void *others(void *arg)
{
    struct stat file_stat;
    char *filepath = (char *)arg;
    if (stat(filepath, &file_stat) == -1)
    {
        perror("stat failed");
        pthread_exit(NULL);
    }

    char *filename = basename(filepath);
    int otherCount = 0;
    for (int i = 0; i < strlen(filename); i++)
    {
        if (!isdigit(filename[i]) && !isVowel(filename[i]))
            otherCount++;
    }

    int *byteSize = malloc(sizeof(int));
    *byteSize = otherCount * sizeof(char);

    int localFileCounts[8] = {0};
    processFileType(&file_stat, localFileCounts);

    pthread_mutex_lock(&lock);
    totalBytes += *byteSize;
    pthread_mutex_unlock(&lock);

    if (verbose)  // Check if verbose flag is set
    {
        printVerboseInfo(filename, *byteSize, localFileCounts, "other thread");
    }

    int *result = malloc(sizeof(int) * 9);
    result[0] = *byteSize;
    memcpy(&result[1], localFileCounts, sizeof(int) * 8);

    pthread_exit((void *)result);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s [-v] <filepaths>\n", argv[0]);
        return 1;
    }

    int startIndex = 1;
    if (strcmp(argv[1], "-v") == 0)
    {
        verbose = true;
        startIndex = 2;
    }

    pthread_mutex_init(&lock, NULL);

    printf("%s\n", argv[startIndex]);

    for (int i = startIndex; i < argc; i++)
    {
        pid_t pid = fork();

        if (pid == 0)  // Child process
        {
            pthread_t vowelThread, digitThread, otherThread;
            int *vowelResult, *digitResult, *otherResult;

            pthread_create(&vowelThread, NULL, vowels, (void *)argv[i]);
            pthread_create(&digitThread, NULL, digits, (void *)argv[i]);
            pthread_create(&otherThread, NULL, others, (void *)argv[i]);

            pthread_join(vowelThread, (void **)&vowelResult);
            pthread_join(digitThread, (void **)&digitResult);
            pthread_join(otherThread, (void **)&otherResult);

            // Print results in order with the correct byte values
            report(vowelResult[0], &vowelResult[1], "vowels");
            report(digitResult[0], &digitResult[1], "digits");
            report(otherResult[0], &otherResult[1], "others");
            printTotal();
            printf("%s\n", argv[i+1]);


            free(vowelResult);
            free(digitResult);
            free(otherResult);

            exit(0);
        }
        else if (pid < 0)
        {
            perror("Fork failed");
            exit(1);
        }
    }

    for (int i = startIndex; i < argc; i++)
    {
        wait(NULL);
    }

    pthread_mutex_destroy(&lock);
    return 0;
}
