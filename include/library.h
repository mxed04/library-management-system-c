#ifndef LIBRARY_H
#define LIBRARY_H

#include "models.h"

/* Initialization and cleanup functions */
LibrarySystem* create_system(void);
void free_system(LibrarySystem *sys);

/* Branch and shelf operations */
int add_library(LibrarySystem *sys, int id, const char *name,
                const char *addr);
int add_shelf(Library *lib, int id, const char *genre, int capacity);

/* Book placement operation */
int add_book_to_library(Library *lib, Book book);

#endif /* LIBRARY_H */