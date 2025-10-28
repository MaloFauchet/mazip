#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

// Affiche un message d'aide
void help();

int is_directory(char *file_name);

// creates and returns a new file
FILE *get_new_file(char* file_name);

// searches for `needle` in a list of string. Returns 0 if not present, 1 if present
int list_contains(char** string_list, int number_elements, char* needle);

// counts file number recursively
int count_files_recursively(char* file_name);

// Creates a directory. returns 0 if everything went well
int create_dir(char *name, int exist_ok);

// Creates a directory tree. Returns 0 if everything went well.
int create_dir_tree(char *file_path, int exist_ok);

// Returns 1 if file exists. else 0 with errno
int file_exists(char *file_path);

#endif