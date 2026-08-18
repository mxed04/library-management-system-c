#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "models.h"
#include "library.h"
#include "book_ops.h"
#include "sales_ops.h"
#include "storage.h"

#define DATA_FILE "data/library_data.dat"

static void read_line(char *buffer, int size) {
    if (fgets(buffer, size, stdin)) {
        buffer[strcspn(buffer, "\n")] = '\0';
    }
}

static int get_int(void) {
    char buffer[100];
    read_line(buffer, sizeof(buffer));
    int val;
    if (sscanf(buffer, "%d", &val) == 1) return val;
    return -1;
}

static double get_double(void) {
    char buffer[100];
    read_line(buffer, sizeof(buffer));
    double val;
    if (sscanf(buffer, "%lf", &val) == 1) return val;
    return -1.0;
}

static int has_permission(UserRole current_role, UserRole required_role) {
    return (current_role <= required_role); /* 0=Admin is highest */
}

/* Authentication Loop */
static User* authenticate_user(LibrarySystem *sys) {
    if (sys->user_count == 0) {
        sys->users = (User*)malloc(sizeof(User));
        strncpy(sys->users[0].username, "admin", MAX_STR);
        strncpy(sys->users[0].password, "1234", MAX_STR);
        sys->users[0].role = ROLE_ADMIN;
        sys->user_count = 1;
        printf(C_GREEN "Init: Default Admin created (admin/1234)\n" C_RESET);
    }

    char user[MAX_STR], pass[MAX_STR];
    while (1) {
        printf(C_CYAN "\n========================================\n");
        printf("        SYSTEM LOGIN REQUIRED\n");
        printf("========================================\n" C_RESET);
        printf("Username (or 'exit'): ");
        read_line(user, MAX_STR);
        
        if (strcmp(user, "exit") == 0) return NULL;

        printf("Password: ");
        read_line(pass, MAX_STR);

        for (int i = 0; i < sys->user_count; i++) {
            if (strcmp(sys->users[i].username, user) == 0 &&
                strcmp(sys->users[i].password, pass) == 0) {
                printf(C_GREEN "\nLogin Successful! Welcome, %s.\n" C_RESET,
                       user);
                return &sys->users[i];
            }
        }
        printf(C_RED "Invalid credentials. Please try again.\n" C_RESET);
    }
}

static void setup_initial_branches(LibrarySystem *sys) {
    if (sys->library_count == 0) {
        add_library(sys, 1, "Central Library", "Main Ave #10");
        add_shelf(&sys->libraries[0], 101, "Fiction", 5);
        add_shelf(&sys->libraries[0], 102, "Science", 8);

        add_library(sys, 2, "North Branch", "North Blvd #45");
        add_shelf(&sys->libraries[1], 201, "History", 6);
    }
}

static void ui_add_book(LibrarySystem *sys) {
    Book b;
    printf("\n--- Add New Book ---\n");
    printf("Title: "); read_line(b.title, MAX_STR);
    printf("Author: "); read_line(b.author, MAX_STR);
    printf("Publisher: "); read_line(b.publisher, MAX_STR);
    printf("Genre: "); read_line(b.genre, MAX_STR);
    printf("ISBN: "); read_line(b.isbn, MAX_ISBN);
    
    printf("Price: ");
    b.price = get_double();
    if (b.price < 0) {
        printf(C_RED "Error: Invalid price.\n" C_RESET);
        return;
    }

    if (register_book(sys, b)) {
        printf(C_GREEN "Success: Book added to Warehouse.\n" C_RESET);
    } else {
        printf(C_RED "Error: Book ISBN already exists.\n" C_RESET);
    }
}

static void ui_search_books(const LibrarySystem *sys) {
    SearchFilter f;
    memset(&f, 0, sizeof(SearchFilter));
    printf(C_CYAN "\n--- Fuzzy Search Books ---\n" C_RESET);
    printf("Search Query (Title/Author/ISBN) or leave blank: ");
    read_line(f.query, MAX_STR);
    
    printf("Filter Min Price (-1 for none): ");
    f.min_price = get_double();
    printf("Filter Max Price (-1 for none): ");
    f.max_price = get_double();

    search_and_display_books(sys, &f);
}

