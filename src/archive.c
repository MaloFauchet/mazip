#include "utils.h"
#include "archive.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

const char BEG_MAGIC[BEG_MAGIC_SIZE] = "FILE ENCODED BY MALO FAUCHET";
const char END_OF_ANY_FILE_MAGIC[END_OF_ANY_FILE_MAGIC_SIZE] = {0x69, 0x69, 0x69, 0x69};
const char EOF_MAGIC[EOF_MAGIC_SIZE] = "EOF ENCODE BY MALO FAUCHET";

int create_archive(int argc, char** argv) {
    // if there is no archive name
    if (argc <= 2) {
        fprintf(stderr, "Error : no archive name given.\nUsage :\n");
        help();
        return 1;
    }

    // getting archive name
    char archive_name[256];
    strcpy(archive_name, argv[2]);

    // if there are no file given
    if (argc <= 3) {
        fprintf(stderr, "Error : no file to put in archive given.\n");
        printf("Usage :\n");
        help();
        return 2;
    }

    // if the archive already exists
    if (file_exists(archive_name) && (!list_contains(argv, argc, "-f") && !list_contains(argv, argc, "--force-write"))) {
        fprintf(stderr, "Error : the archive already exists as another file. Can't overwrite it.\n");
        return 3;
    }

    // checking that each file given exists
    // TODO: check that there are no doublons
    // TODO: add flags checks
    argv += 3;
    int i;
    int file_nb = 0;
    for (i = 0; i < argc-3; i++) {
        if (!file_exists(argv[i])) {
            fprintf(stderr, "Error: %s not found.\n", argv[i]);
            return 4;
        }
        file_nb += count_files_recursively(argv[i]);
    }

    // Get a new file handler
    FILE *f = get_new_file(archive_name);
    
    // write the magic number of the beginning
    write_beg_magic(f);

    // write the number of file
    write_file_number(f, file_nb);

    for (int i = 0; i < argc-3; i++) {
        write_file_to_archive(f, argv[i]);
    }

    write_eof_magic(f);

    fclose(f);
    printf("Archive written successfully\n");
    
    return 0;
}

int write_file_to_archive(FILE *f, char *file_name) {
    if (is_directory(file_name) != 0) {
        struct dirent *de;
        DIR *dr = opendir(file_name);
        if (dr == NULL) {
            printf("Could not open %s directory\n", file_name);
            return 1;
        }

        while ((de = readdir(dr)) != NULL) {
            if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) {
                continue;
            }

            size_t need = strlen(file_name) + 1 /* '/' */ + strlen(de->d_name) + 1 /* '\0' */;
            char *new_file_name = malloc(need);
            if (!new_file_name) {
                perror("malloc");
                closedir(dr);
                return -1;
            }
            snprintf(new_file_name, need, "%s/%s", file_name, de->d_name);
            int ret = write_file_to_archive(f, new_file_name);
            free(new_file_name);
            if (ret != 0) {
                break;
            }
        }
        
        closedir(dr);
        return 0;
    }

    struct stat *buffer;  // buffer for file informations
    buffer = malloc(sizeof(struct stat));

    // copier le nom du fichier
    // printf("%s ", file_name);
    // fflush(stdout);
    // get the file informations
    if (stat(file_name, buffer) != 0) {
        printf("Error while getting file info. file_name = %s\n", file_name);
        perror("");
        free(buffer);
        return -1;
    }

    // write the size of the name of that file
    write_file_name_length(f, strlen(file_name));
    // write the file name
    write_string(f, file_name);

    // write file size
    write_file_length(f, buffer->st_size);
    // write file content
    write_file_content(f, file_name);

    write_end_of_any_file_magic(f);

    free(buffer);

    return 0;
}

int unzip_archive(int argc, char** argv) {
    // if there is no archive name
    if (argc <= 2) {
        fprintf(stderr, "Error : no archive name given.\n");
        printf("Usage :\n");
        help();
        return 1;
    }

    char *archive_name = argv[2];
    if (!file_exists(archive_name)) {
        fprintf(stderr, "Error: \'%s\' archive was not found.\n", archive_name);
        fprintf(stderr, "Exiting...\n");
        return -1;
    }

    // open the archive
    FILE *archive = fopen(argv[2], "rb");

    // check for magic archive beginning number
    if (read_beg_magic(archive) != 0) {
        fprintf(stderr, "Error: Broken archive.\n");
        return -1;
    }

    // read the number of files present in the archive
    uint32_t nb_files;
    size_t read = fread(&nb_files, 1, sizeof(uint32_t), archive);
    if (read != sizeof(uint32_t)) {
        fprintf(stderr, "Error: Broken archive.\n");
        return -1;
    }

    // construct each files
    uint32_t nb_file_constructed;
    for (nb_file_constructed = 0; nb_file_constructed < nb_files; nb_file_constructed++) {
        if (construct_file(archive) != 0) {
            fprintf(stderr, "Error: Broken archive.\n");
            return -1;
        }
        if (read_end_of_any_file_magic(archive) != 0) {
            fprintf(stderr, "Error: Broken archive.\n");
            fprintf(stderr, "Wrong EOAF magic number.\n");
            return -1;
        }
    }
    

    printf("Archive read successfully\n");
    return 0;
}

