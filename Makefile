# Target executable
TARGET = fishtank_control

# Source files
SRCS = fishtank_control.c tasmota.c

# Header files
HEADERS = tasmota.h

# Default target
all: $(TARGET)

# Build the target
$(TARGET): $(SRCS) $(HEADERS)
	gcc -lcurl $(SRCS) -o $(TARGET)
	cp $(TARGET) ~/toolkit/

# Clean target to remove the executable
clean:
	rm -f $(TARGET)

# Phony targets
.PHONY: all clean
