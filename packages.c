#include <stdio.h>
#include "parson.h"
#include "packages.h"
#include "utils.h"
#include <stdlib.h>
#include <sys/stat.h>
#include "caching.h"

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

int download_package_tgz(char* download_link, char* package_name, char* version) {
    char* cache_folder = get_cache_folder(package_name, version);

    char* download_command = malloc(strlen("curl -J -L -o /") + strlen(cache_folder) + strlen(package_name) + strlen(".tgz") + strlen(" --silent ") + strlen(download_link) + 5);
    strcpy(download_command, "curl -J -L -o ");
    strcat(download_command, cache_folder);
    strcat(download_command, "/");
    strcat(download_command, package_name);
    strcat(download_command, ".tgz");
    strcat(download_command, " ");
    strcat(download_command, download_link);
    strcat(download_command, " --silent");
    if (system(download_command) != 0) {
        printf("Error downloading package\n");
        return 0;
    }
    printf("Downloaded package to: %s\n", cache_folder);
    printf("Download command: %s\n", download_command);
    free(download_command);

    free(cache_folder);

    return 1;
}

int copy_to_node_folder(char* package_name, char* version) {
    char* cache_folder = get_cache_folder(package_name, version);

    char* copy_command = malloc(strlen("cp -r ") + strlen(cache_folder) + strlen(package_name) + strlen(".tgz node_modules/") + strlen(package_name) + strlen(".tgz") + 5);
    strcpy(copy_command, "cp -r ");
    strcat(copy_command, cache_folder);
    strcat(copy_command, package_name);
    strcat(copy_command, ".tgz node_modules/");
    strcat(copy_command, package_name);
    strcat(copy_command, ".tgz");
    system(copy_command);
    printf("Copied package to: %s\n", copy_command);
    printf("Copy command: %s\n", copy_command);
    free(copy_command);

    free(cache_folder);
}

void install_package(char* package_name, char* version) {
    if (strcmp(version, "*") == 0) {
        version = "latest";
    }

    // some sanitization at least :)
    if (is_valid_input(package_name) == 0 || is_valid_input(version) == 0) {
        printf("Invalid input\n");
        return;
    }

    if (is_cached(package_name, version) == 0) {
        char* package_text = get_package_text(package_name, version);

        JSON_Object *package_object = parse_package_json(package_text);
        char* download_link = get_download_link(package_object);
        free(package_text);

        if (strcmp(download_link, "") == 0) {
            printf("Error getting download link (probably wrong version)\n");
            return;
        }

        create_cache_folder_for_package(package_name, version);

        if (download_package_tgz(download_link, package_name, version) == 0) {
            return;
        }
    }

    create_package_folder(package_name);

    copy_to_node_folder(package_name, version);

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