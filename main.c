#include <stdio.h>
#include "packages.h"
#include "stdlib.h"

int main(int argc, char *argv[]) {

    if (argc < 4) {
        printf("Usage: %s [install] [package] [version]\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "install") == 0) {
        printf("Installing...\n");
        create_node_folder();
        create_cache_folder();

        install_package(argv[2], argv[3]);

        printf("Done!\n");

        return 0;
    }

    return 0;
}
