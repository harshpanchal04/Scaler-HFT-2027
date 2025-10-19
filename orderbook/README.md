# Low-Latency Limit Order Book Implementation

**Author:** Harsh Kumar Panchal  
**Roll Number:** 23BCS10087  
**Date:** October 19, 2025

---

## Overview

This is a high-performance, in-memory Limit Order Book (LOB) implementation in C++17, designed to simulate exchange-level order management with microsecond-level latency.

## Features

- **Add Order** - Insert buy/sell orders with FIFO priority
- **Cancel Order** - O(1) order removal by ID
- **Amend Order** - Modify price or quantity
- **Get Snapshot** - Retrieve top N aggregated price levels
- **Print Book** - Formatted order book display

## Compilation

```bash
g++ -std=c++17 -O3 -Wall -Wextra main.cpp order_book.cpp -o orderbook
./orderbook
```

## Architecture

### Data Structures

- **Order Registry**: `std::unordered_map<uint64_t, OrderNode*>` for O(1) lookup
- **Bid Levels**: `std::map<double, Level*, std::greater<double>>` (price descending)
- **Ask Levels**: `std::map<double, Level*, std::less<double>>` (price ascending)
- **Level Queue**: `std::list<OrderNode*>` for FIFO ordering

### Time Complexity

| Operation | Complexity |
|-----------|-----------|
| Add Order | O(log P) |
| Cancel Order | O(1) |
| Amend Order (quantity) | O(1) |
| Amend Order (price) | O(log P) |
| Get Snapshot | O(D) |

*P = number of price levels, D = depth*

## Design Highlights

- **PImpl Pattern**: Encapsulated implementation with clean interface
- **FIFO Priority**: Time-priority maintained at each price level using `std::list`
- **Automatic Cleanup**: Empty price levels removed automatically
- **Memory Safe**: Proper resource management with no leaks

## Implementation Details

### Amendment Logic
- **Price Change**: Cancel + Add (loses time priority)
- **Quantity Change**: In-place update (retains time priority)

### Price Level Management
- Aggregated volume tracked per level
- Levels sorted for efficient best bid/ask access
- Empty levels automatically pruned

## Test Suite

Six comprehensive test scenarios covering:
1. Basic order operations
2. Snapshot generation
3. Order cancellation
4. Order amendment
5. FIFO priority validation
6. Complex multi-operation workflows

Run: `./orderbook`

## Requirements

- C++17 or later
- GCC 7+ / Clang 5+ / MSVC 2017+

## Assignment Compliance

✅ All core requirements implemented:
- Add/Cancel/Amend operations
- Snapshot generation with aggregation
- O(1) order lookup
- FIFO priority within price levels
- Cache-friendly design
- Minimal heap allocations

---

**Repository:** https://github.com/harshpanchal04/Scaler-HFT-2027/orderbook