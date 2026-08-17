#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "storage.h"
#include "models.h"

/* Saves all libraries, shelves, and books to a binary file */
int save_system_to_file(const LibrarySystem *sys, const char *filepath) {
    if (!sys || !filepath) return 0;

    FILE *fp = fopen(filepath, "wb");
    if (!fp) return 0;

    /* Write the total count of library branches */
    if (fwrite(&sys->library_count, sizeof(int), 1, fp) != 1) {
        fclose(fp);
        return 0;
    }

    /* Iterate through libraries and serialize their data */
    for (int i = 0; i < sys->library_count; i++) {
        Library *lib = &sys->libraries[i];

        fwrite(&lib->id, sizeof(int), 1, fp);
        fwrite(lib->name, sizeof(char), MAX_STR, fp);
        fwrite(lib->address, sizeof(char), MAX_STR * 2, fp);
        fwrite(&lib->shelf_count, sizeof(int), 1, fp);

        /* Write shelves belonging to this library */
        for (int j = 0; j < lib->shelf_count; j++) {
            Shelf *shelf = &lib->shelves[j];

            fwrite(&shelf->id, sizeof(int), 1, fp);
            fwrite(shelf->genre, sizeof(char), MAX_STR, fp);
            fwrite(&shelf->capacity, sizeof(int), 1, fp);
            fwrite(&shelf->current_count, sizeof(int), 1, fp);

            /* Write book array stored on this shelf */
            if (shelf->current_count > 0) {
                fwrite(shelf->books, sizeof(Book),
                       shelf->current_count, fp);
            }
        }
    }

    fclose(fp);
    return 1;
}

/* Loads and dynamically reconstructs system state from file */
LibrarySystem* load_system_from_file(const char *filepath) {
    if (!filepath) return NULL;

    FILE *fp = fopen(filepath, "rb");
    if (!fp) return NULL;

    LibrarySystem *sys = (LibrarySystem*)malloc(sizeof(LibrarySystem));
    if (!sys) {
        fclose(fp);
        return NULL;
    }

    sys->library_count = 0;
    sys->libraries = NULL;

    int total_libs = 0;
    if (fread(&total_libs, sizeof(int), 1, fp) != 1) {
        fclose(fp);
        free(sys);
        return NULL;
    }

    for (int i = 0; i < total_libs; i++) {
        int lib_id = 0;
        char name[MAX_STR];
        char address[MAX_STR * 2];
        int shelf_count = 0;

        fread(&lib_id, sizeof(int), 1, fp);
        fread(name, sizeof(char), MAX_STR, fp);
        fread(address, sizeof(char), MAX_STR * 2, fp);
        fread(&shelf_count, sizeof(int), 1, fp);

        if (!add_library(sys, lib_id, name, address)) {
            free_system(sys);
            fclose(fp);
            return NULL;
        }

        Library *lib = &sys->libraries[sys->library_count - 1];

        for (int j = 0; j < shelf_count; j++) {
            int shelf_id = 0;
            char genre[MAX_STR];
            int capacity = 0;
            int current_count = 0;

            fread(&shelf_id, sizeof(int), 1, fp);
            fread(genre, sizeof(char), MAX_STR, fp);
            fread(&capacity, sizeof(int), 1, fp);
            fread(&current_count, sizeof(int), 1, fp);

            if (!add_shelf(lib, shelf_id, genre, capacity)) {
                free_system(sys);
                fclose(fp);
                return NULL;
            }

            Shelf *shelf = &lib->shelves[lib->shelf_count - 1];
            if (current_count > 0) {
                shelf->books = (Book*)malloc(current_count *
                                             sizeof(Book));
                if (!shelf->books) {
                    free_system(sys);
                    fclose(fp);
                    return NULL;
                }
                fread(shelf->books, sizeof(Book), current_count, fp);
                shelf->current_count = current_count;
            }
        }
    }

    fclose(fp);
    return sys;
}