//
// Created by plusleft on 10/21/2023.
//

#include "parson.h"

char* make_request(char* url);
char* get_package_text(char* package_name);
JSON_Object *get_package_json(char* package_name);
char* get_latest_download_link(JSON_Object *package_json_object);
void create_node_folder();
JSON_Object *get_all_dependencies(char* path);
JSON_Object *get_dependencies(char* path);
JSON_Object *get_dev_dependencies(char* path);
void install_dependencies(JSON_Object *dependencies);