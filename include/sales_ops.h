#ifndef SALES_OPS_H
#define SALES_OPS_H

#include "models.h"

/* Checks if a branch has at least one non-full shelf for the genre */
int has_eligible_shelf(const Library *lib, const char *genre);

/* Assigns or transfers book to target branch choosing best shelf */
int assign_book_to_library(LibrarySystem *sys, const char *isbn,
                           int library_id);

/* Processes sale, handles change calculation and logs transaction */
int process_book_sale(LibrarySystem *sys, int library_id,
                      const char *isbn, const char *buyer_name,
                      double paid_amount, double *change_out);

/* Displays purchase history filtered by buyer name */
void display_sales_by_buyer(const LibrarySystem *sys,
                            const char *buyer_name);

/* Displays purchase history filtered by library branch name */
void display_sales_by_library(const LibrarySystem *sys,
                              const char *library_name);

#endif /* SALES_OPS_H */