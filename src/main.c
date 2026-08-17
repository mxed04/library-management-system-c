#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "models.h"
#include "library.h"
#include "book_ops.h"
#include "sales_ops.h"
#include "storage.h"

#define DATA_FILE "data/library_data.dat"

/* Helper to safely read a string from stdin */
static void read_line(char *buffer, int size) {
    if (fgets(buffer, size, stdin)) {
        buffer[strcspn(buffer, "\n")] = '\0';
    }
}

/* Helper to setup demo branch structure if file does not exist */
static void setup_initial_branches(LibrarySystem *sys) {
    if (sys->library_count == 0) {
        add_library(sys, 1, "Central Library", "Main Ave #10");
        add_shelf(&sys->libraries[0], 101, "Fiction", 5);
        add_shelf(&sys->libraries[0], 102, "Fiction", 10);
        add_shelf(&sys->libraries[0], 103, "Science", 8);

        add_library(sys, 2, "North Branch", "North Blvd #45");
        add_shelf(&sys->libraries[1], 201, "History", 6);
        add_shelf(&sys->libraries[1], 202, "Science", 4);
    }
}

static void ui_add_book(LibrarySystem *sys) {
    Book b;
    printf("\n--- Add New Book ---\n");
    printf("Enter Title: ");
    read_line(b.title, MAX_STR);
    printf("Enter Author: ");
    read_line(b.author, MAX_STR);
    printf("Enter Publisher: ");
    read_line(b.publisher, MAX_STR);
    printf("Enter Genre: ");
    read_line(b.genre, MAX_STR);
    printf("Enter ISBN: ");
    read_line(b.isbn, MAX_ISBN);
    printf("Enter Price: ");
    scanf("%lf", &b.price);
    getchar();
    if (register_book(sys, b)) {
        printf("Success: Book added to master database (Warehouse).\n");
    } else {
        printf("Error: Book with ISBN '%s' already exists.\n", b.isbn);
    }
}

static void ui_search_books(const LibrarySystem *sys) {
    SearchFilter f;
    memset(&f, 0, sizeof(SearchFilter));
    f.min_price = -1;
    f.max_price = -1;
    printf("\n--- Search Books (Leave blank to skip filter) ---\n");
    printf("Filter Title: ");
    read_line(f.title, MAX_STR);
    printf("Filter Author: ");
    read_line(f.author, MAX_STR);
    printf("Filter ISBN: ");
    read_line(f.isbn, MAX_ISBN);
    printf("Filter Genre: ");
    read_line(f.genre, MAX_STR);
    printf("Filter Publisher: ");
    read_line(f.publisher, MAX_STR);
    printf("Filter Min Price (-1 for none): ");
    scanf("%lf", &f.min_price);
    printf("Filter Max Price (-1 for none): ");
    scanf("%lf", &f.max_price);
    getchar();
    search_and_display_books(sys, &f);
}

static void ui_delete_book(LibrarySystem *sys) {
    char isbn[MAX_ISBN];
    printf("\n--- Delete Book ---\n");
    printf("Enter ISBN of book to remove: ");
    read_line(isbn, MAX_ISBN);

    if (delete_book(sys, isbn)) {
        printf("Success: Book deleted from catalog and all shelves.\n");
    } else {
        printf("Error: Book not found in database.\n");
    }
}

static void ui_assign_book(LibrarySystem *sys) {
    char isbn[MAX_ISBN];
    printf("\n--- Assign / Transfer Book to Library ---\n");
    printf("Enter Book ISBN: ");
    read_line(isbn, MAX_ISBN);

    Book *target = NULL;
    for (int i = 0; i < sys->book_count; i++) {
        if (strcmp(sys->books[i].isbn, isbn) == 0) {
            target = &sys->books[i];
            break;
        }
    }
    if (!target) {
        printf("Error: Book not found.\n");
        return;
    }

    printf("\nEligible Libraries for Genre '%s':\n", target->genre);
    int eligible_count = 0;
    for (int i = 0; i < sys->library_count; i++) {
        if (has_eligible_shelf(&sys->libraries[i], target->genre)) {
            printf("[%d] %s (%s)\n", sys->libraries[i].id,
                   sys->libraries[i].name, sys->libraries[i].address);
            eligible_count++;
        }
    }

    if (eligible_count == 0) {
        printf("No branches currently have free space for this genre.\n");
        return;
    }

    int lib_id;
    printf("Select Library ID: ");
    scanf("%d", &lib_id);
    getchar();
    if (assign_book_to_library(sys, isbn, lib_id)) {
        printf("Success: Book placed in highest priority shelf.\n");
    } else {
        printf("Error: Failed to assign book to the selected branch.\n");
    }
}

