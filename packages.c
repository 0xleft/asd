#include <stdio.h>
#include "thirdparty/parson.h"
#include "packages.h"
#include "utils.h"
#include <stdlib.h>
#include <sys/stat.h>
#include "caching.h"
#include "globals.h"

#define MAX_THREAD_COUNT 10

int thread_count = 0;

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

/// Parse package text to JSON object

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
    char* command = malloc(strlen("mkdir -p node_modules") + strlen(package_name) + 2);
    strcpy(command, "mkdir -p node_modules");
    strcat(command, DIVIDER);
    strcat(command, package_name);
    system(command);
    free(command);
}

int download_package_tgz(char* download_link, char* package_name, char* version) {
    char* cache_folder = get_cache_folder(package_name, version);

    char* download_command = malloc(strlen("curl -J -L -o /") + strlen(cache_folder) + strlen(package_name) + strlen(".tgz") + strlen(" --silent ") + strlen(download_link) + 5);
    strcpy(download_command, "curl -J -L -o ");
    strcat(download_command, cache_folder);
    strcat(download_command, DIVIDER);
    strcat(download_command, package_name);
    strcat(download_command, ".tgz");
    strcat(download_command, " ");
    strcat(download_command, download_link);
    strcat(download_command, " --silent");
    if (system(download_command) != 0) {
        printf("Error downloading package\n");
        return 0;
    }
    // printf("Downloaded package to: %s\n", cache_folder);
    // printf("Download command: %s\n", download_command);
    free(download_command);

    free(cache_folder);

    return 1;
}

int copy_to_node_folder(char* package_name, char* version) {
    char* cache_folder = get_cache_folder(package_name, version);

    char* copy_command = malloc(strlen("cp -r ") + strlen(cache_folder) + strlen(package_name) + strlen(".tgz node_modules ") + strlen(package_name) + strlen(".tgz") + 5);
    strcpy(copy_command, "cp -r ");
    strcat(copy_command, cache_folder);
    strcat(copy_command, DIVIDER);
    strcat(copy_command, package_name);
    strcat(copy_command, ".tgz node_modules");
    strcat(copy_command, DIVIDER);
    strcat(copy_command, package_name);
    strcat(copy_command, ".tgz");
    system(copy_command);
    // printf("Copied package to: %s\n", copy_command);
    // printf("Copy command: %s\n", copy_command);
    free(copy_command);

    free(cache_folder);
}

void add_dep_to_array(JSON_Array *all_array, JSON_Object *dependencies) {
    for (int i = 0; i < json_object_get_count(dependencies); i++) {
        const char *key = json_object_get_name(dependencies, i);
        const char *value = json_object_get_string(dependencies, key);

        JSON_Value *dep_value = json_value_init_object();
        JSON_Object *dep_object = json_value_get_object(dep_value);

        json_object_set_string(dep_object, "name", key);
        json_object_set_string(dep_object, "version", value);

        json_array_append_value(all_array, dep_value);
    }
}

char* get_package_json_path(char* package_name) {
    if (strcmp(package_name, ".") == 0) {
        return "package.json";
    }

    char* path = malloc(strlen("node_modules/") + strlen(package_name) + strlen("/package.json") + 2);
    strcpy(path, "node_modules");
    strcat(path, DIVIDER);
    strcat(path, package_name);
    strcat(path, DIVIDER);
    strcat(path, "package.json");

    return path;
}

// chatgpt
char* read_package_json(char* package_name) {
    char* path = get_package_json_path(package_name);

    FILE *fp;
    long lSize;
    char *buffer;

    fp = fopen ( path , "rb" );
    if( !fp ) perror(path),exit(1);

    fseek( fp , 0L , SEEK_END);
    lSize = ftell( fp );
    rewind( fp );

    /* allocate memory for entire content */
    buffer = calloc( 1, lSize+1 );
    if( !buffer ) fclose(fp),fputs("memory alloc fails",stderr),exit(1);

    /* copy the file into the buffer */
    if( 1!=fread( buffer , lSize, 1 , fp) )
        fclose(fp),free(buffer),fputs("entire read fails",stderr),exit(1);

    fclose(fp);
    free(path);

    return buffer;
}

/// For debugging purposes

