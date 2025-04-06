//Derrick Subnaik
//OS Assignment2
//lineage.c
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <libgen.h>

// Function to print permissions in ls -l format
void printPermissions(mode_t mode, char *permStr) 
{
    sprintf(permStr, "%c%c%c%c%c%c%c%c%c%c",
        (S_ISDIR(mode)) ? 'd' : '-',
        (mode & S_IRUSR) ? 'r' : '-',
        (mode & S_IWUSR) ? 'w' : '-',
        (mode & S_IXUSR) ? 'x' : '-',
        (mode & S_IRGRP) ? 'r' : '-',
        (mode & S_IWGRP) ? 'w' : '-',
        (mode & S_IXGRP) ? 'x' : '-',
        (mode & S_IROTH) ? 'r' : '-',
        (mode & S_IWOTH) ? 'w' : '-',
        (mode & S_IXOTH) ? 'x' : '-');
}


// Function to get file type as a string
char *fileType(mode_t mode) 
{
    if (S_ISREG(mode)) 
    {
        return "Regular";
    }
    else if (S_ISDIR(mode)) 
    {
        return "Directory";
    }
    else if (S_ISLNK(mode)) 
    {
        return "Symlink";
    }
    else if (S_ISBLK(mode)) 
    {
        return "Block";
    }
    else if (S_ISCHR(mode)) 
    {
        return "Character";
    }
    else if (S_ISFIFO(mode)) 
    {
        return "FIFO";
    }
    else if (S_ISSOCK(mode)) 
    {
        return "Socket";
    }
    else
    {
        return "Other";
    }
}

// Function to get owner and group names
void ownerGroup(struct stat *sb, char *owner, char *group) 
{
    struct passwd *pw = getpwuid(sb->st_uid);
    struct group *gr = getgrgid(sb->st_gid);
    strcpy(owner, pw ? pw->pw_name : "Unknown");
    strcpy(group, gr ? gr->gr_name : "Unknown");
}

// Function to format time for display
void formatTime(time_t rawtime, char *buffer) 
{
    struct tm *tm_info = localtime(&rawtime);
    strftime(buffer, 20, "%b %d %H:%M", tm_info);  
}

// Function to determine the max filename width
int getMaxFilenameWidth(char *path) {
    int maxWidth = strlen(path);
    char tempPath[512];
    strcpy(tempPath, path);
    
    while (strcmp(tempPath, "/") != 0) {
        char *parent = dirname(tempPath);
        int len = strlen(parent);
        if (len > maxWidth) maxWidth = len;
        strcpy(tempPath, parent);
    }

    return maxWidth;
}

// Function to display file info
void printFileInfo(char *path, int maxFilenameWidth) {
    struct stat path_stat;

    if (stat(path, &path_stat) != 0) {
        perror("stat failed");
        return;
    }

    char permissions[11], owner[20], group[20], atime[20], ctime[20], mtime[20];
    printPermissions(path_stat.st_mode, permissions);
    ownerGroup(&path_stat, owner, group);
    formatTime(path_stat.st_atime, atime);
    formatTime(path_stat.st_ctime, ctime);
    formatTime(path_stat.st_mtime, mtime);

    // Add trailing slash for directories
    char displayPath[520];
    if (S_ISDIR(path_stat.st_mode)) {
        snprintf(displayPath, sizeof(displayPath), "%s/", path);
    } else {
        snprintf(displayPath, sizeof(displayPath), "%s", path);
    }

    printf("%-*s %-10s %-10s %-10s %-10s %-15s %-15s %-15s\n",
        maxFilenameWidth, displayPath, 
        fileType(path_stat.st_mode), 
        permissions, 
        owner, 
        group, 
        atime, 
        ctime, 
        mtime);
}


int main(int argc, char *argv[]) 
{

    if (argc < 2) {
        printf("Usage: %s <file_or_directory>\n", argv[0]);
        return 1;
    }

    int maxFilenameWidth = 4;  // Minimum width for "Name" column

    // Find the max filename width considering all arguments and their parent directories
    for (int i = 1; i < argc; i++) 
    {
        char resolvedPath[512];
        if (realpath(argv[i], resolvedPath) == NULL) {
            perror("realpath failed");
            continue;
        }
        int width = getMaxFilenameWidth(resolvedPath);
        if (width > maxFilenameWidth) {
            maxFilenameWidth = width;
        }
    }

    // Print the header
    printf("%-*s %-10s %-10s %-10s %-10s %-15s %-15s %-15s\n",
        maxFilenameWidth, "Name", "FileType", "Permissions", "Owner", "Group", "Access", "Change", "Modification");

    // Process each argument and its parent directories
    for (int i = 1; i < argc; i++) 
    {
        char resolvedPath[512];
        if (realpath(argv[i], resolvedPath) == NULL)
        {
            perror("realpath failed");
            continue;
        }

        // Print the file itself
        printFileInfo(resolvedPath, maxFilenameWidth);

        // Print each parent directory
        char tempPath[512];
        strcpy(tempPath, resolvedPath);
        while (1)
        {
          char *parent = dirname(tempPath);
          if (strcmp(parent, "/") == 0) break; // Stop before printing "/"
          printFileInfo(parent, maxFilenameWidth);
          strcpy(tempPath, parent);
        }

    }

    return 0;
}
