#ifndef BOOK_OPS_H
#define BOOK_OPS_H

#include "models.h"

/* Registers a new book into the master catalog/database */
int register_book(LibrarySystem *sys, Book book);

/* Finds the location of a book by ISBN (branch/shelf or warehouse) */
BookLocation locate_book(const LibrarySystem *sys, const char *isbn);

/* Searches master catalog by multiple filters and prints results */
void search_and_display_books(const LibrarySystem *sys,
                              const SearchFilter *filter);

/* Removes a book from master catalog and any shelf it resides on */
int delete_book(LibrarySystem *sys, const char *isbn);

/* Displays all books currently residing in the warehouse */
void display_warehoused_books(const LibrarySystem *sys);

#endif /* BOOK_OPS_H */