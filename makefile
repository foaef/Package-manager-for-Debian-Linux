# Compiler and Flags
CC = gcc
CFLAGS = -Wall -Wextra -I./include -g
LIBS = -lcurl

# Target binary name
TARGET = baciu
# Installation prefix (standard for local builds)
PREFIX = /usr/local

# Source files and Object files
SRCS = src/main.c src/downloader.c src/fs.c src/resolver.c src/Index_parser.c src/installer.c
OBJS = $(SRCS:.c=.o)

# Default rule: Build the binary
all: $(TARGET)

# Link the objects into the final executable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LIBS)

# Compile .c files to .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# --- NEW: Installation Rules ---

install: $(TARGET)
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 $(TARGET) $(DESTDIR)$(PREFIX)/bin

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(TARGET)

# --- End of Installation Rules ---

# Clean up the build files
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets (not actual files)
.PHONY: all clean install uninstall