# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -Werror -O2

# Target executable name
TARGET = parse

# Source files
SRCS = parse.c

# Object files
OBJS = $(SRCS:.c=.o)

# Default target
all: $(TARGET)

# Build the target executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Run the program
run: $(TARGET)
	./$(TARGET)

# Clean up build files
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets (targets that are not files)
.PHONY: all clean run
