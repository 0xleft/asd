#include "caching.h"
#include "utils.h"
#include <sys/stat.h>

#include "stdlib.h"
#include "unistd.h"
#include "string.h"
#include "stdio.h"
#include "globals.h"

#ifdef WIN32
#include <io.h>
#define F_OK 0
#define access _access
#endif

#ifdef _WIN32
char* USER_HOME = "%USERPROFILE%";
char* DIVIDER = "\\";
#else
char* USER_HOME = "$HOME";
char* DIVIDER = "/";
#endif

char* get_resolved_home() {
    if (USER_HOME[0] == '$') {
        char* env = getenv(USER_HOME + 1);
        if (env == NULL) {
            return "";
        }
        return env;
    } else if (USER_HOME[0] == '%') {
        char* env = getenv(USER_HOME + 1);
        if (env == NULL) {
            return "";
        }
        return env;
    }

    return "";
}

char* get_cache_folder(char* package_name, char* version) {
    char* path = malloc(strlen(get_resolved_home()) + strlen(DIVIDER) + strlen(".asdcache") + strlen(DIVIDER) + strlen(package_name) + strlen(DIVIDER) + strlen(version) + 1);
    strcpy(path, get_resolved_home());
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
    // printf("Creating cache folder for package: %s\n", command);
    system(command);
    free(command);
}

int is_cached(char* package_name, char* version) {
    char* path = malloc(strlen(get_resolved_home()) + strlen(DIVIDER) + strlen(".asdcache") + strlen(DIVIDER) + strlen(package_name) + strlen(DIVIDER) + strlen(version) +
                                strlen(DIVIDER) + strlen(package_name) + strlen(".tgz") + 1);
    strcpy(path, get_resolved_home());
    strcat(path, DIVIDER);
    strcat(path, ".asdcache");
    strcat(path, DIVIDER);
    strcat(path, package_name);
    strcat(path, DIVIDER);
    strcat(path, version);
    strcat(path, DIVIDER);
    strcat(path, package_name);
    strcat(path, ".tgz");
    // printf("Checking if package is cached: %s\n", path);

    if (access(path, F_OK) == 0) {
        free(path);
        return 1;
    }

    free(path);
    return 0;
}