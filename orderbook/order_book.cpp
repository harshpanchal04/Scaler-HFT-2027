#include "order_book.h"
#include <unordered_map>
#include <map>
#include <list>
#include <iostream>
#include <iomanip>
#include <algorithm>

// Internal structure to maintain orders at each price level
struct OrderNode {
    Order order;
    std::list<OrderNode*>::iterator position_in_level;
};

// Price level maintains FIFO queue of orders
struct Level {
    double price;
    uint64_t aggregated_volume;
    std::list<OrderNode*> order_queue;
    
    Level(double p) : price(p), aggregated_volume(0) {}
};

class OrderBook::Implementation {
public:
    // Order lookup for O(1) access
    std::unordered_map<uint64_t, OrderNode*> order_registry;
    
    // Buy side: price descending (highest first)
    std::map<double, Level*, std::greater<double>> bid_levels;
    
    // Sell side: price ascending (lowest first)
    std::map<double, Level*, std::less<double>> ask_levels;
    
    ~Implementation() {
        cleanup_side(bid_levels);
        cleanup_side(ask_levels);
    }
    
    template<typename MapType>
    void cleanup_side(MapType& levels) {
        for (auto& pair : levels) {
            Level* lvl = pair.second;
            for (OrderNode* node : lvl->order_queue) {
                delete node;
            }
            delete lvl;
        }
        levels.clear();
    }
    
    void insert_order(const Order& ord) {
        OrderNode* node = new OrderNode();
        node->order = ord;
        
        if (ord.is_buy) {
            insert_into_side(node, bid_levels);
        } else {
            insert_into_side(node, ask_levels);
        }
        
        order_registry[ord.order_id] = node;
    }
    
    template<typename MapType>
    void insert_into_side(OrderNode* node, MapType& side) {
        double px = node->order.price;
        Level* lvl = nullptr;
        
        auto iter = side.find(px);
        if (iter == side.end()) {
            lvl = new Level(px);
            side[px] = lvl;
        } else {
            lvl = iter->second;
        }
        
        lvl->order_queue.push_back(node);
        node->position_in_level = --lvl->order_queue.end();
        lvl->aggregated_volume += node->order.quantity;
    }
    
    bool remove_order(uint64_t oid) {
        auto lookup = order_registry.find(oid);
        if (lookup == order_registry.end()) {
            return false;
        }
        
        OrderNode* node = lookup->second;
        bool buy_side = node->order.is_buy;
        double px = node->order.price;
        
        if (buy_side) {
            remove_from_side(node, bid_levels, px);
        } else {
            remove_from_side(node, ask_levels, px);
        }
        
        order_registry.erase(oid);
        delete node;
        return true;
    }
    
    template<typename MapType>
    void remove_from_side(OrderNode* node, MapType& side, double price) {
        auto iter = side.find(price);
        if (iter == side.end()) return;
        
        Level* lvl = iter->second;
        lvl->aggregated_volume -= node->order.quantity;
        lvl->order_queue.erase(node->position_in_level);
        
        if (lvl->order_queue.empty()) {
            delete lvl;
            side.erase(iter);
        }
    }
    
    bool modify_order(uint64_t oid, double new_px, uint64_t new_qty) {
        auto lookup = order_registry.find(oid);
        if (lookup == order_registry.end()) {
            return false;
        }
        
        OrderNode* node = lookup->second;
        
        // Price change requires removal and re-insertion
        if (node->order.price != new_px) {
            Order temp_order = node->order;
            temp_order.price = new_px;
            temp_order.quantity = new_qty;
            
            remove_order(oid);
            insert_order(temp_order);
        } else {
            // Quantity change only
            double px = node->order.price;
            Level* lvl = nullptr;
            
            if (node->order.is_buy) {
                lvl = bid_levels[px];
            } else {
                lvl = ask_levels[px];
            }
            
            lvl->aggregated_volume -= node->order.quantity;
            node->order.quantity = new_qty;
            lvl->aggregated_volume += new_qty;
        }
        
        return true;
    }
    
    template<typename MapType>
    void extract_levels(const MapType& side, size_t depth, 
                       std::vector<PriceLevel>& output) const {
        output.clear();
        output.reserve(depth);
        
        size_t count = 0;
        for (const auto& pair : side) {
            if (count >= depth) break;
            
            Level* lvl = pair.second;
            PriceLevel pl;
            pl.price = lvl->price;
            pl.total_quantity = lvl->aggregated_volume;
            output.push_back(pl);
            ++count;
        }
    }
    
    void display_book(size_t depth) const {
        std::vector<PriceLevel> bids, asks;
        extract_levels(bid_levels, depth, bids);
        extract_levels(ask_levels, depth, asks);
        
        std::cout << "\n╔════════════════════════════════════════════════════════╗\n";
        std::cout << "║           LIMIT ORDER BOOK (Top " << depth << " Levels)           ║\n";
        std::cout << "╠════════════════════════════════════════════════════════╣\n";
        
        // Display asks in reverse (highest to lowest)
        std::cout << "║  ASK SIDE (Sell Orders)                                ║\n";
        std::cout << "╟────────────────────────────────────────────────────────╢\n";
        
        if (asks.empty()) {
            std::cout << "║  [No sell orders]                                      ║\n";
        } else {
            for (auto it = asks.rbegin(); it != asks.rend(); ++it) {
                std::cout << "║  Price: " << std::fixed << std::setprecision(2) 
                         << std::setw(8) << it->price 
                         << "  |  Quantity: " << std::setw(10) << it->total_quantity 
                         << "              ║\n";
            }
        }
        
        std::cout << "╠════════════════════════════════════════════════════════╣\n";
        std::cout << "║                      SPREAD                            ║\n";
        std::cout << "╠════════════════════════════════════════════════════════╣\n";
        
        // Display bids (highest to lowest)
        std::cout << "║  BID SIDE (Buy Orders)                                 ║\n";
        std::cout << "╟────────────────────────────────────────────────────────╢\n";
        
        if (bids.empty()) {
            std::cout << "║  [No buy orders]                                       ║\n";
        } else {
            for (const auto& bid : bids) {
                std::cout << "║  Price: " << std::fixed << std::setprecision(2) 
                         << std::setw(8) << bid.price 
                         << "  |  Quantity: " << std::setw(10) << bid.total_quantity 
                         << "              ║\n";
            }
        }
        
        std::cout << "╚════════════════════════════════════════════════════════╝\n\n";
    }
};

// OrderBook public interface implementation
OrderBook::OrderBook() : pimpl(new Implementation()) {}

OrderBook::~OrderBook() {
    delete pimpl;
}

void OrderBook::add_order(const Order& order) {
    pimpl->insert_order(order);
}

bool OrderBook::cancel_order(uint64_t order_id) {
    return pimpl->remove_order(order_id);
}

bool OrderBook::amend_order(uint64_t order_id, double new_price, uint64_t new_quantity) {
    return pimpl->modify_order(order_id, new_price, new_quantity);
}

void OrderBook::get_snapshot(size_t depth, std::vector<PriceLevel>& bids, 
                            std::vector<PriceLevel>& asks) const {
    pimpl->extract_levels(pimpl->bid_levels, depth, bids);
    pimpl->extract_levels(pimpl->ask_levels, depth, asks);
}

void OrderBook::print_book(size_t depth) const {
    pimpl->display_book(depth);
}