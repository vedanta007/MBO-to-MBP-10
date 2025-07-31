#include "orderbook.h"
#include <cassert>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

// Test utilities
class TestFramework {
private:
    int tests_run = 0;
    int tests_passed = 0;
    int tests_failed = 0;

public:
    void assert_true(bool condition, const std::string& test_name) {
        tests_run++;
        if (condition) {
            tests_passed++;
            std::cout << "✅ PASS: " << test_name << std::endl;
        } else {
            tests_failed++;
            std::cout << "❌ FAIL: " << test_name << std::endl;
        }
    }

    void assert_equal(const std::string& actual, const std::string& expected, const std::string& test_name) {
        tests_run++;
        if (actual == expected) {
            tests_passed++;
            std::cout << "✅ PASS: " << test_name << std::endl;
        } else {
            tests_failed++;
            std::cout << "❌ FAIL: " << test_name << " (Expected: '" << expected << "', Got: '" << actual << "')" << std::endl;
        }
    }

    void assert_equal(int actual, int expected, const std::string& test_name) {
        tests_run++;
        if (actual == expected) {
            tests_passed++;
            std::cout << "✅ PASS: " << test_name << std::endl;
        } else {
            tests_failed++;
            std::cout << "❌ FAIL: " << test_name << " (Expected: " << expected << ", Got: " << actual << ")" << std::endl;
        }
    }

    void assert_equal(double actual, double expected, const std::string& test_name, double tolerance = 0.001) {
        tests_run++;
        if (std::abs(actual - expected) < tolerance) {
            tests_passed++;
            std::cout << "✅ PASS: " << test_name << std::endl;
        } else {
            tests_failed++;
            std::cout << "❌ FAIL: " << test_name << " (Expected: " << expected << ", Got: " << actual << ")" << std::endl;
        }
    }

    void print_summary() {
        std::cout << "\n=== TEST SUMMARY ===" << std::endl;
        std::cout << "Tests run: " << tests_run << std::endl;
        std::cout << "Tests passed: " << tests_passed << std::endl;
        std::cout << "Tests failed: " << tests_failed << std::endl;
        std::cout << "Success rate: " << (tests_run > 0 ? (100.0 * tests_passed / tests_run) : 0) << "%" << std::endl;
        
        if (tests_failed == 0) {
            std::cout << "🎉 ALL TESTS PASSED! 🎉" << std::endl;
        } else {
            std::cout << "⚠️  " << tests_failed << " TESTS FAILED" << std::endl;
        }
    }
};

// Helper function to create test records
MBORecord createRecord(const std::string& ts_recv, const std::string& ts_event, 
                      char action, char side, double price, int size, long order_id, 
                      const std::string& symbol = "TEST") {
    MBORecord record;
    record.ts_recv = ts_recv;
    record.ts_event = ts_event;
    record.rtype = 160;
    record.publisher_id = 2;
    record.instrument_id = 1108;
    record.action = action;
    record.side = side;
    record.price = price;
    record.size = size;
    record.channel_id = 0;
    record.order_id = order_id;
    record.flags = 130;
    record.ts_in_delta = 165200;
    record.sequence = 851012;
    record.symbol = symbol;
    return record;
}

