//
// Created by adoma on 10/21/2023.
//

#include <stdio.h>
#include "parson.h"
#include <curl/curl.h>
#include "packages.h"
#include <pthread.h>

struct ResponseData {
    char *data;
    size_t size;
};

size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t total_size = size * nmemb;
    struct ResponseData *response = (struct ResponseData *)userp;

    // reallocate memory for the response and append the new data
    response->data = (char *)realloc(response->data, response->size + total_size + 1);
    if (response->data) {
        memcpy(response->data + response->size, contents, total_size);
        response->size += total_size;
        response->data[response->size] = '\0';
    }

    return total_size;
}

char* make_request(char* url) {
    CURL* curl;
    CURLcode res;
    struct ResponseData response = { NULL, 0 };

    curl = curl_easy_init();

    if (curl == NULL) {
        printf("Error initializing curl\n");
        return NULL;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    res = curl_easy_perform(curl);

    curl_easy_cleanup(curl);

    return response.data;
}

char* get_package_text(char* package_name) {
    char* url = "https://registry.npmjs.org/";
    char* full_url = malloc(strlen(url) + strlen(package_name) + 1 + strlen("/latest"));
    strcpy(full_url, url);
    strcat(full_url, package_name);
    strcat(full_url, "/latest");

    const char* package_text = make_request(full_url);

    free(full_url);

    return package_text;
}

JSON_Object *get_package_json(char* package_text) {
    JSON_Value *package_json_value = json_parse_string(package_text);
    JSON_Object *package_json_object = json_value_get_object(package_json_value);

    return package_json_object;
}

char* get_latest_download_link(JSON_Object *package_json_object) {
    JSON_Object *dist;
    dist = json_value_get_object(json_object_get_value(package_json_object, "dist"));

    const char* tarball = json_object_get_string(dist, "tarball");

    if (tarball == NULL) {
        printf("Error getting tarball %s\n", tarball);
        return "";
    }

    return tarball;
}

void create_node_folder() {
    char* command = "mkdir -p node_modules";
    system(command);
}

// parse package.json in main directory
JSON_Object *parse_package_json() {
    JSON_Value *package_json_value;
    package_json_value = json_parse_file("package.json");

    if (package_json_value == NULL) {
        printf("Error parsing package.json\n");
        return NULL;
    }

    JSON_Object *package_json_object = json_value_get_object(package_json_value);

    return package_json_object;
}

JSON_Object *get_dev_dependencies() {
    JSON_Object *package_json_object = parse_package_json();
    JSON_Object *dev_dependencies = json_object_dotget_object(package_json_object, "devDependencies");
    return dev_dependencies;
}

JSON_Object *get_dependencies() {
    JSON_Object *package_json_object = parse_package_json();
    JSON_Object *dependencies = json_object_dotget_object(package_json_object, "dependencies");
    return dependencies;
}

// get all dependencies from package.json
JSON_Object *get_all_dependencies() {
    printf("Getting all dependencies...\n");

    JSON_Object *dependencies;
    dependencies = get_dependencies();
    JSON_Object *dev_dependencies;
    dev_dependencies = get_dev_dependencies();

    for (int i = 0; i < json_object_get_count(dev_dependencies); i++) {
        const char* dependency = json_object_get_name(dev_dependencies, i);

        const char* package_text = get_package_text(dependency);
        JSON_Object *package_json_object = get_package_json(package_text);
        const char* latest_download_link = get_latest_download_link(package_json_object);
        json_object_set_string(dependencies, dependency, latest_download_link);
    }

    for (int i = 0; i < json_object_get_count(dependencies); i++) {
        const char* dependency = json_object_get_name(dependencies, i);

        const char* package_text = get_package_text(dependency);
        JSON_Object *package_json_object = get_package_json(package_text);
        const char* latest_download_link = get_latest_download_link(package_json_object);
        json_object_set_string(dependencies, dependency, latest_download_link);
    }

    return dependencies;
}

void create_package_folder(char *package_name) {
    char* command = malloc(strlen("mkdir -p node_modules/") + strlen(package_name) + 1);
    strcpy(command, "mkdir -p node_modules/");
    strcat(command, package_name);
    system(command);
    free(command);
}

// command injection
void install_dependencies(JSON_Object *dependencies) {
    printf("Installing dependencies...\n");

    for (int i = 0; i < json_object_get_count(dependencies); i++) {
        const char* dependency = json_object_get_name(dependencies, i);
        const char* download_link = json_object_get_string(dependencies, dependency);

        if (download_link == "") {
            continue;
        }

        char* command = malloc(strlen("curl -L -o node_modules/") + strlen(dependency) + strlen(" ") + strlen(download_link) + 1);
        strcpy(command, "curl -L -o node_modules/");
        strcat(command, dependency);
        strcat(command, ".tgz ");
        strcat(command, download_link);
        system(command);
        free(command);

        create_package_folder(dependency);

        command = malloc(strlen("tar -xzvf node_modules/") + strlen(dependency) + strlen(".tgz -C node_modules/") + strlen(dependency) + strlen(" --strip-components=1") + 1);
        strcpy(command, "tar -xzvf node_modules/");
        strcat(command, dependency);
        strcat(command, ".tgz -C node_modules/");
        strcat(command, dependency);
        strcat(command, " --strip-components=1");
        system(command);
        free(command);

        command = malloc(strlen("rm node_modules/") + strlen(dependency) + strlen(".tgz") + 1);
        strcpy(command, "rm node_modules/");
        strcat(command, dependency);
        strcat(command, ".tgz");
        system(command);
        free(command);
    }
}