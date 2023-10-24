//
// Created by adomas on 10/24/2023.
//

#include "utils.h"
#include <curl/curl.h>

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
    } else {
        printf("Error allocating memory for response\n");
        return 0;
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