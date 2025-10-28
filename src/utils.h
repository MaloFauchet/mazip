#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

// Affiche un message d'aide
void help();

// Test si file_name existe. 1 si il existe, 0 sinon
int file_exists(char* file_name);

int is_directory(char *file_name);

// creates and returns a new file
FILE *get_new_file(char* file_name);

// searches for `needle` in a list of string. Returns 0 if not present, 1 if present
int list_contains(char** string_list, int number_elements, char* needle);

// counts file number recursively
int count_files_recursively(char* file_name);

#endif