int construct_file(FILE *f) {
    // read the file name size
    uint16_t file_name_size;
    size_t read = fread(&file_name_size, 1, sizeof(uint16_t), f);
    if (read != sizeof(uint16_t)) {
        fprintf(stderr, "Error: Broken archive.\n");
        return -1;
    }

    // read the file name (allocate room for terminating NUL)
    char *file_name = malloc((size_t)file_name_size + 1);
    if (!file_name) {
        fprintf(stderr, "Error: Out of memory.\n");
        return -1;
    }
    if (fread(file_name, 1, file_name_size, f) != file_name_size) {
        free(file_name);
        fprintf(stderr, "Error: Broken archive.\n");
        return -1;
    }
    file_name[file_name_size] = '\0'; // ensure NUL-terminated
    create_dir_tree(file_name, 1);
    FILE *file = get_new_file(file_name);

    printf("Extracting %s\n", file_name);

    // read size of file
    uint64_t file_size;
    read = fread(&file_size, 1, sizeof(uint64_t), f);
    if (read != sizeof(uint64_t)) {
        fprintf(stderr, "Error: Broken archive.\n");
        return -1;
    }

    char buffer[BUFSIZ];
    uint64_t remaining = file_size;

    // read exactly `file_size` bytes in chunks; do not read past this entry
    while (remaining > 0) {
        size_t to_read = (remaining < BUFSIZ) ? (size_t)remaining : BUFSIZ;
        size_t got = fread(buffer, 1, to_read, f);
        if (got == 0) {
            if (feof(f)) {
                fprintf(stderr, "Error: Broken archive (unexpected EOF).\n");
            } else {
                fprintf(stderr, "Error: Read failure.\n");
            }
            free(file_name);
            return -1;
        }

        // Process `got` bytes from buffer here
        // e.g. verify, checksum, skip, etc.
        fwrite(buffer, 1, got, file);
        remaining -= got;
    }
    free(file_name);
    return 0;
}

int write_beg_magic(FILE *f) {
    size_t written = fwrite(BEG_MAGIC, 1, BEG_MAGIC_SIZE, f);
    if (written != BEG_MAGIC_SIZE) {
        return -1;
    }
    return 0;
}

int read_beg_magic(FILE *f) {
    char buffer[BEG_MAGIC_SIZE];
    size_t read = fread(buffer, 1, BEG_MAGIC_SIZE, f);
    if (read != BEG_MAGIC_SIZE) {
        return -1;
    }
    if (memcmp(buffer, BEG_MAGIC, BEG_MAGIC_SIZE)) {
        return -2;
    }
    return 0;
}

int write_end_of_any_file_magic(FILE *f) {
    size_t written = fwrite(END_OF_ANY_FILE_MAGIC, 1, END_OF_ANY_FILE_MAGIC_SIZE, f);
    if (written != END_OF_ANY_FILE_MAGIC_SIZE) {
        return -1;
    }
    return 0;
}

int read_end_of_any_file_magic(FILE *f) {
    char buffer[END_OF_ANY_FILE_MAGIC_SIZE];
    size_t read = fread(buffer, 1, END_OF_ANY_FILE_MAGIC_SIZE, f);
    if (read != END_OF_ANY_FILE_MAGIC_SIZE) {
        return -1;
    }
    if (memcmp(buffer, END_OF_ANY_FILE_MAGIC, END_OF_ANY_FILE_MAGIC_SIZE)) {
        return -2;
    }
    return 0;
}

int write_eof_magic(FILE *f) {
    size_t written = fwrite(EOF_MAGIC, 1, EOF_MAGIC_SIZE, f);
    if (written != EOF_MAGIC_SIZE) {
        return -1;
    }
    return 0;
}

int write_file_number(FILE *f, uint32_t file_number) {
    size_t written = fwrite(&file_number, 1, sizeof(uint32_t), f);
    if (written != sizeof(uint32_t)) {
        return -1;
    }
    return 0;
}

int write_file_name_length(FILE *f, uint16_t file_name_length) {
    size_t written = fwrite(&file_name_length, 1, sizeof(uint16_t), f);
    if (written != sizeof(uint16_t)) {
        return -1;
    }
    return 0;
}

int write_file_length(FILE *f, uint64_t file_length) {
    size_t written = fwrite(&file_length, 1, sizeof(uint64_t), f);
    if (written != sizeof(uint64_t)) {
        return -1;
    }
    return 0;
}

int write_file_content(FILE *f, char* file_name) {
    FILE *file_to_copy = fopen(file_name, "rb");
    char buffer[BUFSIZ];
    size_t size_read;
    while ((size_read = fread(buffer, 1, BUFSIZ, file_to_copy)) > 0) {
        fwrite(buffer, size_read, 1, f);
    }
    fclose(file_to_copy);
    return 0;
}

int write_string(FILE *f, char* string) {
    size_t written = fwrite(string, 1, strlen(string), f);
    if (written != strlen(string)) {
        return -1;
    }
    return 0;
}