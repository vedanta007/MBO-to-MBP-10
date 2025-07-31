#include "orderbook.h"
#include <algorithm>

// CSVParser Implementation
std::vector<MBORecord> CSVParser::parseFile(const std::string& filename) {
    std::vector<MBORecord> records;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return records;
    }

    std::string line;
    bool first_line = true;

    while (std::getline(file, line)) {
        if (first_line) {
            first_line = false;
            continue; // Skip header
        }

        if (!line.empty()) {
            try {
                MBORecord record = parseLine(line);
                records.push_back(record);
            } catch (const std::exception& e) {
                std::cerr << "Error parsing line: " << line << std::endl;
                std::cerr << "Exception: " << e.what() << std::endl;
            }
        }
    }

    file.close();
    return records;
}

MBORecord CSVParser::parseLine(const std::string& line) {
    std::vector<std::string> fields = splitCSV(line);

    if (fields.size() < 15) {
        throw std::runtime_error("Insufficient fields in CSV line");
    }

    MBORecord record;

    try {
        record.ts_recv = fields[0];
        record.ts_event = fields[1];
        record.rtype = std::stoi(fields[2]);
        record.publisher_id = std::stoi(fields[3]);
        record.instrument_id = std::stoi(fields[4]);
        record.action = fields[5].empty() ? '?' : fields[5][0];
        record.side = fields[6].empty() ? '?' : fields[6][0];
        record.price = fields[7].empty() ? 0.0 : std::stod(fields[7]);
        record.size = fields[8].empty() ? 0 : std::stoi(fields[8]);
        record.channel_id = std::stoi(fields[9]);
        record.order_id = std::stol(fields[10]);
        record.flags = std::stoi(fields[11]);
        record.ts_in_delta = std::stoi(fields[12]);
        record.sequence = std::stol(fields[13]);
        record.symbol = fields[14];
    } catch (const std::exception& e) {
        std::cerr << "Error parsing fields: " << e.what() << std::endl;
        throw;
    }

    return record;
}

std::vector<std::string> CSVParser::splitCSV(const std::string& line) {
    std::vector<std::string> fields;
    std::stringstream ss(line);
    std::string field;

    while (std::getline(ss, field, ',')) {
        fields.push_back(field);
    }

    return fields;
}

// PerformanceTimer Implementation
PerformanceTimer::PerformanceTimer(const std::string& name) 
    : operation_name(name), start_time(std::chrono::high_resolution_clock::now()) {
}

PerformanceTimer::~PerformanceTimer() {
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    std::cout << operation_name << " took " << duration.count() << " microseconds" << std::endl;
}