static void ui_delete_book(LibrarySystem *sys) {
    char isbn[MAX_ISBN];
    printf("\n--- Delete Book ---\n");
    printf("Enter ISBN: ");
    read_line(isbn, MAX_ISBN);

    if (delete_book(sys, isbn)) {
        printf(C_GREEN "Success: Book completely deleted.\n" C_RESET);
    } else {
        printf(C_RED "Error: Book not found.\n" C_RESET);
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
        printf(C_RED "Error: Book not found.\n" C_RESET);
        return;
    }

    int eligible = 0;
    for (int i = 0; i < sys->library_count; i++) {
        if (has_eligible_shelf(&sys->libraries[i], target->genre)) {
            printf("[%d] %s\n", sys->libraries[i].id, sys->libraries[i].name);
            eligible++;
        }
    }

    if (eligible == 0) {
        printf(C_RED "No branches have free space for this genre.\n" C_RESET);
        return;
    }

    printf("Select Library ID: ");
    int lib_id = get_int();
    
    if (assign_book_to_library(sys, isbn, lib_id)) {
        printf(C_GREEN "Success: Book placed via smart priority.\n" C_RESET);
    } else {
        printf(C_RED "Error: Assignment failed.\n" C_RESET);
    }
}

static void ui_sell_book(LibrarySystem *sys) {
    printf("\n--- Sell Book ---\n");
    for (int i = 0; i < sys->library_count; i++) {
        printf("[%d] %s\n", sys->libraries[i].id, sys->libraries[i].name);
    }
    printf("Enter Branch ID: ");
    int lib_id = get_int();

    Library *lib = NULL;
    for (int i = 0; i < sys->library_count; i++) {
        if (sys->libraries[i].id == lib_id) {
            lib = &sys->libraries[i]; break;
        }
    }
    if (!lib) return;

    char isbn[MAX_ISBN], buyer[MAX_STR];
    printf("Enter ISBN of book to buy: ");
    read_line(isbn, MAX_ISBN);
    printf("Enter Buyer Name: ");
    read_line(buyer, MAX_STR);
    printf("Enter Paid Amount: $");
    double paid = get_double();

    double change = 0.0;
    int res = process_book_sale(sys, lib_id, isbn, buyer, paid, &change);
    
    if (res == 1) {
        printf(C_GREEN "\nSale completed! Change: $%.2f\n" C_RESET, change);
    } else if (res == -1) {
        printf(C_RED "\nError: Insufficient payment.\n" C_RESET);
    } else {
        printf(C_RED "\nError: Transaction failed.\n" C_RESET);
    }
}

/* SENIOR DEVELOPER BONUS: Analytics Dashboard & CSV Export */
static void export_sales_csv(const LibrarySystem *sys) {
    FILE *fp = fopen("data/export_sales.csv", "w");
    if (!fp) {
        printf(C_RED "Error creating CSV file.\n" C_RESET);
        return;
    }
    fprintf(fp, "Buyer,Book Title,ISBN,Library,Price,Paid,Change\n");
    for(int i = 0; i < sys->sale_count; i++) {
        SaleRecord s = sys->sales[i];
        fprintf(fp, "%s,%s,%s,%s,%.2f,%.2f,%.2f\n",
            s.buyer_name, s.book_title, s.book_isbn, s.library_name,
            s.price, s.paid_amount, s.change);
    }
    fclose(fp);
    printf(C_GREEN "Success: Exported to 'data/export_sales.csv'\n" C_RESET);
}

