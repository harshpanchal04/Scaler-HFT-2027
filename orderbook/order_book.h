#pragma once
#include <cstdint>
#include <vector>
#include <string>

struct Order {
    uint64_t order_id;
    bool is_buy;
    double price;
    uint64_t quantity;
    uint64_t timestamp_ns;
};

struct PriceLevel {
    double price;
    uint64_t total_quantity;
};

class OrderBook {
public:
    OrderBook();
    ~OrderBook();
    
    // Insert a new order into the book
    void add_order(const Order& order);
    
    // Cancel an existing order by its ID
    bool cancel_order(uint64_t order_id);
    
    // Amend an existing order's price or quantity
    bool amend_order(uint64_t order_id, double new_price, uint64_t new_quantity);
    
    // Get a snapshot of top N bid and ask levels (aggregated quantities)
    void get_snapshot(size_t depth, std::vector<PriceLevel>& bids, 
                     std::vector<PriceLevel>& asks) const;
    
    // Print current state of the order book
    void print_book(size_t depth = 10) const;

private:
    class Implementation;
    Implementation* pimpl;
};