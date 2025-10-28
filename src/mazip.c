#include "archive.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char** argv) {
    // check that a command was given
    if ((argc < 2) || list_contains(argv, argc, "--help") || list_contains(argv, argc, "-h")) {
        printf("Usage:\n");
        help();
        return 0;
    }
    
    int return_code;
    if (strcmp(argv[1], "help") == 0) {
        help();
        return_code = 0;
    } else if (strcmp(argv[1], "zip") == 0) {
        return_code = create_archive(argc, argv);
    } else if (strcmp(argv[1], "unzip") == 0) {
        // printf("unzipping archive\n");
        return_code = unzip_archive(argc, argv);
    } else {
        printf("Unknown command: %s\nUsage:\n", argv[1]);
        help();
        return_code = 1;
    }

    return return_code;
}