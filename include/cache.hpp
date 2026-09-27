#pragma once

#include <cstddef>
#include <functional>
#include <list>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

// Values are integers so a miss can be reported as -1.
template <typename Key>
class BasicCache {
public:
    explicit BasicCache(std::ptrdiff_t capacity)
        : capacity_(validated_capacity(capacity)) {
        // A new entry is indexed before the old LRU entry is evicted.
        index_.reserve(capacity_ + 1);
    }

    BasicCache(const BasicCache& other)
        : capacity_(other.capacity_), order_(other.order_) {
        index_.reserve(capacity_ + 1);
        for (auto entry = order_.begin(); entry != order_.end(); ++entry) {
            index_.emplace(entry->first, entry);
        }
    }

    BasicCache& operator=(const BasicCache& other) {
        if (this != &other) {
            BasicCache copy(other);
            swap(copy);
        }
        return *this;
    }

    void swap(BasicCache& other) {
        using std::swap;
        swap(capacity_, other.capacity_);
        order_.swap(other.order_);
        index_.swap(other.index_);
    }

    // A hit also moves the entry to the most-recently-used end.
    int get(const Key& key) {
        const auto found = index_.find(key);
        if (found == index_.end()) {
            return -1;
        }

        order_.splice(order_.begin(), order_, found->second);
        return found->second->second;
    }

    void put(const Key& key, int value) {
        const auto found = index_.find(key);
        if (found != index_.end()) {
            found->second->second = value;
            order_.splice(order_.begin(), order_, found->second);
            return;
        }

        order_.emplace_front(key, value);
        try {
            index_.emplace(order_.front().first, order_.begin());
        } catch (...) {
            order_.pop_front();
            throw;
        }

        if (order_.size() > capacity_) {
            index_.erase(order_.back().first);
            order_.pop_back();
        }
    }

private:
    using Entry = std::pair<Key, int>;
    using Order = std::list<Entry>;

    static std::size_t validated_capacity(std::ptrdiff_t capacity) {
        if (capacity <= 0) {
            throw std::invalid_argument("Cache capacity must be positive");
        }
        return static_cast<std::size_t>(capacity);
    }

    std::size_t capacity_;
    Order order_;  // Front is most recently used; back is least recently used.
    std::unordered_map<Key, typename Order::iterator> index_;
};

using Cache = BasicCache<std::string>;
