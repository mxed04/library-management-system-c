#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sales_ops.h"
#include "book_ops.h"

/* Checks if branch has space in shelves matching the book genre */
int has_eligible_shelf(const Library *lib, const char *genre) {
    if (!lib || !genre) return 0;
    for (int i = 0; i < lib->shelf_count; i++) {
        if (strcmp(lib->shelves[i].genre, genre) == 0 &&
            lib->shelves[i].current_count < lib->shelves[i].capacity) {
            return 1;
        }
    }
    return 0;
}

/* Helper to remove book from a shelf index */
static void remove_book_from_shelf(Library *lib, int s_idx, int b_idx) {
    Shelf *sh = &lib->shelves[s_idx];
    for (int i = b_idx; i < sh->current_count - 1; i++) {
        sh->books[i] = sh->books[i + 1];
    }
    sh->current_count--;
    if (sh->current_count == 0) {
        free(sh->books);
        sh->books = NULL;
    } else {
        Book *temp = (Book*)realloc(sh->books,
                                    sh->current_count * sizeof(Book));
        if (temp) sh->books = temp;
    }
}

/* Removes book from whatever branch/shelf it is currently placed in */
static void unassign_book_everywhere(LibrarySystem *sys,
                                     const char *isbn) {
    for (int i = 0; i < sys->library_count; i++) {
        Library *lib = &sys->libraries[i];
        for (int j = 0; j < lib->shelf_count; j++) {
            Shelf *sh = &lib->shelves[j];
            for (int k = 0; k < sh->current_count; k++) {
                if (strcmp(sh->books[k].isbn, isbn) == 0) {
                    remove_book_from_shelf(lib, j, k);
                    return;
                }
            }
        }
    }
}

/* Assigns book choosing shelf with least % free space */
int assign_book_to_library(LibrarySystem *sys, const char *isbn,
                           int library_id) {
    if (!sys || !isbn) return 0;
    Book target_book;
    int found = 0;
    for (int i = 0; i < sys->book_count; i++) {
        if (strcmp(sys->books[i].isbn, isbn) == 0) {
            target_book = sys->books[i];
            found = 1;
            break;
        }
    }
    if (!found) return 0;

    Library *dest_lib = NULL;
    for (int i = 0; i < sys->library_count; i++) {
        if (sys->libraries[i].id == library_id) {
            dest_lib = &sys->libraries[i];
            break;
        }
    }
    if (!dest_lib) return 0;

    /* Priority: shelf with smallest free ratio (fullest shelf) */
    int best_shelf_idx = -1;
    double min_free_ratio = 2.0;

    for (int j = 0; j < dest_lib->shelf_count; j++) {
        Shelf *sh = &dest_lib->shelves[j];
        if (strcmp(sh->genre, target_book.genre) == 0 &&
            sh->current_count < sh->capacity) {
            double free_ratio = (double)(sh->capacity - sh->current_count)
                                / (double)sh->capacity;
            if (free_ratio < min_free_ratio) {
                min_free_ratio = free_ratio;
                best_shelf_idx = j;
            }
        }
    }

    if (best_shelf_idx == -1) return 0;
    unassign_book_everywhere(sys, isbn);

    Shelf *shelf = &dest_lib->shelves[best_shelf_idx];
    Book *temp = (Book*)realloc(shelf->books,
                                (shelf->current_count + 1) * sizeof(Book));
    if (!temp) return 0;

    shelf->books = temp;
    shelf->books[shelf->current_count] = target_book;
    shelf->current_count++;
    return 1;
}

/* Processes selling a book, logs record and removes from system */
int process_book_sale(LibrarySystem *sys, int library_id,
                      const char *isbn, const char *buyer_name,
                      double paid_amount, double *change_out) {
    if (!sys || !isbn || !buyer_name) return 0;
    Library *lib = NULL;
    for (int i = 0; i < sys->library_count; i++) {
        if (sys->libraries[i].id == library_id) {
            lib = &sys->libraries[i];
            break;
        }
    }
    if (!lib) return 0;

    int shelf_idx = -1, book_idx = -1;
    Book book_to_sell;

    for (int j = 0; j < lib->shelf_count; j++) {
        Shelf *sh = &lib->shelves[j];
        for (int k = 0; k < sh->current_count; k++) {
            if (strcmp(sh->books[k].isbn, isbn) == 0) {
                shelf_idx = j;
                book_idx = k;
                book_to_sell = sh->books[k];
                break;
            }
        }
        if (shelf_idx != -1) break;
    }

    if (shelf_idx == -1) return 0;
    if (paid_amount < book_to_sell.price) {
        return -1;
        /* Insufficient payment error */
    }

    if (change_out) {
        *change_out = paid_amount - book_to_sell.price;
    }

    remove_book_from_shelf(lib, shelf_idx, book_idx);
    delete_book(sys, isbn);

    /* Log sale transaction */
    SaleRecord record;
    strncpy(record.buyer_name, buyer_name, MAX_STR - 1);
    record.buyer_name[MAX_STR - 1] = '\0';
    strncpy(record.book_title, book_to_sell.title, MAX_STR - 1);
    record.book_title[MAX_STR - 1] = '\0';
    strncpy(record.book_isbn, book_to_sell.isbn, MAX_ISBN - 1);
    record.book_isbn[MAX_ISBN - 1] = '\0';
    strncpy(record.library_name, lib->name, MAX_STR - 1);
    record.library_name[MAX_STR - 1] = '\0';
    record.price = book_to_sell.price;
    record.paid_amount = paid_amount;
    record.change = paid_amount - book_to_sell.price;

    SaleRecord *temp = (SaleRecord*)realloc(sys->sales,
                                            (sys->sale_count + 1) *
                                            sizeof(SaleRecord));
    if (temp) {
        sys->sales = temp;
        sys->sales[sys->sale_count] = record;
        sys->sale_count++;
    }
    return 1;
}

static void print_sale_card(const SaleRecord *s) {
    printf("--------------------------------------------------\n");
    printf("Buyer Name   : %s\n", s->buyer_name);
    printf("Book Title   : %s\n", s->book_title);
    printf("Book ISBN    : %s\n", s->book_isbn);
    printf("Library      : %s\n", s->library_name);
    printf("Price        : $%.2f\n", s->price);
    printf("Paid Amount  : $%.2f\n", s->paid_amount);
    printf("Change Given : $%.2f\n", s->change);
    printf("--------------------------------------------------\n");
}

void display_sales_by_buyer(const LibrarySystem *sys,
                            const char *buyer_name) {
    if (!sys || !buyer_name) return;
    int count = 0;
    for (int i = 0; i < sys->sale_count; i++) {
        if (strstr(sys->sales[i].buyer_name, buyer_name)) {
            print_sale_card(&sys->sales[i]);
            count++;
        }
    }
    if (count == 0) {
        printf("No records found for buyer '%s'.\n", buyer_name);
    }
}

void display_sales_by_library(const LibrarySystem *sys,
                              const char *library_name) {
    if (!sys || !library_name) return;
    int count = 0;
    for (int i = 0; i < sys->sale_count; i++) {
        if (strstr(sys->sales[i].library_name, library_name)) {
            print_sale_card(&sys->sales[i]);
            count++;
        }
    }
    if (count == 0) {
        printf("No records found for library '%s'.\n", library_name);
    }
}