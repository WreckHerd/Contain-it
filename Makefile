# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude

# Directories
SRC_DIR = src
OBJ_DIR = build
INC_DIR = include

# Files
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SOURCES))
EXECUTABLE = contain-it

# Default target
all: $(EXECUTABLE)

# Link the object files into the final executable
$(EXECUTABLE): $(OBJECTS)
	@echo "Linking $@"
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile each .cpp file into a .o file
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	@echo "Compiling $<"
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	@echo "Cleaning up..."
	rm -rf $(OBJ_DIR) $(EXECUTABLE)

.PHONY: all clean

# Define the rootfs directory
ROOTFS_DIR = alpine-rootfs

# The automation target
setup-rootfs:
	@echo "[INFO] Downloading and extracting Alpine mini-rootfs..."
	@mkdir -p $(ROOTFS_DIR)
	@wget -qO- https://dl-cdn.alpinelinux.org/alpine/v3.18/releases/x86_64/alpine-minirootfs-3.18.4-x86_64.tar.gz | tar -xz -C $(ROOTFS_DIR)
	@echo "[SUCCESS] Root filesystem ready at ./$(ROOTFS_DIR)"