static void ui_sell_book(LibrarySystem *sys) {
    printf("\n--- Sell Book ---\n");
    printf("Select Branch:\n");
    for (int i = 0; i < sys->library_count; i++) {
        printf("[%d] %s\n", sys->libraries[i].id, sys->libraries[i].name);
    }
    int lib_id;
    printf("Enter Branch ID: ");
    scanf("%d", &lib_id);
    getchar();

    Library *lib = NULL;
    for (int i = 0; i < sys->library_count; i++) {
        if (sys->libraries[i].id == lib_id) {
            lib = &sys->libraries[i];
            break;
        }
    }
    if (!lib) {
        printf("Error: Branch not found.\n");
        return;
    }

    printf("\nBooks available in %s:\n", lib->name);
    int total_books = 0;
    for (int j = 0; j < lib->shelf_count; j++) {
        for (int k = 0; k < lib->shelves[j].current_count; k++) {
            printf("%d. Title: %s\n", total_books + 1,
                   lib->shelves[j].books[k].title);
            total_books++;
        }
    }

    if (total_books == 0) {
        printf("This branch currently has no books.\n");
        return;
    }

    char isbn[MAX_ISBN];
    printf("Enter ISBN of book to buy: ");
    read_line(isbn, MAX_ISBN);

    Book *selected_book = NULL;
    for (int j = 0; j < lib->shelf_count; j++) {
        for (int k = 0; k < lib->shelves[j].current_count; k++) {
            if (strcmp(lib->shelves[j].books[k].isbn, isbn) == 0) {
                selected_book = &lib->shelves[j].books[k];
                break;
            }
        }
    }

    if (!selected_book) {
        printf("Error: Book not found in this branch.\n");
        return;
    }

    printf("\n--- Book Details ---\n");
    printf("Title: %s | Author: %s | Price: $%.2f\n",
           selected_book->title, selected_book->author,
           selected_book->price);
    char buyer[MAX_STR];
    double paid = 0.0, change = 0.0;
    printf("Enter Buyer Name: ");
    read_line(buyer, MAX_STR);
    printf("Enter Paid Amount: $");
    scanf("%lf", &paid);
    getchar();

    int res = process_book_sale(sys, lib_id, isbn, buyer, paid, &change);
    if (res == 1) {
        printf("\nSale completed! Change to return: $%.2f\n", change);
    } else if (res == -1) {
        printf("\nError: Paid amount is less than book price ($%.2f).\n",
               selected_book->price);
        printf("Returning to main menu...\n");
    } else {
        printf("\nError: Sale transaction failed.\n");
    }
}

static void ui_view_history(const LibrarySystem *sys) {
    printf("\n--- Sales History ---\n");
    printf("1. View by Buyer Name\n");
    printf("2. View by Library Name\n");
    printf("Select option: ");
    int opt;
    scanf("%d", &opt);
    getchar();

    char query[MAX_STR];
    if (opt == 1) {
        printf("Enter Buyer Name: ");
        read_line(query, MAX_STR);
        display_sales_by_buyer(sys, query);
    } else if (opt == 2) {
        printf("Enter Library Name: ");
        read_line(query, MAX_STR);
        display_sales_by_library(sys, query);
    }
}

int main(void) {
    /* Automatically load database at startup */
    LibrarySystem *sys = load_system_from_file(DATA_FILE);
    if (!sys) {
        sys = create_system();
        setup_initial_branches(sys);
    }

    int choice = 0;
    while (1) {
        printf("\n========================================\n");
        printf("   CHAIN LIBRARY MANAGEMENT SYSTEM\n");
        printf("========================================\n");
        printf("1. Add Book (to Warehouse/Catalog)\n");
        printf("2. Search Books\n");
        printf("3. Delete Book\n");
        printf("4. View Warehouse Books\n");
        printf("5. Assign / Transfer Book to Library\n");
        printf("6. Sell Book\n");
        printf("7. View Sales History\n");
        printf("8. Save Data to File\n");
        printf("9. Exit\n");
        printf("Select option (1-9): ");
        if (scanf("%d", &choice) != 1) {
            break;
        }
        getchar();

        switch (choice) {
            case 1: ui_add_book(sys); break;
            case 2: ui_search_books(sys); break;
            case 3: ui_delete_book(sys); break;
            case 4: display_warehoused_books(sys); break;
            case 5: ui_assign_book(sys); break;
            case 6: ui_sell_book(sys); break;
            case 7: ui_view_history(sys); break;
            case 8:
                if (save_system_to_file(sys, DATA_FILE)) {
                    printf("Success: All data saved to %s\n", DATA_FILE);
                } else {
                    printf("Error: Could not save data.\n");
                }
                break;
            case 9:
                save_system_to_file(sys, DATA_FILE);
                free_system(sys);
                printf("Data saved. System exited successfully.\n");
                return 0;
            default:
                printf("Invalid choice. Please select 1-9.\n");
        }
    }

    free_system(sys);
    return 0;
}