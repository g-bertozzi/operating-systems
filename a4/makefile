# Makefile for Virtual Memory Simulator (single-file version)

CC = gcc
CFLAGS = -Wall -g
TARGET = virtmem
SRC = virtmem.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
