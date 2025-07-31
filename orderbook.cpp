#include "orderbook.h"
#include <algorithm>
#include <cmath>

// OrderBook Implementation
OrderBook::OrderBook() {
    orders.reserve(10000);  // Pre-allocate for performance
}

OrderBook::~OrderBook() {
    clear();
}

void OrderBook::processRecord(const MBORecord& record) {
    // Handle special cases first
    if (record.action == 'R') {
        // Clear/Reset - ignore as per requirements
        return;
    }

    if (record.side == 'N' && record.action == 'T') {
        // Trade with side 'N' - don't alter orderbook as per requirements
        return;
    }

    // Handle T->F->C sequence detection
    if (record.action == 'T' || record.action == 'F' || record.action == 'C') {
        pending_sequence.push_back(record);

        // Check if we have a complete T->F->C sequence
        if (pending_sequence.size() >= 3) {
            size_t start = pending_sequence.size() - 3;
            if (pending_sequence[start].action == 'T' &&
                pending_sequence[start + 1].action == 'F' &&
                pending_sequence[start + 2].action == 'C') {

                // Process as combined trade sequence
                std::vector<MBORecord> sequence(pending_sequence.begin() + start, pending_sequence.end());
                handleTradeSequence(sequence);

                // Clear the processed sequence
                pending_sequence.erase(pending_sequence.begin() + start, pending_sequence.end());
                return;
            }
        }

        // If not part of a complete sequence and action is C, process immediately
        if (record.action == 'C') {
            cancelOrder(record);
        }
        // For standalone T and F actions, we might need to handle differently
        // But based on requirements, they should mostly be part of sequences

        return;
    }

    // Handle regular Add and Cancel actions
    switch (record.action) {
        case 'A':
            addOrder(record);
            break;
        case 'C':
            cancelOrder(record);
            break;
        default:
            // Unknown action, skip
            break;
    }
}

void OrderBook::addOrder(const MBORecord& record) {
    // Create order entry
    Order order(record.order_id, record.price, record.size, record.side);
    orders[record.order_id] = order;

    // Update price level
    if (record.side == 'B') {
        updatePriceLevel(bids, record.price, record.size, 1);
    } else if (record.side == 'A') {
        updatePriceLevel(asks, record.price, record.size, 1);
    }
}

void OrderBook::cancelOrder(const MBORecord& record) {
    auto it = orders.find(record.order_id);
    if (it != orders.end()) {
        const Order& order = it->second;

        // Update price level (subtract the cancelled order)
        if (order.side == 'B') {
            updatePriceLevel(bids, order.price, -order.size, -1);
        } else if (order.side == 'A') {
            updatePriceLevel(asks, order.price, -order.size, -1);
        }

        // Remove order from tracking
        orders.erase(it);
    }
}

void OrderBook::handleTradeSequence(const std::vector<MBORecord>& sequence) {
    if (sequence.size() != 3) return;

    const MBORecord& trade = sequence[0];
    const MBORecord& fill = sequence[1];
    const MBORecord& cancel = sequence[2];

    // As per requirements: Trade appears on opposite side, but actual change is on the book side
    // The Fill and Cancel actions show the actual side where the order exists
    char actual_side = fill.side;  // This is where the order actually exists in the book

    // Find and remove the filled order
    auto it = orders.find(fill.order_id);
    if (it != orders.end()) {
        const Order& order = it->second;

        // Update price level (subtract the filled/cancelled order)
        if (actual_side == 'B') {
            updatePriceLevel(bids, order.price, -order.size, -1);
        } else if (actual_side == 'A') {
            updatePriceLevel(asks, order.price, -order.size, -1);
        }

        // Remove order from tracking
        orders.erase(it);
    }
}

void OrderBook::updatePriceLevel(std::map<double, PriceLevel>& levels, double price, int size_delta, int count_delta) {
    auto it = levels.find(price);
    if (it != levels.end()) {
        // Update existing level
        it->second.total_size += size_delta;
        it->second.order_count += count_delta;

        // Remove level if no orders remain
        if (it->second.order_count <= 0 || it->second.total_size <= 0) {
            levels.erase(it);
        }
    } else if (size_delta > 0 && count_delta > 0) {
        // Add new level
        levels[price] = PriceLevel(price, size_delta, count_delta);
    }
}

std::vector<PriceLevel> OrderBook::getBidLevels(int max_levels) const {
    std::vector<PriceLevel> result;
    result.reserve(max_levels);

    // Bids are sorted in ascending order by map, but we want descending (highest first)
    auto it = bids.rbegin();
    for (int i = 0; i < max_levels && it != bids.rend(); ++it, ++i) {
        result.push_back(it->second);
    }

    return result;
}

std::vector<PriceLevel> OrderBook::getAskLevels(int max_levels) const {
    std::vector<PriceLevel> result;
    result.reserve(max_levels);

    // Asks are sorted in ascending order (lowest first) - this is what we want
    auto it = asks.begin();
    for (int i = 0; i < max_levels && it != asks.end(); ++it, ++i) {
        result.push_back(it->second);
    }

    return result;
}

std::string OrderBook::generateMBPOutput(const MBORecord& record) {
    std::stringstream ss;

    // Output the basic record information (columns 0-13)
    ss << record.ts_recv << ","
       << record.ts_event << ","
       << "10,"  // rtype for MBP
       << record.publisher_id << ","
       << record.instrument_id << ","
       << record.action << ","
       << record.side << ","
       << "0,"   // depth
       << std::fixed << std::setprecision(2) << record.price << ","
       << record.size << ","
       << record.flags << ","
       << record.ts_in_delta << ","
       << record.sequence << ",";

    // Get current orderbook levels
    auto bid_levels = getBidLevels(10);
    auto ask_levels = getAskLevels(10);

    // Output 10 levels of bid/ask data
    for (int i = 0; i < 10; ++i) {
        // Bid level
        if (i < bid_levels.size()) {
            ss << std::fixed << std::setprecision(2) << bid_levels[i].price << ","
               << bid_levels[i].total_size << ","
               << bid_levels[i].order_count << ",";
        } else {
            ss << ",0,0,";
        }

        // Ask level
        if (i < ask_levels.size()) {
            ss << std::fixed << std::setprecision(2) << ask_levels[i].price << ","
               << ask_levels[i].total_size << ","
               << ask_levels[i].order_count;
        } else {
            ss << ",0,0";
        }

        if (i < 9) ss << ",";
    }

    // Symbol and order_id
    ss << "," << record.symbol << "," << record.order_id;

    return ss.str();
}

void OrderBook::clear() {
    orders.clear();
    bids.clear();
    asks.clear();
    pending_sequence.clear();
}

void OrderBook::printBook() const {
    std::cout << "=== ORDERBOOK STATE ===\n";
    std::cout << "Orders tracked: " << orders.size() << "\n";
    std::cout << "Bid levels: " << bids.size() << "\n";
    std::cout << "Ask levels: " << asks.size() << "\n";

    auto bid_levels = getBidLevels(5);
    auto ask_levels = getAskLevels(5);

    std::cout << "\nTop 5 Asks:\n";
    for (auto it = ask_levels.rbegin(); it != ask_levels.rend(); ++it) {
        std::cout << "  " << it->price << " x " << it->total_size << " (" << it->order_count << " orders)\n";
    }

    std::cout << "\nTop 5 Bids:\n";
    for (const auto& level : bid_levels) {
        std::cout << "  " << level.price << " x " << level.total_size << " (" << level.order_count << " orders)\n";
    }
    std::cout << "=======================\n\n";
}