// Test basic orderbook functionality
void test_basic_orderbook(TestFramework& tf) {
    std::cout << "\n=== Testing Basic Orderbook Functionality ===" << std::endl;
    
    OrderBook book;
    
    // Test empty book
    auto bid_levels = book.getBidLevels();
    auto ask_levels = book.getAskLevels();
    tf.assert_equal(bid_levels.size(), 0, "Empty book should have no bid levels");
    tf.assert_equal(ask_levels.size(), 0, "Empty book should have no ask levels");
    
    // Test adding a bid order
    auto bid_record = createRecord("2025-01-01T10:00:00Z", "2025-01-01T10:00:00Z", 
                                 'A', 'B', 100.0, 100, 1);
    book.processRecord(bid_record);
    
    bid_levels = book.getBidLevels();
    ask_levels = book.getAskLevels();
    tf.assert_equal(bid_levels.size(), 1, "Should have one bid level after adding bid");
    tf.assert_equal(ask_levels.size(), 0, "Should have no ask levels");
    tf.assert_equal(bid_levels[0].price, 100.0, "Bid price should be 100.0");
    tf.assert_equal(bid_levels[0].total_size, 100, "Bid size should be 100");
    tf.assert_equal(bid_levels[0].order_count, 1, "Bid order count should be 1");
    
    // Test adding an ask order
    auto ask_record = createRecord("2025-01-01T10:00:01Z", "2025-01-01T10:00:01Z", 
                                 'A', 'A', 101.0, 50, 2);
    book.processRecord(ask_record);
    
    bid_levels = book.getBidLevels();
    ask_levels = book.getAskLevels();
    tf.assert_equal(bid_levels.size(), 1, "Should still have one bid level");
    tf.assert_equal(ask_levels.size(), 1, "Should have one ask level");
    tf.assert_equal(ask_levels[0].price, 101.0, "Ask price should be 101.0");
    tf.assert_equal(ask_levels[0].total_size, 50, "Ask size should be 50");
    tf.assert_equal(ask_levels[0].order_count, 1, "Ask order count should be 1");
}

// Test order cancellation
void test_order_cancellation(TestFramework& tf) {
    std::cout << "\n=== Testing Order Cancellation ===" << std::endl;
    
    OrderBook book;
    
    // Add a bid order
    auto bid_record = createRecord("2025-01-01T10:00:00Z", "2025-01-01T10:00:00Z", 
                                 'A', 'B', 100.0, 100, 1);
    book.processRecord(bid_record);
    
    // Cancel the order
    auto cancel_record = createRecord("2025-01-01T10:00:01Z", "2025-01-01T10:00:01Z", 
                                    'C', 'B', 100.0, 100, 1);
    book.processRecord(cancel_record);
    
    auto bid_levels = book.getBidLevels();
    tf.assert_equal(bid_levels.size(), 0, "Should have no bid levels after cancellation");
}

// Test multiple orders at same price level
void test_multiple_orders_same_price(TestFramework& tf) {
    std::cout << "\n=== Testing Multiple Orders at Same Price ===" << std::endl;
    
    OrderBook book;
    
    // Add two bid orders at same price
    auto bid1 = createRecord("2025-01-01T10:00:00Z", "2025-01-01T10:00:00Z", 
                           'A', 'B', 100.0, 100, 1);
    auto bid2 = createRecord("2025-01-01T10:00:01Z", "2025-01-01T10:00:01Z", 
                           'A', 'B', 100.0, 200, 2);
    
    book.processRecord(bid1);
    book.processRecord(bid2);
    
    auto bid_levels = book.getBidLevels();
    tf.assert_equal(bid_levels.size(), 1, "Should have one bid level");
    tf.assert_equal(bid_levels[0].total_size, 300, "Total size should be 300");
    tf.assert_equal(bid_levels[0].order_count, 2, "Order count should be 2");
}

