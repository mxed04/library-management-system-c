CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -std=c99
SRC = src/main.c src/library.c src/book_ops.c src/sales_ops.c src/storage.c
TARGET = libman

all:
	mkdir -p data
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)