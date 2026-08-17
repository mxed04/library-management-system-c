#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "book_ops.h"

/* Registers a new book in the central database (initially warehouse) */
int register_book(LibrarySystem *sys, Book book) {
    if (!sys) return 0;

    /* Check for duplicate ISBN */
    for (int i = 0; i < sys->book_count; i++) {
        if (strcmp(sys->books[i].isbn, book.isbn) == 0) {
            return 0; /* Duplicate ISBN not allowed */
        }
    }

    Book *temp = (Book*)realloc(sys->books,
                                (sys->book_count + 1) * sizeof(Book));
    if (!temp) return 0;

    sys->books = temp;
    sys->books[sys->book_count] = book;
    sys->book_count++;
    return 1;
}

/* Locates which library and shelf holds the given book */
BookLocation locate_book(const LibrarySystem *sys, const char *isbn) {
    BookLocation loc;
    loc.is_placed = 0;
    loc.shelf_id = -1;
    loc.library_name[0] = '\0';

    if (!sys || !isbn) return loc;

    for (int i = 0; i < sys->library_count; i++) {
        Library *lib = &sys->libraries[i];
        for (int j = 0; j < lib->shelf_count; j++) {
            Shelf *shelf = &lib->shelves[j];
            for (int k = 0; k < shelf->current_count; k++) {
                if (strcmp(shelf->books[k].isbn, isbn) == 0) {
                    loc.is_placed = 1;
                    loc.shelf_id = shelf->id;
                    strncpy(loc.library_name, lib->name, MAX_STR - 1);
                    loc.library_name[MAX_STR - 1] = '\0';
                    return loc;
                }
            }
        }
    }
    return loc;
}

/* Formats and prints single book information with location */
static void print_book_card(const Book *b, BookLocation loc) {
    printf("--------------------------------------------------\n");
    printf("Title     : %s\n", b->title);
    printf("Author    : %s\n", b->author);
    printf("Publisher : %s\n", b->publisher);
    printf("Genre     : %s\n", b->genre);
    printf("ISBN      : %s\n", b->isbn);
    printf("Price     : $%.2f\n", b->price);
    if (loc.is_placed) {
        printf("Location  : Branch '%s' -> Shelf ID #%d\n",
               loc.library_name, loc.shelf_id);
    } else {
        printf("Location  : In Warehouse (Not assigned to branch)\n");
    }
    printf("--------------------------------------------------\n");
}

/* Checks if a book matches active filter criteria */
static int matches_filter(const Book *b, const SearchFilter *f) {
    if (f->title[0] && !strstr(b->title, f->title)) return 0;
    if (f->author[0] && !strstr(b->author, f->author)) return 0;
    if (f->isbn[0] && strcmp(b->isbn, f->isbn) != 0) return 0;
    if (f->genre[0] && !strstr(b->genre, f->genre)) return 0;
    if (f->publisher[0] && !strstr(b->publisher, f->publisher)) return 0;
    if (f->min_price >= 0 && b->price < f->min_price) return 0;
    if (f->max_price >= 0 && b->price > f->max_price) return 0;
    return 1;
}

/* Searches across the master book database using filters */
void search_and_display_books(const LibrarySystem *sys,
                              const SearchFilter *filter) {
    if (!sys || !filter) return;

    int match_count = 0;
    for (int i = 0; i < sys->book_count; i++) {
        if (matches_filter(&sys->books[i], filter)) {
            BookLocation loc = locate_book(sys, sys->books[i].isbn);
            print_book_card(&sys->books[i], loc);
            match_count++;
        }
    }

    if (match_count == 0) {
        printf("No books matched the search criteria.\n");
    } else {
        printf("Total matched books: %d\n", match_count);
    }
}

/* Deletes a book from physical shelves */
static void remove_from_shelves(LibrarySystem *sys, const char *isbn) {
    for (int i = 0; i < sys->library_count; i++) {
        Library *lib = &sys->libraries[i];
        for (int j = 0; j < lib->shelf_count; j++) {
            Shelf *shelf = &lib->shelves[j];
            for (int k = 0; k < shelf->current_count; k++) {
                if (strcmp(shelf->books[k].isbn, isbn) == 0) {
                    /* Shift books to fill deleted gap */
                    for (int m = k; m < shelf->current_count - 1; m++) {
                        shelf->books[m] = shelf->books[m + 1];
                    }
                    shelf->current_count--;
                    if (shelf->current_count == 0) {
                        free(shelf->books);
                        shelf->books = NULL;
                    }
                    return;
                }
            }
        }
    }
}

/* Completely deletes a book from catalog and associated shelves */
int delete_book(LibrarySystem *sys, const char *isbn) {
    if (!sys || !isbn) return 0;

    int target_index = -1;
    for (int i = 0; i < sys->book_count; i++) {
        if (strcmp(sys->books[i].isbn, isbn) == 0) {
            target_index = i;
            break;
        }
    }

    if (target_index == -1) return 0; /* Book not found */

    /* Remove from physical shelf if assigned */
    remove_from_shelves(sys, isbn);

    /* Shift master catalog */
    for (int i = target_index; i < sys->book_count - 1; i++) {
        sys->books[i] = sys->books[i + 1];
    }
    sys->book_count--;

    if (sys->book_count == 0) {
        free(sys->books);
        sys->books = NULL;
    } else {
        Book *temp = (Book*)realloc(sys->books,
                                    sys->book_count * sizeof(Book));
        if (temp) sys->books = temp;
    }

    return 1;
}

/* Displays all books that are only in the warehouse */
void display_warehoused_books(const LibrarySystem *sys) {
    if (!sys) return;

    int unassigned_count = 0;
    for (int i = 0; i < sys->book_count; i++) {
        BookLocation loc = locate_book(sys, sys->books[i].isbn);
        if (!loc.is_placed) {
            print_book_card(&sys->books[i], loc);
            unassigned_count++;
        }
    }

    if (unassigned_count == 0) {
        printf("Warehouse is empty. All books are assigned to branches.\n");
    } else {
        printf("Total unassigned books in warehouse: %d\n", unassigned_count);
    }
}