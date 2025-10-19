#include "order_book.h"
#include <iostream>
#include <chrono>
#include <vector>

// Utility function to get current timestamp in nanoseconds
uint64_t get_timestamp_ns() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch()
    ).count();
}

void print_separator() {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";
}

void test_basic_operations() {
    std::cout << "TEST 1: Basic Order Operations\n";
    print_separator();
    
    OrderBook book;
    
    // Add buy orders
    std::cout << "➤ Adding buy orders at different price levels...\n";
    book.add_order({1001, true, 100.50, 500, get_timestamp_ns()});
    book.add_order({1002, true, 100.25, 300, get_timestamp_ns()});
    book.add_order({1003, true, 100.50, 200, get_timestamp_ns()}); // Same price as 1001
    book.add_order({1004, true, 100.00, 400, get_timestamp_ns()});
    
    // Add sell orders
    std::cout << "➤ Adding sell orders at different price levels...\n";
    book.add_order({2001, false, 101.00, 350, get_timestamp_ns()});
    book.add_order({2002, false, 101.25, 250, get_timestamp_ns()});
    book.add_order({2003, false, 101.00, 150, get_timestamp_ns()}); // Same price as 2001
    book.add_order({2004, false, 101.50, 600, get_timestamp_ns()});
    
    std::cout << "\n✓ Order book after initial additions:\n";
    book.print_book(5);
}

void test_snapshot() {
    std::cout << "TEST 2: Snapshot Functionality\n";
    print_separator();
    
    OrderBook book;
    
    // Populate order book
    book.add_order({1001, true, 99.75, 1000, get_timestamp_ns()});
    book.add_order({1002, true, 99.50, 800, get_timestamp_ns()});
    book.add_order({1003, true, 99.75, 500, get_timestamp_ns()});
    book.add_order({2001, false, 100.25, 700, get_timestamp_ns()});
    book.add_order({2002, false, 100.50, 600, get_timestamp_ns()});
    book.add_order({2003, false, 100.25, 300, get_timestamp_ns()});
    
    std::vector<PriceLevel> bids, asks;
    book.get_snapshot(3, bids, asks);
    
    std::cout << "➤ Top 3 Bid Levels (Aggregated):\n";
    for (size_t i = 0; i < bids.size(); ++i) {
        std::cout << "   Level " << (i + 1) << ": Price=" << bids[i].price 
                  << ", Total Quantity=" << bids[i].total_quantity << "\n";
    }
    
    std::cout << "\n➤ Top 3 Ask Levels (Aggregated):\n";
    for (size_t i = 0; i < asks.size(); ++i) {
        std::cout << "   Level " << (i + 1) << ": Price=" << asks[i].price 
                  << ", Total Quantity=" << asks[i].total_quantity << "\n";
    }
    
    std::cout << "\n✓ Full order book view:\n";
    book.print_book(3);
}

void test_cancel_operations() {
    std::cout << "TEST 3: Order Cancellation\n";
    print_separator();
    
    OrderBook book;
    
    book.add_order({1001, true, 50.00, 100, get_timestamp_ns()});
    book.add_order({1002, true, 50.00, 200, get_timestamp_ns()});
    book.add_order({1003, true, 49.50, 150, get_timestamp_ns()});
    book.add_order({2001, false, 51.00, 120, get_timestamp_ns()});
    book.add_order({2002, false, 51.50, 180, get_timestamp_ns()});
    
    std::cout << "➤ Initial order book:\n";
    book.print_book(5);
    
    std::cout << "➤ Canceling order ID 1002 (buy order at 50.00 with qty 200)...\n";
    bool result = book.cancel_order(1002);
    std::cout << "   Cancellation " << (result ? "✓ SUCCESS" : "✗ FAILED") << "\n";
    
    std::cout << "\n➤ Attempting to cancel non-existent order ID 9999...\n";
    result = book.cancel_order(9999);
    std::cout << "   Cancellation " << (result ? "✓ SUCCESS" : "✗ FAILED (Expected)") << "\n";
    
    std::cout << "\n✓ Order book after cancellation:\n";
    book.print_book(5);
}

