#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "models.h"
#include "library.h"
#include "book_ops.h"
#include "sales_ops.h"
#include "storage.h"

#define DATA_FILE "data/library_data.dat"

/* Robust string input to prevent buffer overflows */
static void read_line(char *buffer, int size) {
    if (fgets(buffer, size, stdin)) {
        buffer[strcspn(buffer, "\n")] = '\0';
    }
}

/* Robust integer input to prevent infinite loops and crashes */
static int get_int(void) {
    char buffer[100];
    read_line(buffer, sizeof(buffer));
    int val;
    if (sscanf(buffer, "%d", &val) == 1) {
        return val;
    }
    return -1; /* Returns -1 on invalid input (e.g., typing letters) */
}

/* Robust double input for prices and payments */
static double get_double(void) {
    char buffer[100];
    read_line(buffer, sizeof(buffer));
    double val;
    if (sscanf(buffer, "%lf", &val) == 1) {
        return val;
    }
    return -1.0;
}

/* Pre-loads mock data into the system if the database is empty */
static void setup_initial_branches(LibrarySystem *sys) {
    if (sys->library_count == 0) {
        /* Add Libraries and Shelves */
        add_library(sys, 1, "Central Library", "Main Ave #10");
        add_shelf(&sys->libraries[0], 101, "Fiction", 5);
        add_shelf(&sys->libraries[0], 102, "Science", 8);

        add_library(sys, 2, "North Branch", "North Blvd #45");
        add_shelf(&sys->libraries[1], 201, "History", 6);

        /* Add Pre-defined Books to Warehouse */
        Book b1 = {"1984", "George Orwell", "Secker", "Fiction",
                   "978-045152", 15.99};
        Book b2 = {"Cosmos", "Carl Sagan", "Random", "Science",
                   "978-034533", 18.50};
        Book b3 = {"Sapiens", "Yuval Harari", "Harvill", "History",
                   "978-006231", 22.00};
        
        register_book(sys, b1);
        register_book(sys, b2);
        register_book(sys, b3);

        /* Assign some books to libraries automatically */
        assign_book_to_library(sys, "978-045152", 1);
        assign_book_to_library(sys, "978-034533", 1);
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
    b.price = get_double();
    if (b.price < 0) {
        printf("Error: Invalid price entered. Returning to menu.\n");
        return;
    }

    if (register_book(sys, b)) {
        printf("Success: Book added to master database (Warehouse).\n");
    } else {
        printf("Error: Book with ISBN '%s' already exists.\n", b.isbn);
    }
}

static void ui_search_books(const LibrarySystem *sys) {
    SearchFilter f;
    memset(&f, 0, sizeof(SearchFilter));
    printf("\n--- Search Book (Leave blank to skip filter) ---\n");
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
    f.min_price = get_double();
    printf("Filter Max Price (-1 for none): ");
    f.max_price = get_double();

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
    printf("\n--- Assign Book to Library ---\n");
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
    int eligible = 0;
    for (int i = 0; i < sys->library_count; i++) {
        if (has_eligible_shelf(&sys->libraries[i], target->genre)) {
            printf("[%d] %s\n", sys->libraries[i].id, sys->libraries[i].name);
            eligible++;
        }
    }

    if (eligible == 0) {
        printf("Error: No branches have free space for this genre.\n");
        return;
    }

    printf("Select Library ID: ");
    int lib_id = get_int();
    
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
    printf("Enter Branch ID: ");
    int lib_id = get_int();

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
    int count = 0;
    for (int j = 0; j < lib->shelf_count; j++) {
        for (int k = 0; k < lib->shelves[j].current_count; k++) {
            printf("- %s (ISBN: %s)\n", lib->shelves[j].books[k].title,
                   lib->shelves[j].books[k].isbn);
            count++;
        }
    }

    if (count == 0) {
        printf("This branch currently has no books.\n");
        return;
    }

    char isbn[MAX_ISBN];
    printf("\nEnter ISBN of book to buy: ");
    read_line(isbn, MAX_ISBN);

    char buyer[MAX_STR];
    printf("Enter Buyer Name: ");
    read_line(buyer, MAX_STR);
    
    printf("Enter Paid Amount: $");
    double paid = get_double();
    if (paid < 0) {
        printf("Error: Invalid payment amount.\n");
        return;
    }

    double change = 0.0;
    int res = process_book_sale(sys, lib_id, isbn, buyer, paid, &change);
    
    if (res == 1) {
        printf("\nSale completed! Change to return: $%.2f\n", change);
    } else if (res == -1) {
        printf("\nError: Paid amount is less than book price.\n");
        printf("Returning to main menu...\n");
    } else {
        printf("\nError: Transaction failed. Book might not exist.\n");
    }
}

static void ui_view_history(const LibrarySystem *sys) {
    printf("\n--- View Sold Books History ---\n");
    printf("1. View by Buyer Name\n");
    printf("2. View by Library Name\n");
    printf("Select option: ");
    int opt = get_int();

    char query[MAX_STR];
    if (opt == 1) {
        printf("Enter Buyer Name: ");
        read_line(query, MAX_STR);
        display_sales_by_buyer(sys, query);
    } else if (opt == 2) {
        printf("Enter Library Name: ");
        read_line(query, MAX_STR);
        display_sales_by_library(sys, query);
    } else {
        printf("Error: Invalid option.\n");
    }
}

int main(void) {
    /* Auto-load database; if empty, preload default data */
    LibrarySystem *sys = load_system_from_file(DATA_FILE);
    if (!sys) {
        sys = create_system();
        setup_initial_branches(sys);
    }

    int choice = 0;
    while (1) {
        printf("\nMain Menu\n\n");
        printf("1. Add Book\n");
        printf("2. Search Book\n");
        printf("3. Delete Book\n");
        printf("4. Display Stocked Books\n");
        printf("5. Assign Book to Library\n");
        printf("6. Sell Book\n");
        printf("7. View Sold Books History\n");
        printf("8. Save Current Data\n");
        printf("9. Exit\n\n");
        printf("Enter your choice (1-9): ");
        
        choice = get_int();

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
                return 0;
            default:
                printf("Error: Invalid choice. Please enter a number 1-9.\n");
        }
    }

    free_system(sys);
    return 0;
}