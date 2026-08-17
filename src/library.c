#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "library.h"

/* Initializes an empty library system */
LibrarySystem* create_system(void) {
    LibrarySystem *sys = (LibrarySystem*)malloc(sizeof(LibrarySystem));
    if (!sys) return NULL;
    sys->library_count = 0;
    sys->libraries = NULL;
    sys->book_count = 0;
    sys->books = NULL;
    return sys;
}

/* Adds a new library branch using realloc */
int add_library(LibrarySystem *sys, int id, const char *name,
                const char *addr) {
    if (!sys) return 0;

    Library *temp = (Library*)realloc(sys->libraries,
                                      (sys->library_count + 1) *
                                      sizeof(Library));
    if (!temp) return 0;

    sys->libraries = temp;
    Library *lib = &sys->libraries[sys->library_count];
    lib->id = id;
    strncpy(lib->name, name, MAX_STR - 1);
    lib->name[MAX_STR - 1] = '\0';
    strncpy(lib->address, addr, (MAX_STR * 2) - 1);
    lib->address[(MAX_STR * 2) - 1] = '\0';
    lib->shelf_count = 0;
    lib->shelves = NULL;

    sys->library_count++;
    return 1;
}

/* Adds a new shelf to a library branch */
int add_shelf(Library *lib, int id, const char *genre, int capacity) {
    if (!lib || capacity <= 0) return 0;

    Shelf *temp = (Shelf*)realloc(lib->shelves,
                                  (lib->shelf_count + 1) * sizeof(Shelf));
    if (!temp) return 0;

    lib->shelves = temp;
    Shelf *shelf = &lib->shelves[lib->shelf_count];
    shelf->id = id;
    strncpy(shelf->genre, genre, MAX_STR - 1);
    shelf->genre[MAX_STR - 1] = '\0';
    shelf->capacity = capacity;
    shelf->current_count = 0;
    shelf->books = NULL;

    lib->shelf_count++;
    return 1;
}

/* Places a book into the first eligible shelf with free capacity */
int add_book_to_library(Library *lib, Book book) {
    if (!lib) return 0;

    for (int i = 0; i < lib->shelf_count; i++) {
        Shelf *shelf = &lib->shelves[i];
        
        /* Check genre match and available space */
        if (strcmp(shelf->genre, book.genre) == 0 &&
            shelf->current_count < shelf->capacity) {
            
            Book *temp = (Book*)realloc(shelf->books,
                                        (shelf->current_count + 1) *
                                        sizeof(Book));
            if (!temp) return 0;

            shelf->books = temp;
            shelf->books[shelf->current_count] = book;
            shelf->current_count++;
            return 1; /* Successfully placed */
        }
    }

    return 0; /* No matching or non-full shelf found */
}

/* Safely deallocates all dynamically allocated memory */
void free_system(LibrarySystem *sys) {
    if (!sys) return;

    /* Free all shelves and their books in each library */
    for (int i = 0; i < sys->library_count; i++) {
        Library *lib = &sys->libraries[i];
        for (int j = 0; j < lib->shelf_count; j++) {
            free(lib->shelves[j].books);
        }
        free(lib->shelves);
    }
    free(sys->libraries);

    /* Free master book catalog */
    free(sys->books);
    free(sys);
}