#ifndef ARCHIVE_H
#define ARCHIVE_H

#include <stdio.h>
#include <stdint.h>

// Taille fixe du beggining magic number
#define BEG_MAGIC_SIZE 28
#define END_OF_ANY_FILE_MAGIC_SIZE 4
#define EOF_MAGIC_SIZE 26

// Le magic number
extern const char BEG_MAGIC[BEG_MAGIC_SIZE];
extern const char END_OF_ANY_FILE_MAGIC[END_OF_ANY_FILE_MAGIC_SIZE];
extern const char EOF_MAGIC[EOF_MAGIC_SIZE];

int create_archive(int argc, char** argv);

int write_file_to_archive(FILE *f, char *file_name);

int unzip_archive(int argc, char** argv);

int construct_file(FILE *f);


// Ecrit le beginning magic number, retourne 0 si succes
int write_beg_magic(FILE *f);

// Lit et vérifie le magic number dans un fichier, retourne 0 si OK
int read_beg_magic(FILE *f);

int write_end_of_any_file_magic(FILE *f);

int read_end_of_any_file_magic(FILE *f);

int write_eof_magic(FILE *f);

int write_file_number(FILE *f, uint32_t file_number);

int write_file_name_length(FILE *f, uint16_t file_name_length);

int write_file_length(FILE *f, uint64_t file_length);

int write_file_content(FILE *f, char* file_name);

int write_string(FILE *f, char* string);

#endif