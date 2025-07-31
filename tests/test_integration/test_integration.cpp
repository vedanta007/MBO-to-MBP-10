#include "orderbook.h"
#include <iostream>
#include <fstream>
#include <string>

int main() {
    std::cout << "🔗 Starting Integration Test..." << std::endl;
    
    // Test the complete reconstruction pipeline
    std::string input_file = "../data/test_data.csv";
    std::string output_file = "../data/test_output_mbp.csv";
    
    std::cout << "Processing " << input_file << " -> " << output_file << std::endl;
    
    // Parse input file
    std::vector<MBORecord> records = CSVParser::parseFile(input_file);
    std::cout << "Parsed " << records.size() << " records" << std::endl;
    
    // Initialize orderbook
    OrderBook orderbook;
    
    // Open output file
    std::ofstream output(output_file);
    if (!output.is_open()) {
        std::cerr << "Error: Cannot create output file " << output_file << std::endl;
        return 1;
    }
    
    // Write CSV header
    output << ",ts_recv,ts_event,rtype,publisher_id,instrument_id,action,side,depth,price,size,flags,ts_in_delta,sequence,";
    
    // Write bid/ask level headers
    for (int i = 0; i < 10; ++i) {
        output << "bid_px_" << std::setfill('0') << std::setw(2) << i << ","
               << "bid_sz_" << std::setfill('0') << std::setw(2) << i << ","
               << "bid_ct_" << std::setfill('0') << std::setw(2) << i << ",";

        output << "ask_px_" << std::setfill('0') << std::setw(2) << i << ","
               << "ask_sz_" << std::setfill('0') << std::setw(2) << i << ","
               << "ask_ct_" << std::setfill('0') << std::setw(2) << i;

        if (i < 9) output << ",";
    }
    output << ",symbol,order_id\n";
    
    // Process records and generate output
    int row_index = 0;
    for (const auto& record : records) {
        // Process the record
        orderbook.processRecord(record);
        
        // Generate output for every record
        std::string mbp_line = orderbook.generateMBPOutput(record, row_index);
        output << mbp_line << "\n";
        row_index++;
    }
    
    output.close();
    
    // Print final orderbook state
    std::cout << "\nFinal orderbook state:" << std::endl;
    orderbook.printBook();
    
    std::cout << "Integration test completed successfully!" << std::endl;
    std::cout << "Output written to: " << output_file << std::endl;
    
    return 0;
}