#include <stdio.h>
#include "requests.h"

int main() {
    char* res = get_package("requests");
    printf("%s", res);
    return 0;
}
