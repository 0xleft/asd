//
// Created by adoma on 10/21/2023.
//

#include <stdio.h>
#include "parson.h"
#include <curl/curl.h>
#include "requests.h"

struct ResponseData {
    char *data;
    size_t size;
};

size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t total_size = size * nmemb;
    struct ResponseData *response = (struct ResponseData *)userp;

    // Reallocate memory for the response and append the new data
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

char* get_package(char* package_name) {
    char* url = "https://registry.npmjs.org/";
    char* full_url = malloc(strlen(url) + strlen(package_name) + 1);
    strcpy(full_url, url);
    strcat(full_url, package_name);
    return make_request(full_url);
}

JSON_Object *get_package_json(char* package_name) {
    char* package_json = get_package(package_name);
    JSON_Value *package_json_value = json_parse_string(package_json);
    JSON_Object *package_json_object = json_value_get_object(package_json_value);
    return package_json_object;
}