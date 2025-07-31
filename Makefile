# Makefile for Orderbook Reconstruction
CXX = g++
CXXFLAGS = -std=c++17 -O3 -Wall -Wextra -march=native -DNDEBUG
LDFLAGS = 

# Source files
SOURCES = main.cpp orderbook.cpp csv_parser.cpp
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = reconstruction_blockhouse

# Default target
all: $(TARGET)

# Build target
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Compile source files
%.o: %.cpp orderbook.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Debug build
debug: CXXFLAGS = -std=c++17 -g -Wall -Wextra -DDEBUG
debug: $(TARGET)

# Performance build with additional optimizations
performance: CXXFLAGS = -std=c++17 -O3 -Wall -Wextra -march=native -DNDEBUG -flto -ffast-math -funroll-loops
performance: $(TARGET)

# Clean build files
clean:
	rm -f $(OBJECTS) $(TARGET)

# Run with sample data
test: $(TARGET)
	./$(TARGET) mbo.csv

# Install dependencies (if needed)
install-deps:
	@echo "No external dependencies required"

# Help
help:
	@echo "Available targets:"
	@echo "  all         - Build the reconstruction tool (default)"
	@echo "  debug       - Build with debug flags"
	@echo "  performance - Build with maximum optimization"
	@echo "  clean       - Remove build files"
	@echo "  test        - Run with sample data"
	@echo "  help        - Show this help"

.PHONY: all debug performance clean test install-deps help