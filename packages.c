#include <stdio.h>
#include "parson.h"
#include "packages.h"
#include "utils.h"
#include <stdlib.h>

#ifdef _WIN32
    const char* USER_HOME = "%userprofile%";
    const char* DIVIDER = "\\"
#else
    const char* USER_HOME = "~";
    const char* DIVIDER = "/";
#endif


char* get_package_text(char* package_name, char* version) {
    char* url = "https://registry.npmjs.org/";
    char* full_url = malloc(strlen(url) + strlen(package_name) + 1 + strlen(version) + 1);
    strcpy(full_url, url);
    strcat(full_url, package_name);
    strcat(full_url, "/");
    strcat(full_url, version);

    char* package_text = make_request(full_url);

    free(full_url);

    return package_text;
}

JSON_Object *parse_package_json(char* package_text) {
    JSON_Value *package_json_value = json_parse_string(package_text);
    JSON_Object *package_json_object = json_value_get_object(package_json_value);

    return package_json_object;
}

char* get_download_link(JSON_Object *package_json_object) {
    JSON_Object *dist;
    dist = json_value_get_object(json_object_get_value(package_json_object, "dist"));

    const char* tarball = json_object_get_string(dist, "tarball");

    if (tarball == NULL) {
        return "";
    }

    return tarball;
}

void create_node_folder() {
    char* command = "mkdir -p node_modules";
    system(command);
}

void create_package_folder(char* package_name) {
    char* command = malloc(strlen("mkdir -p node_modules/") + strlen(package_name) + 1);
    strcpy(command, "mkdir -p node_modules/");
    strcat(command, package_name);
    system(command);
    free(command);
}

void create_cache_folder() {
    char* command = malloc(strlen("mkdir -p ") + strlen(USER_HOME) + strlen(DIVIDER) + strlen(".asdcache") + 1);
    strcpy(command, "mkdir -p ");
    strcat(command, USER_HOME);
    strcat(command, DIVIDER);
    strcat(command, ".asdcache");
    system(command);
    free(command);
}

int is_cached(char* package_name, char* version) {

}

int copy_to_node_folder(char* package_name, char* version) {

}

// TODO remove command injection
void install_package(char* package_name, char* version) {
    if (strcmp(version, "*") == 0) {
        version = "latest";
    }

    char* package_text = get_package_text(package_name, version);
    JSON_Object *package_object = parse_package_json(package_text);
    const char* download_link = get_download_link(package_object);
    free(package_text);


    if (is_cached(package_name, version)) {

    }

    create_package_folder(package_name);

    // TODO download to cache folder and then move to node_modules folder
    char* download_command = malloc(strlen("curl -J -L -o node_modules/ --silent ") + strlen(package_name) + strlen(" ") + strlen(download_link) + 5);
    strcpy(download_command, "curl -J -L -o node_modules/");
    strcat(download_command, package_name);
    strcat(download_command, ".tgz ");
    strcat(download_command, download_link);
    strcat(download_command, " --silent");
    system(download_command);
    free(download_command);

    char *extract_command = malloc(strlen("tar -xzf node_modules/") + strlen(package_name) + strlen(".tgz -C node_modules/") + strlen(package_name) + strlen(" --strip-components=1") + 5);
    strcpy(extract_command, "tar -xzf node_modules/");
    strcat(extract_command, package_name);
    strcat(extract_command, ".tgz -C node_modules/");
    strcat(extract_command, package_name);
    strcat(extract_command, " --strip-components=1");
    system(extract_command);
    free(extract_command);

    char *rm_command = malloc(strlen("rm node_modules/") + strlen(package_name) + strlen(".tgz") + 5);
    strcpy(rm_command, "rm node_modules/");
    strcat(rm_command, package_name);
    strcat(rm_command, ".tgz");
    system(rm_command);
    free(rm_command);
}