void print_deps_from_array(JSON_Array *all_array) {
    for (int i = 0; i < json_array_get_count(all_array); i++) {
        JSON_Value *dep_value = json_array_get_value(all_array, i);
        JSON_Object *dep_object = json_value_get_object(dep_value);

        const char *name = json_object_get_string(dep_object, "name");
        const char *version = json_object_get_string(dep_object, "version");

        printf("%s : %s\n", name, version);
    }
}

JSON_Array *get_deps_from_json(char* package_text) {
    JSON_Object *package_json_object = parse_package_json(package_text);

    JSON_Object *dependencies;
    dependencies = json_value_get_object(json_object_get_value(package_json_object, "dependencies"));

    JSON_Object *dev_dependencies;
    dev_dependencies = json_value_get_object(json_object_get_value(package_json_object, "devDependencies"));

    JSON_Value *all_dependencies_value = json_value_init_array();
    JSON_Array *all_dependencies = json_value_get_array(all_dependencies_value);
    json_array_clear(all_dependencies);

    add_dep_to_array(all_dependencies, dependencies);
    add_dep_to_array(all_dependencies, dev_dependencies);

    return all_dependencies;
}

/// Get already installed dependencies
JSON_Array *get_installed_deps() {
    JSON_Value *all_dependencies_value = json_value_init_array();
    JSON_Array *all_dependencies = json_value_get_array(all_dependencies_value);
    json_array_clear(all_dependencies);

    char* command = "ls node_modules";
    FILE *fp;
    char path[1035];

    fp = popen(command, "r");
    if (fp == NULL) {
        printf("Failed to run command\n" );
        exit(1);
    }
    // print fpp
    while (fgets(path, sizeof(path)-1, fp) != NULL) {
        // rmeove \n
        path[strlen(path) - 1] = '\0';
        char* package_text = read_package_json(path);
        JSON_Array *package_json = parse_package_json(package_text);
        char* version = json_object_dotget_string(package_json, "version");

        JSON_Value *dep_value = json_value_init_object();
        JSON_Object *dep_object = json_value_get_object(dep_value);

        json_object_set_string(dep_object, "name", path);
        json_object_set_string(dep_object, "version", version);

        json_array_append_value(all_dependencies, dep_value);

        free(package_text);
    }

    pclose(fp);
    return all_dependencies;
}

char* resolve_version(char* version) {
    if (strcmp(version, "*") == 0) {
        return "latest";
    }

    // TODO actually resolve version

    return "latest";
    //return version;
}

void install_package(char* package_name, char* version) {
    version = resolve_version(version);

    // some sanitization at least :)
    if (is_valid_input(package_name) == 0 || is_valid_input(version) == 0) {
        printf("Invalid input for %s : %s\n", package_name, version);
        return;
    }

    if (is_cached(package_name, version) == 0) {
        char* package_text = get_package_text(package_name, version);

        JSON_Object *package_object = parse_package_json(package_text);
        char* download_link = get_download_link(package_object);
        free(package_text);

        if (strcmp(download_link, "") == 0) {
            printf("Error getting download link (probably wrong version) %s\n", version);
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
    strcpy(extract_command, "tar -xzf node_modules");
    strcat(extract_command, DIVIDER);
    strcat(extract_command, package_name);
    strcat(extract_command, ".tgz -C node_modules");
    strcat(extract_command, DIVIDER);
    strcat(extract_command, package_name);
    strcat(extract_command, " --strip-components=1");
    system(extract_command);
    free(extract_command);

    char *rm_command = malloc(strlen("rm node_modules/") + strlen(package_name) + strlen(".tgz") + 5);
    strcpy(rm_command, "rm node_modules");
    strcat(rm_command, DIVIDER);
    strcat(rm_command, package_name);
    strcat(rm_command, ".tgz");
    system(rm_command);
    free(rm_command);

    JSON_Array *all_dependencies = get_deps_from_json(read_package_json(package_name));

    for (int i = 0; i < json_array_get_count(all_dependencies); i++) {
        JSON_Value *dep_value = json_array_get_value(all_dependencies, i);
        JSON_Object *dep_object = json_value_get_object(dep_value);

        const char *name = json_object_get_string(dep_object, "name");
        const char *version = json_object_get_string(dep_object, "version");

        printf("Installing dependency: %s : %s\n", name, version);

        install_package(name, version);
    }
    // print_deps_from_array(all_dependencies);
}