// Test price level ordering
void test_price_level_ordering(TestFramework& tf) {
    std::cout << "\n=== Testing Price Level Ordering ===" << std::endl;
    
    OrderBook book;
    
    // Add bids in random order
    auto bid1 = createRecord("2025-01-01T10:00:00Z", "2025-01-01T10:00:00Z", 
                           'A', 'B', 98.0, 100, 1);
    auto bid2 = createRecord("2025-01-01T10:00:01Z", "2025-01-01T10:00:01Z", 
                           'A', 'B', 100.0, 100, 2);
    auto bid3 = createRecord("2025-01-01T10:00:02Z", "2025-01-01T10:00:02Z", 
                           'A', 'B', 99.0, 100, 3);
    
    book.processRecord(bid1);
    book.processRecord(bid2);
    book.processRecord(bid3);
    
    auto bid_levels = book.getBidLevels();
    tf.assert_equal(bid_levels.size(), 3, "Should have three bid levels");
    tf.assert_equal(bid_levels[0].price, 100.0, "Highest bid should be first");
    tf.assert_equal(bid_levels[1].price, 99.0, "Second highest bid should be second");
    tf.assert_equal(bid_levels[2].price, 98.0, "Lowest bid should be last");
    
    // Test ask ordering
    auto ask1 = createRecord("2025-01-01T10:00:03Z", "2025-01-01T10:00:03Z", 
                           'A', 'A', 102.0, 100, 4);
    auto ask2 = createRecord("2025-01-01T10:00:04Z", "2025-01-01T10:00:04Z", 
                           'A', 'A', 101.0, 100, 5);
    auto ask3 = createRecord("2025-01-01T10:00:05Z", "2025-01-01T10:00:05Z", 
                           'A', 'A', 103.0, 100, 6);
    
    book.processRecord(ask1);
    book.processRecord(ask2);
    book.processRecord(ask3);
    
    auto ask_levels = book.getAskLevels();
    tf.assert_equal(ask_levels.size(), 3, "Should have three ask levels");
    tf.assert_equal(ask_levels[0].price, 101.0, "Lowest ask should be first");
    tf.assert_equal(ask_levels[1].price, 102.0, "Second lowest ask should be second");
    tf.assert_equal(ask_levels[2].price, 103.0, "Highest ask should be last");
}

// Test T->F->C sequence handling
void test_trade_sequence(TestFramework& tf) {
    std::cout << "\n=== Testing T->F->C Sequence Handling ===" << std::endl;
    
    OrderBook book;
    
    // First add an order that will be traded
    auto add_record = createRecord("2025-01-01T10:00:00Z", "2025-01-01T10:00:00Z", 
                                 'A', 'B', 100.0, 100, 1);
    book.processRecord(add_record);
    
    // Create T->F->C sequence
    auto trade_record = createRecord("2025-01-01T10:00:01Z", "2025-01-01T10:00:01Z", 
                                   'T', 'A', 100.0, 100, 2);  // Trade appears on opposite side
    auto fill_record = createRecord("2025-01-01T10:00:02Z", "2025-01-01T10:00:02Z", 
                                  'F', 'B', 100.0, 100, 1);   // Fill on actual side
    auto cancel_record = createRecord("2025-01-01T10:00:03Z", "2025-01-01T10:00:03Z", 
                                    'C', 'B', 100.0, 100, 1); // Cancel on actual side
    
    book.processRecord(trade_record);
    book.processRecord(fill_record);
    book.processRecord(cancel_record);
    
    auto bid_levels = book.getBidLevels();
    tf.assert_equal(bid_levels.size(), 0, "Bid level should be removed after T->F->C sequence");
}

// Test special cases
void test_special_cases(TestFramework& tf) {
    std::cout << "\n=== Testing Special Cases ===" << std::endl;
    
    OrderBook book;
    
    // Test 'R' (reset) action - should be ignored in processing
    auto reset_record = createRecord("2025-01-01T10:00:00Z", "2025-01-01T10:00:00Z", 
                                   'R', 'N', 0.0, 0, 0);
    book.processRecord(reset_record);
    
    auto bid_levels = book.getBidLevels();
    auto ask_levels = book.getAskLevels();
    tf.assert_equal(bid_levels.size(), 0, "Reset action should not affect orderbook");
    tf.assert_equal(ask_levels.size(), 0, "Reset action should not affect orderbook");
    
    // Test 'T' action with side 'N' - should not alter orderbook
    auto trade_n_record = createRecord("2025-01-01T10:00:01Z", "2025-01-01T10:00:01Z", 
                                     'T', 'N', 100.0, 100, 1);
    book.processRecord(trade_n_record);
    
    bid_levels = book.getBidLevels();
    ask_levels = book.getAskLevels();
    tf.assert_equal(bid_levels.size(), 0, "Trade with side 'N' should not affect orderbook");
    tf.assert_equal(ask_levels.size(), 0, "Trade with side 'N' should not affect orderbook");
}

