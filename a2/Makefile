# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99

# Executable name
TARGET = schedule

# Source files
SRCS = feedbackq.c queue.c

# Object files (generated from the source files)
OBJS = $(SRCS:.c=.o)

# Default target
all: $(TARGET)

# Rule to create the schedule executable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

# Rule to create the object files from source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target to remove object files and the executable
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets to prevent make from getting confused by files named 'clean' or 'all'
.PHONY: all clean
