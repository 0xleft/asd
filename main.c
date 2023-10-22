#include <stdio.h>
#include "packages.h"
#include "stdlib.h"

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("Usage: %s [install, build]\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "install") == 0) {
        printf("Installing...\n");
        create_node_folder();

        JSON_Object *dependencies = get_all_dependencies();
        install_dependencies(dependencies);

        return 0;
    }

    if (strcmp(argv[1], "build") == 0) {
        printf("Building...\n");
        return 0;
    }

    return 0;
}
