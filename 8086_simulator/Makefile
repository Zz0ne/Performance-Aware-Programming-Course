# Directory where CMake will generate build files
BUILD_DIR := build
EXECUTABLE := sim8086

.PHONY: all setup build clean run

# Default target when you just run 'make'
all: build

# 1. Setup target: Creates build dir and configures CMake
setup:
	@cmake -B $(BUILD_DIR)

# 2. Build target: Compiles the binary (auto-runs setup if build folder is missing)
build:
	@if [ ! -d "$(BUILD_DIR)" ]; then $(MAKE) setup; fi
	@cmake --build $(BUILD_DIR)

# 3. Run target: Compiles and executes the program
run: build
	@./$(BUILD_DIR)/$(EXECUTABLE)

# 4. Clean target: Completely removes build directory and binaries
clean:
	@rm -rf $(BUILD_DIR)