static void ui_admin_dashboard(const LibrarySystem *sys) {
    printf(C_CYAN "\n=== ADMIN ANALYTICS DASHBOARD ===\n" C_RESET);
    printf("Total Branches   : %d\n", sys->library_count);
    printf("Master Catalog   : %d books\n", sys->book_count);
    printf("Total Sales      : %d transactions\n\n", sys->sale_count);

    printf(C_YELLOW "--- Branch Capacity (ASCII Chart) ---\n" C_RESET);
    for(int i = 0; i < sys->library_count; i++) {
        int total_cap = 0, current = 0;
        for(int j = 0; j < sys->libraries[i].shelf_count; j++) {
            total_cap += sys->libraries[i].shelves[j].capacity;
            current += sys->libraries[i].shelves[j].current_count;
        }
        printf("%-15s | ", sys->libraries[i].name);
        int bars = total_cap > 0 ? (current * 20 / total_cap) : 0;
        for(int b = 0; b < bars; b++) printf("█");
        for(int b = bars; b < 20; b++) printf("-");
        printf(" %d/%d\n", current, total_cap);
        if(total_cap > 0 && (float)current/total_cap > 0.85) {
            printf(C_RED "   [!] WARNING: Branch capacity critical!\n" C_RESET);
        }
    }
    
    printf(C_YELLOW "\n[1] Export Sales to CSV | [0] Go Back\n" C_RESET);
    printf("Select: ");
    if (get_int() == 1) export_sales_csv(sys);
}

int main(void) {
    LibrarySystem *sys = load_system_from_file(DATA_FILE);
    if (!sys) {
        sys = create_system();
        setup_initial_branches(sys);
    }

    User *current_user = authenticate_user(sys);
    if (!current_user) {
        free_system(sys);
        return 0;
    }

    while (1) {
        printf(C_CYAN "\nMain Menu - Logged in as: %s\n\n" C_RESET,
               current_user->username);
        
        if (has_permission(current_user->role, ROLE_LIBRARIAN)) {
            printf("1. Add Book\n3. Delete Book\n5. Assign Book to Library\n");
        }
        
        printf("2. Search Book (Fuzzy & Paginated)\n4. Display Stocked Books\n");
        
        if (has_permission(current_user->role, ROLE_SELLER)) {
            printf("6. Sell Book\n");
        }
        
        if (has_permission(current_user->role, ROLE_ADMIN)) {
            printf("7. View Sold Books History\n");
            printf(C_YELLOW "10. Admin Analytics Dashboard & CSV Export\n" C_RESET);
        }
        
        printf("8. Save Current Data\n9. Exit\n\n");
        printf("Enter your choice: ");
        int choice = get_int();

        switch (choice) {
            case 1:
                if (has_permission(current_user->role, ROLE_LIBRARIAN))
                    ui_add_book(sys); else printf("Access Denied.\n"); break;
            case 2: ui_search_books(sys); break;
            case 3:
                if (has_permission(current_user->role, ROLE_LIBRARIAN))
                    ui_delete_book(sys); else printf("Access Denied.\n"); break;
            case 4: display_warehoused_books(sys); break;
            case 5:
                if (has_permission(current_user->role, ROLE_LIBRARIAN))
                    ui_assign_book(sys); else printf("Access Denied.\n"); break;
            case 6:
                if (has_permission(current_user->role, ROLE_SELLER))
                    ui_sell_book(sys); else printf("Access Denied.\n"); break;
            case 7:
                if (has_permission(current_user->role, ROLE_ADMIN)) {
                    printf("Filter by Library Name: ");
                    char q[MAX_STR]; read_line(q, MAX_STR);
                    display_sales_by_library(sys, q);
                } else printf("Access Denied.\n"); break;
            case 10:
                if (has_permission(current_user->role, ROLE_ADMIN))
                    ui_admin_dashboard(sys); else printf("Access Denied.\n"); break;
            case 8:
                if (save_system_to_file(sys, DATA_FILE))
                    printf(C_GREEN "Data saved.\n" C_RESET);
                break;
            case 9:
                save_system_to_file(sys, DATA_FILE);
                free_system(sys);
                return 0;
            default:
                printf(C_RED "Invalid choice.\n" C_RESET);
        }
    }
    return 0;
}