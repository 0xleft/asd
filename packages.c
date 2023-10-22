//
// Created by adoma on 10/21/2023.
//

#include <stdio.h>
#include "parson.h"
#include <curl/curl.h>
#include "packages.h"

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
    char* full_url = malloc(strlen(url) + strlen(package_name) + 1);
    strcpy(full_url, url);
    strcat(full_url, package_name);
    return make_request(full_url);
}

JSON_Object *get_package_json(char* package_text) {
    JSON_Value *package_json_value = json_parse_string(package_text);
    JSON_Object *package_json_object = json_value_get_object(package_json_value);

    return package_json_object;
}

char* get_latest_version(JSON_Object *package_json_object) {
    JSON_Object *dist_tags = json_object_dotget_object(package_json_object, "dist-tags");
    const char* latest = json_object_dotget_string(dist_tags, "latest");

    return latest;
}

char* get_latest_download_link(JSON_Object *package_json_object) {
    char* latest = get_latest_version(package_json_object);

    if (latest == NULL) {
        printf("Error getting latest version\n");
        return "";
    }

    JSON_Object *versions;
    versions = json_object_dotget_object(package_json_object, "versions");

    JSON_Object *latest_version;
    latest_version = json_object_get_object(versions, latest);

    JSON_Object *dist;
    dist = json_value_get_object(json_object_get_value(latest_version, "dist"));

    const char* tarball = json_object_get_string(dist, "tarball");

    free(latest);

    if (tarball == NULL) {
        printf("Error getting tarball\n");
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

void install_dependencies(JSON_Object *dependencies) {
    for (int i = 0; i < json_object_get_count(dependencies); i++) {
        const char* dependency = json_object_get_name(dependencies, i);
        const char* download_link = json_object_get_string(dependencies, dependency);

        
    }
}