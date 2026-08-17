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

/* Shelf structure with a specific genre and capacity limit */
typedef struct {
    int id;
    char genre[MAX_STR];
    int capacity;
    int current_count;
    Book *books; /* Dynamically allocated array of books */
} Shelf;

/* Library structure representing a branch in the chain */
typedef struct {
    int id;
    char name[MAX_STR];
    char address[MAX_STR * 2];
    int shelf_count;
    Shelf *shelves; /* Dynamically allocated array of shelves */
} Library;

/* Book location details: placed in a shelf or in warehouse */
typedef struct {
    int is_placed;              /* 1: placed in a shelf, 0: in warehouse */
    char library_name[MAX_STR]; /* Name of branch */
    int shelf_id;               /* Target shelf ID */
} BookLocation;

/* Filter options for multi-criteria search */
typedef struct {
    char title[MAX_STR];
    char author[MAX_STR];
    char isbn[MAX_ISBN];
    char genre[MAX_STR];
    char publisher[MAX_STR];
    double min_price;           /* -1 indicates no lower price filter */
    double max_price;           /* -1 indicates no upper price filter */
} SearchFilter;

/* System-wide manager holding libraries and all registered books */
typedef struct {
    int library_count;
    Library *libraries;         /* Dynamically allocated library list */
    int book_count;
    Book *books;                /* Master catalog / database of books */
} LibrarySystem;

#endif /* MODELS_H */