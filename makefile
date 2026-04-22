# Compiler and Flags
CC = gcc
CFLAGS = -Wall -Wextra -I./include -g
LIBS = -lcurl -lcjson

# Target binary name
TARGET = baciu

# Source files and Object files
# This looks for all .c files in the src directory
SRCS = src/main.c src/repo.c src/downloader.c src/fs.c src/resolver.c src/Index_parser.c src/installer.c
OBJS = $(SRCS:.c=.o)

# Default rule: Build the binary
all: $(TARGET)

# Link the objects into the final executable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LIBS)

# Compile .c files to .o files
# The -c flag tells gcc not to link yet
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up the build files
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets (not actual files)
.PHONY: all clean