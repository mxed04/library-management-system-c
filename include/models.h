#ifndef MODELS_H
#define MODELS_H

#define MAX_STR 100
#define MAX_ISBN 20

/* ANSI Color Codes for UI */
#define C_RED     "\x1b[31m"
#define C_GREEN   "\x1b[32m"
#define C_YELLOW  "\x1b[33m"
#define C_CYAN    "\x1b[36m"
#define C_RESET   "\x1b[0m"

/* User Roles for Access Control */
typedef enum {
    ROLE_ADMIN,
    ROLE_LIBRARIAN,
    ROLE_SELLER,
    ROLE_USER
} UserRole;

/* System User structure */
typedef struct {
    char username[MAX_STR];
    char password[MAX_STR];
    UserRole role;
} User;

/* Enhanced Book structure with ratings */
typedef struct {
    char title[MAX_STR];
    char author[MAX_STR];
    char publisher[MAX_STR];
    char genre[MAX_STR];
    char isbn[MAX_ISBN];
    double price;
    double total_rating;
    int rating_count;
} Book;

/* Shelf structure */
typedef struct {
    int id;
    char genre[MAX_STR];
    int capacity;
    int current_count;
    Book *books;
} Shelf;

/* Enhanced Library structure with ratings */
typedef struct {
    int id;
    char name[MAX_STR];
    char address[MAX_STR * 2];
    int shelf_count;
    Shelf *shelves;
    double total_rating;
    int rating_count;
} Library;

/* Book location info */
typedef struct {
    int is_placed;
    char library_name[MAX_STR];
    int shelf_id;
} BookLocation;

/* Advanced Search Filter */
typedef struct {
    char query[MAX_STR]; /* Used for fuzzy search across fields */
    double min_price;
    double max_price;
} SearchFilter;

/* Sale Record */
typedef struct {
    char buyer_name[MAX_STR];
    char book_title[MAX_STR];
    char book_isbn[MAX_ISBN];
    char library_name[MAX_STR];
    double price;
    double paid_amount;
    double change;
} SaleRecord;

/* Master System Configuration */
typedef struct {
    int library_count;
    Library *libraries;
    int book_count;
    Book *books;
    int sale_count;
    SaleRecord *sales;
    int user_count;
    User *users; /* Dynamic array of authenticated users */
} LibrarySystem;

#endif /* MODELS_H */