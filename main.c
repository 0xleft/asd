#include <stdio.h>
#include "packages.h"
#include "stdlib.h"
#include <time.h>

int main(int argc, char *argv[]) {

    clock_t start, end;
    double cpu_time_used;

    start = clock();

    if (argc < 4) {
        printf("Usage: %s [install] [package] [version]\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "install") == 0) {
        printf("Installing...\n");
        create_node_folder();
        create_cache_folder();

        install_package(argv[2], argv[3]);
    }

    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Time elapsed: %f\n", cpu_time_used);

    return 0;
}
