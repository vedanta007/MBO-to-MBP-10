#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include <unordered_map>
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>

// Structure to represent a single MBO record
struct MBORecord {
    std::string ts_recv;
    std::string ts_event;
    int rtype;
    int publisher_id;
    int instrument_id;
    char action;
    char side;
    double price;
    int size;
    int channel_id;
    long order_id;
    int flags;
    int ts_in_delta;
    long sequence;
    std::string symbol;
};

// Structure to represent price level information
struct PriceLevel {
    double price;
    int total_size;
    int order_count;

    PriceLevel() : price(0.0), total_size(0), order_count(0) {}
    PriceLevel(double p, int s, int c) : price(p), total_size(s), order_count(c) {}
};

// Structure to represent an order in the book
struct Order {
    long order_id;
    double price;
    int size;
    char side;

    Order() : order_id(0), price(0.0), size(0), side('N') {}
    Order(long id, double p, int s, char sd) : order_id(id), price(p), size(s), side(sd) {}
};

// High-performance orderbook class
class OrderBook {
private:
    // Order tracking - Robin Hood hash map equivalent (unordered_map)
    std::unordered_map<long, Order> orders;

    // Price level tracking - using multimap for sorted price levels
    std::map<double, PriceLevel> bids;  // Sorted descending (highest first)
    std::map<double, PriceLevel> asks;  // Sorted ascending (lowest first)

    // Sequence tracking for T->F->C patterns
    std::vector<MBORecord> pending_sequence;

public:
    OrderBook();
    ~OrderBook();

    // Core functionality
    void processRecord(const MBORecord& record);
    void addOrder(const MBORecord& record);
    void cancelOrder(const MBORecord& record);
    void handleTradeSequence(const std::vector<MBORecord>& sequence);

    // Output generation
    std::string generateMBPOutput(const MBORecord& record, int row_index);
    std::vector<PriceLevel> getBidLevels(int max_levels = 10) const;
    std::vector<PriceLevel> getAskLevels(int max_levels = 10) const;

    // Utility functions
    void clear();
    void printBook() const;

private:
    void updatePriceLevel(std::map<double, PriceLevel>& levels, double price, int size_delta, int count_delta);
    void removePriceLevel(std::map<double, PriceLevel>& levels, double price);
    bool isTradeSequenceComplete() const;
};

// CSV parsing utilities
class CSVParser {
public:
    static std::vector<MBORecord> parseFile(const std::string& filename);
    static MBORecord parseLine(const std::string& line);
    static std::vector<std::string> splitCSV(const std::string& line);
};

// Performance utilities
class PerformanceTimer {
private:
    std::chrono::high_resolution_clock::time_point start_time;
    std::string operation_name;

public:
    PerformanceTimer(const std::string& name);
    ~PerformanceTimer();
};

#endif // ORDERBOOK_H