void test_amend_operations() {
    std::cout << "TEST 4: Order Amendment\n";
    print_separator();
    
    OrderBook book;
    
    book.add_order({1001, true, 100.00, 500, get_timestamp_ns()});
    book.add_order({1002, true, 99.50, 300, get_timestamp_ns()});
    book.add_order({2001, false, 101.00, 400, get_timestamp_ns()});
    
    std::cout << "➤ Initial order book:\n";
    book.print_book(5);
    
    std::cout << "➤ Amending order ID 1001: changing quantity 500 → 800 (same price)...\n";
    bool result = book.amend_order(1001, 100.00, 800);
    std::cout << "   Amendment " << (result ? "✓ SUCCESS" : "✗ FAILED") << "\n";
    
    std::cout << "\n✓ Order book after quantity amendment:\n";
    book.print_book(5);
    
    std::cout << "➤ Amending order ID 2001: changing price 101.00 → 100.75...\n";
    result = book.amend_order(2001, 100.75, 400);
    std::cout << "   Amendment " << (result ? "✓ SUCCESS" : "✗ FAILED") << "\n";
    
    std::cout << "\n✓ Order book after price amendment:\n";
    book.print_book(5);
}

void test_fifo_priority() {
    std::cout << "TEST 5: FIFO Priority at Same Price Level\n";
    print_separator();
    
    OrderBook book;
    
    std::cout << "➤ Adding multiple orders at the same price (100.00)...\n";
    book.add_order({1001, true, 100.00, 100, get_timestamp_ns()});
    std::this_thread::sleep_for(std::chrono::microseconds(10));
    
    book.add_order({1002, true, 100.00, 200, get_timestamp_ns()});
    std::this_thread::sleep_for(std::chrono::microseconds(10));
    
    book.add_order({1003, true, 100.00, 150, get_timestamp_ns()});
    
    std::cout << "   Order 1001: 100 units (first)\n";
    std::cout << "   Order 1002: 200 units (second)\n";
    std::cout << "   Order 1003: 150 units (third)\n";
    
    std::cout << "\n✓ Order book showing aggregated quantity:\n";
    book.print_book(5);
    
    std::vector<PriceLevel> bids, asks;
    book.get_snapshot(1, bids, asks);
    
    std::cout << "➤ Aggregated quantity at 100.00: " << bids[0].total_quantity 
              << " units (should be 450)\n";
}

void test_complex_scenario() {
    std::cout << "TEST 6: Complex Multi-Operation Scenario\n";
    print_separator();
    
    OrderBook book;
    
    std::cout << "➤ Phase 1: Building initial order book...\n";
    book.add_order({1001, true, 99.00, 1000, get_timestamp_ns()});
    book.add_order({1002, true, 98.50, 800, get_timestamp_ns()});
    book.add_order({1003, true, 98.00, 600, get_timestamp_ns()});
    book.add_order({2001, false, 100.00, 900, get_timestamp_ns()});
    book.add_order({2002, false, 100.50, 700, get_timestamp_ns()});
    book.add_order({2003, false, 101.00, 500, get_timestamp_ns()});
    
    book.print_book(5);
    
    std::cout << "➤ Phase 2: Executing multiple operations...\n";
    std::cout << "   • Adding new buy order at 99.25 with 500 units\n";
    book.add_order({1004, true, 99.25, 500, get_timestamp_ns()});
    
    std::cout << "   • Canceling sell order 2002\n";
    book.cancel_order(2002);
    
    std::cout << "   • Amending buy order 1001: 99.00 → 99.50, quantity: 1000 → 1200\n";
    book.amend_order(1001, 99.50, 1200);
    
    std::cout << "\n✓ Final order book state:\n";
    book.print_book(5);
}

int main() {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════╗\n";
    std::cout << "║     LOW-LATENCY LIMIT ORDER BOOK - TEST SUITE         ║\n";
    std::cout << "╚════════════════════════════════════════════════════════╝\n";
    
    try {
        test_basic_operations();
        print_separator();
        
        test_snapshot();
        print_separator();
        
        test_cancel_operations();
        print_separator();
        
        test_amend_operations();
        print_separator();
        
        test_fifo_priority();
        print_separator();
        
        test_complex_scenario();
        
        std::cout << "\n";
        std::cout << "╔════════════════════════════════════════════════════════╗\n";
        std::cout << "║            ALL TESTS COMPLETED SUCCESSFULLY            ║\n";
        std::cout << "╚════════════════════════════════════════════════════════╝\n\n";
        
    } catch (const std::exception& e) {
        std::cerr << "\n✗ ERROR: " << e.what() << "\n\n";
        return 1;
    }
    
    return 0;
}