# Executable name
TARGET = fishtank_control

# Source and header files
SRCS = fishtank_control.c leddy.c
HEADERS = leddy.h

# Compiler and flags
CC = clang
CFLAGS_COMMON = -Wall -Werror -lcurl
CFLAGS_DEV = $(CFLAGS_COMMON) -fsanitize=address -g
CFLAGS_PROD = $(CFLAGS_COMMON) -O2

# Output directory
OUT_DIR = build

# Default target: dev build
all: dev

# Dev build target
dev: $(OUT_DIR)/$(TARGET)

$(OUT_DIR)/$(TARGET): $(SRCS) $(HEADERS)
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS_DEV) $(SRCS) -o $(OUT_DIR)/$(TARGET)
	cp $(OUT_DIR)/$(TARGET) ~/toolkit/

# Prod build target
prod: clean
	@mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS_PROD) $(SRCS) -o $(OUT_DIR)/$(TARGET)
	cp $(OUT_DIR)/$(TARGET) ~/toolkit/

# Clean build artifacts
clean:
	rm -rf $(OUT_DIR)

# Mark these targets as not real files
.PHONY: all dev prod clean
