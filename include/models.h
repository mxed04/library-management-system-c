#ifndef MODELS_H
#define MODELS_H

#define MAX_STR 100
#define MAX_ISBN 20

/* Book structure holding core bibliographic data */
typedef struct {
    char title[MAX_STR];
    char author[MAX_STR];
    char publisher[MAX_STR];
    char genre[MAX_STR];
    char isbn[MAX_ISBN];
    double price;
} Book;

/* Shelf structure with genre and capacity limit */
typedef struct {
    int id;
    char genre[MAX_STR];
    int capacity;
    int current_count;
    Book *books; /* Dynamic array of books */
} Shelf;

/* Library structure representing a branch */
typedef struct {
    int id;
    char name[MAX_STR];
    char address[MAX_STR * 2];
    int shelf_count;
    Shelf *shelves; /* Dynamic array of shelves */
} Library;

/* Book location info */
typedef struct {
    int is_placed;
    char library_name[MAX_STR];
    int shelf_id;
} BookLocation;

/* Filter options for multi-criteria search */
typedef struct {
    char title[MAX_STR];
    char author[MAX_STR];
    char isbn[MAX_ISBN];
    char genre[MAX_STR];
    char publisher[MAX_STR];
    double min_price;
    double max_price;
} SearchFilter;

/* Record for completed book sales */
typedef struct {
    char buyer_name[MAX_STR];
    char book_title[MAX_STR];
    char book_isbn[MAX_ISBN];
    char library_name[MAX_STR];
    double price;
    double paid_amount;
    double change;
} SaleRecord;

/* System manager holding libraries, master books, and sales history */
typedef struct {
    int library_count;
    Library *libraries;
    int book_count;
    Book *books;
    int sale_count;
    SaleRecord *sales; /* Dynamic array of sales records */
} LibrarySystem;

#endif /* MODELS_H */