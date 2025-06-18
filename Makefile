# Compiler and Flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -O0 -Iinclude

# Directories
SRC_DIR = src
INC_DIR = include
OBJ_DIR = build

# Find all .c files recursively
SRC = $(shell find $(SRC_DIR) -name '*.c')

# Map each .c file in src/ to a .o file in build/, preserving directory structure
OBJ = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))

# Output Executable
TARGET = program

# Default Rule: Build Executable
all: $(TARGET)

# Create build/ directory structure as needed before compiling
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

# Link object files into final executable
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

# Include header dependencies
-include $(OBJ:.o=.d)

# Clean rule
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

# Debug build
debug: CFLAGS += -g -O0
debug: clean all

# Run the program
run: $(TARGET)
	./$(TARGET)

# Phony targets
.PHONY: all clean debug run
