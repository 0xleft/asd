#include "caching.h"
#include "utils.h"
#include "sys/stat.h"

#ifdef _WIN32
char* USER_HOME = "%userprofile%";
char* DIVIDER = "\\";
#else
char* USER_HOME = "~";
char* DIVIDER = "/";
#endif

char* get_cache_folder(char* package_name, char* version) {
    char* path = malloc(strlen(USER_HOME) + strlen(DIVIDER) + strlen(".asdcache") + strlen(DIVIDER) + strlen(package_name) + strlen(DIVIDER) + strlen(version) + 1);
    strcpy(path, USER_HOME);
    strcat(path, DIVIDER);
    strcat(path, ".asdcache");
    strcat(path, DIVIDER);
    strcat(path, package_name);
    strcat(path, DIVIDER);
    strcat(path, version);

    return path;
}

void create_cache_folder_for_package(char* package_name, char* version) {
    char* path = get_cache_folder(package_name, version);
    char* command = malloc(strlen("mkdir -p ") + strlen(path) + 1);
    strcpy(command, "mkdir -p ");
    strcat(command, path);
    printf("Creating cache folder for package: %s\n", command);
    system(command);
    free(command);
}

int is_cached(char* package_name, char* version) {
    char* path = malloc(strlen(USER_HOME) + strlen(DIVIDER) + strlen(".asdcache") + strlen(DIVIDER) + strlen(package_name) + strlen(DIVIDER) + strlen(version) +
                                strlen(DIVIDER) + strlen(package_name) + strlen(".tgz") + 1);
    strcpy(path, USER_HOME);
    strcat(path, DIVIDER);
    strcat(path, ".asdcache");
    strcat(path, DIVIDER);
    strcat(path, package_name);
    strcat(path, DIVIDER);
    strcat(path, version);
    strcat(path, DIVIDER);
    strcat(path, package_name);
    strcat(path, ".tgz");
    printf("Checking if package is cached: %s\n", path);

    struct stat st;

    if (stat(path, &st) == 0) {
        return 1;
    }

    free(path);

    return 0;
}