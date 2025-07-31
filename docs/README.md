# Order Book Reconstruction - MBO to MBP-10 Converter

A high-performance C++ implementation for reconstructing Market By Price (MBP-10) data from Market By Order (MBO) data.

## 🚀 Overview

This project processes tick-by-tick order events and maintains a real-time order book, outputting snapshots in the MBP-10 format. It handles special requirements including T→F→C sequence processing and proper side determination for trades.

## 📁 Project Structure

```
├── src/
│   ├── csv_parser.cpp            # CSV parsing implementation
│   ├── orderbook.cpp             # Order book logic
│   ├── main.cpp                  # Entry point
│   ├── Makefile                  # Build configuration
│   ├── mbo.csv                   # Input data (sample)
│   └── mbp.csv                   # Expected output (reference)
├── tests/
│   ├── test_integration.cpp      # Integration tests
│   ├── test_orderbook.cpp        # Unit tests for order book
├── README.md                      # This file
└── docs/
    ├── TESTING.md                # Testing documentation
    └── quant_dev_trial.pdf       # Design document
```

## 🛠 Quick Start

### Prerequisites
- C++17 compatible compiler (g++, clang++)
- Make (optional, for using Makefile)

### Compilation
```bash
# Using g++ directly
g++ -std=c++17 -O2 -Wall -Wextra -o reconstruction_blockhouse src/main.cpp src/csv_parser.cpp src/orderbook.cpp

# Or using make (if available)
make
```

### Usage
```bash
./reconstruction_blockhouse data/mbo.csv
```

This will generate `output/output_mbp.csv` with the reconstructed order book data.

## 🔧 Key Features

### ✅ Complete MBO Processing
- Parses CSV data with correct column mapping
- Handles all action types: Add (A), Cancel (C), Trade (T), Fill (F) 
- Skips Reset (R) actions as specified
- Ignores trades with side 'N'

### ✅ High-Performance Order Book
- **O(1)** order lookups using `std::unordered_map`
- **O(log n)** price level operations using `std::map`
- Automatic cleanup of empty price levels
- Minimal memory allocations

### ✅ MBP-10 Output Generation
- Matches exact format of reference output
- All 60 columns (10 levels × 3 fields × 2 sides)
- Proper timestamp and metadata preservation
- Real-time snapshot generation

### ✅ Special Requirements
- **T→F→C Sequence Handling**: Combines trade sequences into single trade actions
- **Side Logic**: Properly determines which side is affected by trades
- **Format Compliance**: Exact match with expected MBP format

## 📊 Performance

- **Time Complexity**: O(m log n) where m = orders, n = price levels
- **Space Complexity**: O(m + n) for active orders and price levels  
- **Memory Usage**: Linear scaling with order book depth
- **Processing Speed**: Single-pass algorithm with optimized data structures

## 🧪 Testing

### Basic Functionality Test
```bash
# Compile test
g++ -std=c++17 -O2 -o test_orderbook tests/test_orderbook.cpp src/orderbook.cpp

# Run test
./test_orderbook
```

### Manual Verification
Compare generated `output/output_mbp.csv` with the reference file in `data/mbp.csv`.

## 📋 Implementation Details

### Data Structures
- `std::unordered_map<string, Order>` for order storage
- `std::map<double, PriceLevel, std::greater<double>>` for bids (descending)
- `std::map<double, PriceLevel>` for asks (ascending)

### Order Processing Flow
1. Parse MBO record from CSV
2. Skip reset actions (R)
3. Process action: Add (A), Cancel (C), Trade (T/F)
4. Update order book state
5. Generate MBP-10 snapshot
6. Write to output file

### Special Handling
- **Trades**: Show impact on the side being hit (opposite of aggressor)
- **Empty Levels**: Automatic removal to prevent memory bloat
- **Format**: Exact compliance with 60-column MBP format

## 🔍 Algorithm Optimization

### Memory Management
- Efficient object reuse
- Automatic cleanup of empty price levels
- Minimal string operations during hot path

### I/O Optimization  
- Buffered file output with explicit flushing
- Single-pass input processing
- Pre-allocated string formatting

### Data Access Patterns
- Cache-friendly data structures
- Minimized pointer dereferencing
- Optimized for sequential processing

## 📖 Documentation

See `docs/TESTING.md` for comprehensive documentation including:
- Detailed optimization strategies
- Performance characteristics
- Limitations and trade-offs
- Testing recommendations
- Future enhancement ideas

## 🤝 Contributing

This is a specialized financial data processing implementation. For questions or improvements:

1. Review the detailed documentation in `docs/TESTING.md`
2. Run the test suite to verify functionality
3. Benchmark performance with your datasets

## 📄 License

Private implementation for order book reconstruction evaluation.

---

**Author**: Vedanta  
**Date**: July 2025  
**Version**: 1.0