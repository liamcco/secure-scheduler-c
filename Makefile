# Compiler and Flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -O2  # Added -O2 for optimization

# Source and Object Files
SRC = $(wildcard *.c)
OBJ_DIR = build
OBJ = $(patsubst %.c, $(OBJ_DIR)/%.o, $(SRC))

# Output Executable
TARGET = program

# Default Rule: Build Executable
all: $(TARGET)

# Ensure build directory exists
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Link Object Files into Executable
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

# Compile .c to .o with header dependency tracking
$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

# Include header dependencies
-include $(OBJ:.o=.d)

# Clean Build Files
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

# Debug Build (no optimization)
debug: CFLAGS += -g -O0  # Disable optimizations for easier debugging
debug: clean all

# Run the Program
run: $(TARGET)
	./$(TARGET)

# Phony Targets
.PHONY: all clean debug run
