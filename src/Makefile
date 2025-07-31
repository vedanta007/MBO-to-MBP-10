# Makefile for Orderbook Reconstruction
CXX = g++
CXXFLAGS = -std=c++17 -O3 -Wall -Wextra -march=native -DNDEBUG
LDFLAGS = 

# Source files
SOURCES = main.cpp orderbook.cpp csv_parser.cpp
TEST_SOURCES = ../tests/test_orderbook/test_orderbook.cpp orderbook.cpp csv_parser.cpp
INTEGRATION_SOURCES = test_integration.cpp orderbook.cpp csv_parser.cpp
OBJECTS = $(SOURCES:.cpp=.o)
TEST_OBJECTS = ../build/test_orderbook.o orderbook.o csv_parser.o
INTEGRATION_OBJECTS = $(INTEGRATION_SOURCES:.cpp=.o)
TARGET = reconstruction_blockhouse
TEST_TARGET = test_orderbook
INTEGRATION_TARGET = test_integration

# Default target
all: $(TARGET)

# Build target
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Compile source files
%.o: %.cpp orderbook.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Update rule for compiling test object files
../build/%.o: ../tests/test_orderbook/%.cpp orderbook.h
	$(CXX) $(CXXFLAGS) -I../src -c $< -o $@

# Debug build
debug: CXXFLAGS = -std=c++17 -g -Wall -Wextra -DDEBUG
debug: $(TARGET)

# Performance build with additional optimizations
performance: CXXFLAGS = -std=c++17 -O3 -Wall -Wextra -march=native -DNDEBUG -flto -ffast-math -funroll-loops
performance: $(TARGET)

# Clean build files
clean:
	rm -f $(OBJECTS) $(TEST_OBJECTS) $(INTEGRATION_OBJECTS) $(TARGET) $(TEST_TARGET) $(INTEGRATION_TARGET)

# Build and run unit tests
test: $(TEST_TARGET)
	./$(TEST_TARGET)

# Build test executable
$(TEST_TARGET): $(TEST_OBJECTS)
	$(CXX) $(TEST_OBJECTS) -o $(TEST_TARGET) $(LDFLAGS)

# Build and run integration test
integration: $(INTEGRATION_TARGET)
	./$(INTEGRATION_TARGET)

# Build integration test executable
$(INTEGRATION_TARGET): $(INTEGRATION_OBJECTS)
	$(CXX) $(INTEGRATION_OBJECTS) -o $(INTEGRATION_TARGET) $(LDFLAGS)

# Run with sample data
run-sample: $(TARGET)
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
	@echo "  test        - Build and run unit tests"
	@echo "  integration - Build and run integration test"
	@echo "  run-sample  - Run with sample data"
	@echo "  help        - Show this help"

.PHONY: all debug performance clean test install-deps help