// Test MBP output generation
void test_mbp_output(TestFramework& tf) {
    std::cout << "\n=== Testing MBP Output Generation ===" << std::endl;
    
    OrderBook book;
    
    // Add some orders
    auto bid_record = createRecord("2025-01-01T10:00:00Z", "2025-01-01T10:00:00Z", 
                                 'A', 'B', 100.0, 100, 1);
    auto ask_record = createRecord("2025-01-01T10:00:01Z", "2025-01-01T10:00:01Z", 
                                 'A', 'A', 101.0, 50, 2);
    
    book.processRecord(bid_record);
    book.processRecord(ask_record);
    
    // Generate MBP output
    std::string mbp_output = book.generateMBPOutput(ask_record, 0);
    
    // Check that output contains expected fields
    tf.assert_true(mbp_output.find("100.00") != std::string::npos, "MBP output should contain bid price");
    tf.assert_true(mbp_output.find("101.00") != std::string::npos, "MBP output should contain ask price");
    tf.assert_true(mbp_output.find("100") != std::string::npos, "MBP output should contain bid size");
    tf.assert_true(mbp_output.find("50") != std::string::npos, "MBP output should contain ask size");
    tf.assert_true(mbp_output.find("TEST") != std::string::npos, "MBP output should contain symbol");
}

// Test CSV parsing
void test_csv_parsing(TestFramework& tf) {
    std::cout << "\n=== Testing CSV Parsing ===" << std::endl;
    
    // Test single line parsing
    std::string test_line = "2025-01-01T10:00:00Z,2025-01-01T10:00:00Z,160,2,1108,A,B,100.50,100,0,12345,130,165200,851012,TEST";
    MBORecord record = CSVParser::parseLine(test_line);
    
    tf.assert_equal(record.ts_recv, "2025-01-01T10:00:00Z", "Timestamp receive should match");
    tf.assert_equal(record.action, 'A', "Action should be 'A'");
    tf.assert_equal(record.side, 'B', "Side should be 'B'");
    tf.assert_equal(record.price, 100.50, "Price should be 100.50");
    tf.assert_equal(record.size, 100, "Size should be 100");
    tf.assert_equal(record.order_id, 12345, "Order ID should be 12345");
    tf.assert_equal(record.symbol, "TEST", "Symbol should be TEST");
    
    // Test file parsing
    std::vector<MBORecord> records = CSVParser::parseFile("../data/test_data.csv");
    tf.assert_equal(records.size(), 8, "Should parse 8 records from test file");
    
    if (records.size() >= 8) {
        tf.assert_equal(records[0].action, 'R', "First record should be reset action");
        tf.assert_equal(records[1].action, 'A', "Second record should be add action");
        tf.assert_equal(records[1].side, 'B', "Second record should be bid");
        tf.assert_equal(records[1].price, 100.00, "Second record price should be 100.00");
    }
}

