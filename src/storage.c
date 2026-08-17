#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "storage.h"
#include "models.h"

/* Serializes the complete library state and sales to binary file */
int save_system_to_file(const LibrarySystem *sys, const char *filepath) {
    if (!sys || !filepath) return 0;
    FILE *fp = fopen(filepath, "wb");
    if (!fp) return 0;

    /* 1. Write Master Book Catalog */
    fwrite(&sys->book_count, sizeof(int), 1, fp);
    if (sys->book_count > 0) {
        fwrite(sys->books, sizeof(Book), sys->book_count, fp);
    }

    /* 2. Write Libraries and Shelves */
    fwrite(&sys->library_count, sizeof(int), 1, fp);
    for (int i = 0; i < sys->library_count; i++) {
        Library *lib = &sys->libraries[i];
        fwrite(&lib->id, sizeof(int), 1, fp);
        fwrite(lib->name, sizeof(char), MAX_STR, fp);
        fwrite(lib->address, sizeof(char), MAX_STR * 2, fp);
        fwrite(&lib->shelf_count, sizeof(int), 1, fp);
        for (int j = 0; j < lib->shelf_count; j++) {
            Shelf *sh = &lib->shelves[j];
            fwrite(&sh->id, sizeof(int), 1, fp);
            fwrite(sh->genre, sizeof(char), MAX_STR, fp);
            fwrite(&sh->capacity, sizeof(int), 1, fp);
            fwrite(&sh->current_count, sizeof(int), 1, fp);
            if (sh->current_count > 0) {
                fwrite(sh->books, sizeof(Book), sh->current_count, fp);
            }
        }
    }

    /* 3. Write Sales History */
    fwrite(&sys->sale_count, sizeof(int), 1, fp);
    if (sys->sale_count > 0) {
        fwrite(sys->sales, sizeof(SaleRecord), sys->sale_count, fp);
    }

    fclose(fp);
    return 1;
}

/* Deserializes complete state from binary file */
LibrarySystem* load_system_from_file(const char *filepath) {
    if (!filepath) return NULL;
    FILE *fp = fopen(filepath, "rb");
    if (!fp) return NULL;

    LibrarySystem *sys = (LibrarySystem*)malloc(sizeof(LibrarySystem));
    if (!sys) {
        fclose(fp);
        return NULL;
    }
    sys->book_count = 0;
    sys->books = NULL;
    sys->library_count = 0;
    sys->libraries = NULL;
    sys->sale_count = 0;
    sys->sales = NULL;

    /* 1. Read Master Book Catalog */
    if (fread(&sys->book_count, sizeof(int), 1, fp) == 1 &&
        sys->book_count > 0) {
        sys->books = (Book*)malloc(sys->book_count * sizeof(Book));
        if (sys->books) {
            fread(sys->books, sizeof(Book), sys->book_count, fp);
        }
    }

    /* 2. Read Libraries */
    int total_libs = 0;
    if (fread(&total_libs, sizeof(int), 1, fp) == 1) {
        for (int i = 0; i < total_libs; i++) {
            int lib_id = 0, shelf_count = 0;
            char name[MAX_STR], addr[MAX_STR * 2];

            fread(&lib_id, sizeof(int), 1, fp);
            fread(name, sizeof(char), MAX_STR, fp);
            fread(addr, sizeof(char), MAX_STR * 2, fp);
            fread(&shelf_count, sizeof(int), 1, fp);

            add_library(sys, lib_id, name, addr);
            Library *lib = &sys->libraries[sys->library_count - 1];
            for (int j = 0; j < shelf_count; j++) {
                int s_id = 0, cap = 0, cur = 0;
                char s_genre[MAX_STR];

                fread(&s_id, sizeof(int), 1, fp);
                fread(s_genre, sizeof(char), MAX_STR, fp);
                fread(&cap, sizeof(int), 1, fp);
                fread(&cur, sizeof(int), 1, fp);
                add_shelf(lib, s_id, s_genre, cap);
                Shelf *sh = &lib->shelves[lib->shelf_count - 1];
                if (cur > 0) {
                    sh->books = (Book*)malloc(cur * sizeof(Book));
                    if (sh->books) {
                        fread(sh->books, sizeof(Book), cur, fp);
                        sh->current_count = cur;
                    }
                }
            }
        }
    }

    /* 3. Read Sales History */
    if (fread(&sys->sale_count, sizeof(int), 1, fp) == 1 &&
        sys->sale_count > 0) {
        sys->sales = (SaleRecord*)malloc(sys->sale_count *
                                          sizeof(SaleRecord));
        if (sys->sales) {
            fread(sys->sales, sizeof(SaleRecord), sys->sale_count, fp);
        }
    }

    fclose(fp);
    return sys;
}