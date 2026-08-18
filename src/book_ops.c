#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "book_ops.h"

/* Registers a new book and initializes its ratings */
int register_book(LibrarySystem *sys, Book book) {
    if (!sys) return 0;
    for (int i = 0; i < sys->book_count; i++) {
        if (strcmp(sys->books[i].isbn, book.isbn) == 0) return 0;
    }
    Book *temp = (Book*)realloc(sys->books,
                                (sys->book_count + 1) * sizeof(Book));
    if (!temp) return 0;

    book.total_rating = 0.0;
    book.rating_count = 0;
    sys->books = temp;
    sys->books[sys->book_count] = book;
    sys->book_count++;
    return 1;
}

BookLocation locate_book(const LibrarySystem *sys, const char *isbn) {
    BookLocation loc;
    loc.is_placed = 0;
    loc.shelf_id = -1;
    loc.library_name[0] = '\0';
    if (!sys || !isbn) return loc;

    for (int i = 0; i < sys->library_count; i++) {
        Library *lib = &sys->libraries[i];
        for (int j = 0; j < lib->shelf_count; j++) {
            Shelf *sh = &lib->shelves[j];
            for (int k = 0; k < sh->current_count; k++) {
                if (strcmp(sh->books[k].isbn, isbn) == 0) {
                    loc.is_placed = 1;
                    loc.shelf_id = sh->id;
                    strncpy(loc.library_name, lib->name, MAX_STR - 1);
                    return loc;
                }
            }
        }
    }
    return loc;
}

/* Case-insensitive subsequence matcher for Fuzzy Search */
static int fuzzy_match(const char *pattern, const char *text) {
    if (!pattern || !text) return 0;
    while (*pattern && *text) {
        if (tolower((unsigned char)*pattern) ==
            tolower((unsigned char)*text)) {
            pattern++;
        }
        text++;
    }
    return (*pattern == '\0');
}

/* Formats and prints single book card with ANSI Colors */
static void print_colored_book_card(const Book *b, BookLocation loc) {
    printf(C_CYAN "--------------------------------------------------\n" C_RESET);
    printf("Title     : " C_YELLOW "%s\n" C_RESET, b->title);
    printf("Author    : %s\n", b->author);
    printf("Genre     : %s\n", b->genre);
    printf("ISBN      : %s\n", b->isbn);
    printf("Price     : " C_GREEN "$%.2f\n" C_RESET, b->price);
    
    if (b->rating_count > 0) {
        double avg = b->total_rating / b->rating_count;
        printf("Rating    : ★ %.1f/5.0 (%d reviews)\n", avg,
               b->rating_count);
    } else {
        printf("Rating    : No reviews yet\n");
    }

    if (loc.is_placed) {
        printf("Location  : Branch '%s' -> Shelf #%d\n",
               loc.library_name, loc.shelf_id);
    } else {
        printf("Location  : " C_RED "In Warehouse (Not assigned)" C_RESET "\n");
    }
}

/* Performs Fuzzy Search and Displays Results with Pagination */
void search_and_display_books(const LibrarySystem *sys,
                              const SearchFilter *filter) {
    if (!sys || !filter) return;
    int *matches = (int*)malloc(sys->book_count * sizeof(int));
    int match_count = 0;

    for (int i = 0; i < sys->book_count; i++) {
        Book *b = &sys->books[i];
        int matched = 0;
        
        if (strlen(filter->query) == 0) {
            matched = 1;
        } else if (fuzzy_match(filter->query, b->title) ||
                   fuzzy_match(filter->query, b->author) ||
                   fuzzy_match(filter->query, b->isbn)) {
            matched = 1;
        }

        if (filter->min_price >= 0 && b->price < filter->min_price) matched = 0;
        if (filter->max_price >= 0 && b->price > filter->max_price) matched = 0;

        if (matched) matches[match_count++] = i;
    }

    const int PAGE_SIZE = 20;
    int current_page = 0;
    int total_pages = (match_count + PAGE_SIZE - 1) / PAGE_SIZE;

    if (match_count == 0) {
        printf(C_RED "\nNo books matched your search criteria.\n" C_RESET);
        free(matches);
        return;
    }

    while (1) {
        printf(C_CYAN "\n=== Search Results (Page %d of %d) - Total: %d ===\n"
               C_RESET, current_page + 1, total_pages, match_count);
        
        int start_idx = current_page * PAGE_SIZE;
        int end_idx = start_idx + PAGE_SIZE;
        if (end_idx > match_count) end_idx = match_count;

        for (int i = start_idx; i < end_idx; i++) {
            BookLocation loc = locate_book(sys, sys->books[matches[i]].isbn);
            print_colored_book_card(&sys->books[matches[i]], loc);
        }

        printf(C_YELLOW "\n[N]ext Page | [P]rev Page | [Q]uit to Menu\n" C_RESET);
        printf("Action: ");
        char action[10];
        if (!fgets(action, sizeof(action), stdin)) break;

        char c = tolower((unsigned char)action[0]);
        if (c == 'q') break;
        else if (c == 'n' && current_page < total_pages - 1) current_page++;
        else if (c == 'p' && current_page > 0) current_page--;
        else if (c == 'n' || c == 'p') {
            printf(C_RED "Boundary reached! Cannot go that way.\n" C_RESET);
        }
    }
    free(matches);
}

static void remove_from_shelves(LibrarySystem *sys, const char *isbn) {
    for (int i = 0; i < sys->library_count; i++) {
        Library *lib = &sys->libraries[i];
        for (int j = 0; j < lib->shelf_count; j++) {
            Shelf *sh = &lib->shelves[j];
            for (int k = 0; k < sh->current_count; k++) {
                if (strcmp(sh->books[k].isbn, isbn) == 0) {
                    for (int m = k; m < sh->current_count - 1; m++) {
                        sh->books[m] = sh->books[m + 1];
                    }
                    sh->current_count--;
                    if (sh->current_count == 0) {
                        free(sh->books);
                        sh->books = NULL;
                    }
                    return;
                }
            }
        }
    }
}

int delete_book(LibrarySystem *sys, const char *isbn) {
    if (!sys || !isbn) return 0;
    int target_index = -1;
    for (int i = 0; i < sys->book_count; i++) {
        if (strcmp(sys->books[i].isbn, isbn) == 0) {
            target_index = i;
            break;
        }
    }
    if (target_index == -1) return 0;

    remove_from_shelves(sys, isbn);
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

void display_warehoused_books(const LibrarySystem *sys) {
    if (!sys) return;
    int unassigned = 0;
    printf(C_CYAN "\n=== Warehouse Stock ===\n" C_RESET);
    for (int i = 0; i < sys->book_count; i++) {
        BookLocation loc = locate_book(sys, sys->books[i].isbn);
        if (!loc.is_placed) {
            print_colored_book_card(&sys->books[i], loc);
            unassigned++;
        }
    }
    if (unassigned == 0) {
        printf(C_GREEN "Warehouse is empty. All assigned!\n" C_RESET);
    }
}