// Test performance with large dataset
void test_performance(TestFramework& tf) {
    std::cout << "\n=== Testing Performance ===" << std::endl;
    
    OrderBook book;
    const int num_orders = 10000;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Add many orders
    for (int i = 0; i < num_orders; ++i) {
        double price = 100.0 + (i % 100) * 0.01;  // Prices from 100.00 to 100.99
        char side = (i % 2 == 0) ? 'B' : 'A';
        auto record = createRecord("2025-01-01T10:00:00Z", "2025-01-01T10:00:00Z", 
                                 'A', side, price, 100, i);
        book.processRecord(record);
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    double orders_per_second = (num_orders * 1000000.0) / duration.count();
    
    tf.assert_true(orders_per_second > 10000, "Should process at least 10,000 orders per second");
    std::cout << "Performance: " << orders_per_second << " orders/second" << std::endl;
}

// Test edge cases
void test_edge_cases(TestFramework& tf) {
    std::cout << "\n=== Testing Edge Cases ===" << std::endl;
    
    OrderBook book;
    
    // Test zero size order
    auto zero_size_record = createRecord("2025-01-01T10:00:00Z", "2025-01-01T10:00:00Z", 
                                       'A', 'B', 100.0, 0, 1);
    book.processRecord(zero_size_record);
    
    auto bid_levels = book.getBidLevels();
    tf.assert_equal(bid_levels.size(), 0, "Zero size order should not create price level");
    
    // Test negative price (should be handled gracefully)
    auto negative_price_record = createRecord("2025-01-01T10:00:01Z", "2025-01-01T10:00:01Z", 
                                            'A', 'B', -100.0, 100, 2);
    book.processRecord(negative_price_record);
    
    bid_levels = book.getBidLevels();
    tf.assert_equal(bid_levels.size(), 1, "Negative price order should still be processed");
    tf.assert_equal(bid_levels[0].price, -100.0, "Negative price should be preserved");
    
    // Test very large order ID
    auto large_id_record = createRecord("2025-01-01T10:00:02Z", "2025-01-01T10:00:02Z", 
                                      'A', 'A', 101.0, 100, 9223372036854775807L);
    book.processRecord(large_id_record);
    
    auto ask_levels = book.getAskLevels();
    tf.assert_equal(ask_levels.size(), 1, "Large order ID should be handled");
}

// Test incomplete T->F->C sequences
void test_incomplete_sequences(TestFramework& tf) {
    std::cout << "\n=== Testing Incomplete T->F->C Sequences ===" << std::endl;
    
    OrderBook book;
    
    // Add an order
    auto add_record = createRecord("2025-01-01T10:00:00Z", "2025-01-01T10:00:00Z", 
                                 'A', 'B', 100.0, 100, 1);
    book.processRecord(add_record);
    
    // Test incomplete T->F sequence (no C)
    auto trade_record = createRecord("2025-01-01T10:00:01Z", "2025-01-01T10:00:01Z", 
                                   'T', 'A', 100.0, 100, 2);
    auto fill_record = createRecord("2025-01-01T10:00:02Z", "2025-01-01T10:00:02Z", 
                                  'F', 'B', 100.0, 100, 1);
    
    book.processRecord(trade_record);
    book.processRecord(fill_record);
    
    auto bid_levels = book.getBidLevels();
    tf.assert_equal(bid_levels.size(), 1, "Incomplete sequence should not remove order");
    tf.assert_equal(bid_levels[0].total_size, 100, "Order size should remain unchanged");
}

// Test complete reconstruction pipeline
void test_reconstruction_pipeline(TestFramework& tf) {
    std::cout << "\n=== Testing Complete Reconstruction Pipeline ===" << std::endl;
    
    // Parse test data
    std::vector<MBORecord> records = CSVParser::parseFile("../data/test_data.csv");
    tf.assert_equal(records.size(), 8, "Should parse 8 records from test file");
    
    // Process through orderbook
    OrderBook book;
    for (const auto& record : records) {
        book.processRecord(record);
    }
    
    // Check final state
    auto bid_levels = book.getBidLevels();
    auto ask_levels = book.getAskLevels();
    
    // After processing the test data, we should have:
    // - No bids (order 1 was cancelled, order 3 was traded via T->F->C sequence)
    // - Ask at 101.00 with 50 size (order 2)
    
    tf.assert_equal(bid_levels.size(), 0, "Should have no bid levels after processing");
    tf.assert_equal(ask_levels.size(), 1, "Should have one ask level after processing");
    
    if (ask_levels.size() >= 1) {
        tf.assert_equal(ask_levels[0].price, 101.00, "Ask price should be 101.00");
        tf.assert_equal(ask_levels[0].total_size, 50, "Ask size should be 50");
    }
}

int main() {
    std::cout << "🧪 Starting Orderbook Unit Tests..." << std::endl;
    
    TestFramework tf;
    
    // Run all tests
    test_basic_orderbook(tf);
    test_order_cancellation(tf);
    test_multiple_orders_same_price(tf);
    test_price_level_ordering(tf);
    test_trade_sequence(tf);
    test_special_cases(tf);
    test_mbp_output(tf);
    test_csv_parsing(tf);
    test_performance(tf);
    test_edge_cases(tf);
    test_incomplete_sequences(tf);
    test_reconstruction_pipeline(tf);
    
    // Print summary
    tf.print_summary();
    
    return 0;
}