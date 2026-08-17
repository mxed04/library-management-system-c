#ifndef STORAGE_H
#define STORAGE_H

#include "library.h"

/* Saves the entire library system state to a binary file */
int save_system_to_file(const LibrarySystem *sys, const char *filepath);

/* Reads a binary file and constructs a new LibrarySystem in memory */
LibrarySystem* load_system_from_file(const char *filepath);

#endif /* STORAGE_H */