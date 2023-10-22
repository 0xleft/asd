//
// Created by plusleft on 10/21/2023.
//

#include "parson.h"

char* make_request(char* url);
char* get_package_text(char* package_name);
JSON_Object *get_package_json(char* package_name);
char* get_latest_version(JSON_Object *package_json_object);
char* get_latest_download_link(JSON_Object *package_json_object);
void create_node_folder();
JSON_Object *parse_package_json();
JSON_Object *get_all_dependencies();
JSON_Object *get_dependencies();
JSON_Object *get_dev_dependencies();
void install_dependencies(JSON_Object *dependencies);