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

/* System-wide manager for all library branches */
typedef struct {
    int library_count;
    Library *libraries; /* Dynamically allocated array of libraries */
} LibrarySystem;

#endif /* MODELS_H */