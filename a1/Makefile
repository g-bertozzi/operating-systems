CC = gcc
CFLAGS = -Wall -g

# Targets
all: pipe4 fetch-info

pipe4: pipe4.o
	$(CC) $(CFLAGS) -o pipe4 pipe4.o

fetch-info: fetch-info.o
	$(CC) $(CFLAGS) -o fetch-info fetch-info.o

pipe4.o: pipe4.c
	$(CC) $(CFLAGS) -c pipe4.c

fetch-info.o: fetch-info.c
	$(CC) $(CFLAGS) -c fetch-info.c

clean:
	rm -f *.o pipe4 fetch-info
