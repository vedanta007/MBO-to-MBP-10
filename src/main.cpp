#include "orderbook.h"
#include <iostream>
#include <fstream>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_mbo_file.csv>" << std::endl;
        return 1;
    }

    std::string input_file = argv[1];
    std::string output_file = "../output/output_mbp.csv";

    std::cout << "Starting orderbook reconstruction..." << std::endl;
    std::cout << "Input file: " << input_file << std::endl;
    std::cout << "Output file: " << output_file << std::endl;

    // Initialize performance timer
    PerformanceTimer total_timer("Total processing");

    // Parse input file
    std::vector<MBORecord> records;
    {
        PerformanceTimer parse_timer("CSV parsing");
        records = CSVParser::parseFile(input_file);
    }

    if (records.empty()) {
        std::cerr << "Error: No records found in input file" << std::endl;
        return 1;
    }

    std::cout << "Loaded " << records.size() << " MBO records" << std::endl;

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
    {
        PerformanceTimer process_timer("Orderbook processing");

        int processed_count = 0;
        int output_count = 0;
        int row_index = 0;

        for (const auto& record : records) {
            // Process the record
            orderbook.processRecord(record);
            processed_count++;

            // Generate output for every record that affects the book
            // Include the initial 'R' (reset) action as it appears in expected output
            std::string mbp_line = orderbook.generateMBPOutput(record, row_index);
            output << mbp_line << "\n";
            output_count++;
            row_index++;

            // Progress indicator
            if (processed_count % 1000 == 0) {
                std::cout << "Processed " << processed_count << " records..." << std::endl;
            }
        }

        std::cout << "Processing complete!" << std::endl;
        std::cout << "Processed " << processed_count << " MBO records" << std::endl;
        std::cout << "Generated " << output_count << " MBP records" << std::endl;
    }

    output.close();

    // Final orderbook state
    std::cout << "\nFinal orderbook state:" << std::endl;
    orderbook.printBook();

    std::cout << "Output written to: " << output_file << std::endl;

    return 0;
}