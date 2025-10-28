#include "utils.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>

void help() {
    printf("\thelp : To display this message\n");
    printf("\tzip <archive_name> <file> [<file>, ...] : to zip files\n");
    printf("\tunzip <file> : to unzip an archive\n");
    printf("\n");
    printf("zip options:\n");
    printf("\t-f --force-write : Allow overwriting an existing file for the archive\n");
}

int file_exists(char* file_name) {
    if (access(file_name, F_OK) == 0) {
        return 1; // if it exists
    }
    return 0;
}

int is_directory(char *file_name) {
    struct stat statbuf;
    if (stat(file_name, &statbuf) != 0)
        return 0;
    // printf("Is %s a directory : %d\n", file_name, S_ISDIR(statbuf.st_mode));
    return S_ISDIR(statbuf.st_mode);
}

FILE *get_new_file(char* file_name) {
    FILE *f = fopen(file_name, "wb");
    return f;
}

int list_contains(char** string_list, int number_elements, char* needle) {
    for (int i = 0; i < number_elements; i++) {
        if (strcmp(string_list[i], needle) == 0) {
            return 1;
        }
    }
    return 0;    
}

int count_files_recursively(char* file_name) {
    // printf("current file_name = %s\n", file_name);
    if (!file_exists(file_name)) {
        return 0;
    }
    
    if (!is_directory(file_name)) {
        return 1;
    }

    int file_number = 0;
    struct dirent *de;
    DIR *dr = opendir(file_name);
    if (dr == NULL) {
        // printf("Could not open %s directory\n", file_name);
        return 0;
    }

    while ((de = readdir(dr)) != NULL) {
        // get the file name
        char* cur_file_name = de->d_name;

        // if the file_name is excluded, skip it.
        if (strcmp(cur_file_name, ".") == 0 || strcmp(cur_file_name, "..") == 0) {
            // printf("skipping %s\n", file_name);
            continue;
        }

        // if file_name is a directory, enter it and count it's file number
        if (is_directory(cur_file_name)) {
            file_number += count_files_recursively(strcat(strcat(file_name, "/"), cur_file_name));
        }

        file_number += 1;
    }
    
    closedir(dr);
    printf("file_number = %d\n", file_number);
    return file_number;
}