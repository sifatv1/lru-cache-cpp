#include "cache.hpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

void expect_equal(int actual, int expected, const std::string& context) {
    if (actual != expected) {
        throw std::runtime_error(context + ": expected " + std::to_string(expected)
                                 + ", got " + std::to_string(actual));
    }
}

void test_given_example() {
    Cache cache(2);
    cache.put("A", 10);
    cache.put("B", 20);
    expect_equal(cache.get("A"), 10, "example A hit");
    cache.put("C", 30);
    expect_equal(cache.get("B"), -1, "example B eviction");
    expect_equal(cache.get("C"), 30, "example C hit");
    expect_equal(cache.get("A"), 10, "example A retained");
}

void test_update_and_miss() {
    Cache cache(2);
    cache.put("A", 1);
    cache.put("B", 2);
    expect_equal(cache.get("missing"), -1, "miss");
    cache.put("A", 11);  // Updating an existing key makes it most recently used.
    cache.put("C", 3);
    expect_equal(cache.get("B"), -1, "updated A evicts B");
    expect_equal(cache.get("A"), 11, "updated value");
    expect_equal(cache.get("C"), 3, "new value");
}

void test_capacity_one_and_negative_value() {
    Cache cache(1);
    cache.put("A", -1);
    expect_equal(cache.get("A"), -1, "stored negative one");
    cache.put("B", 2);
    expect_equal(cache.get("A"), -1, "capacity-one eviction");
    expect_equal(cache.get("B"), 2, "capacity-one retained key");

    Cache two(2);
    two.put("A", -1);
    two.put("B", 2);
    expect_equal(two.get("A"), -1, "negative-one hit");
    two.put("C", 3);
    expect_equal(two.get("B"), -1, "negative-one hit refreshed A");
    expect_equal(two.get("C"), 3, "C retained");
    two.put("D", 4);
    expect_equal(two.get("A"), -1, "A eventually evicted");
}

void test_invalid_capacity() {
    for (const int capacity : {0, -1, -10}) {
        bool threw = false;
        try {
            Cache cache(capacity);
        } catch (const std::invalid_argument&) {
            threw = true;
        }
        if (!threw) {
            throw std::runtime_error("nonpositive capacity was accepted");
        }
    }
}

// A small, linear-time model makes the randomized test independent of the
// hash-map and linked-list implementation.
class ReferenceCache {
public:
    explicit ReferenceCache(std::size_t capacity) : capacity_(capacity) {}

    int get(const std::string& key) {
        const auto found = find(key);
        if (found == entries_.end()) {
            return -1;
        }
        const int value = found->second;
        entries_.erase(found);
        entries_.insert(entries_.begin(), {key, value});
        return value;
    }

    void put(const std::string& key, int value) {
        const auto found = find(key);
        if (found != entries_.end()) {
            entries_.erase(found);
        }
        entries_.insert(entries_.begin(), {key, value});
        if (entries_.size() > capacity_) {
            entries_.pop_back();
        }
    }

private:
    using Entry = std::pair<std::string, int>;

    std::vector<Entry>::iterator find(const std::string& key) {
        return std::find_if(entries_.begin(), entries_.end(),
                            [&](const Entry& entry) { return entry.first == key; });
    }

    std::size_t capacity_;
    std::vector<Entry> entries_;  // Front is most recently used.
};

void test_randomized_against_reference() {
    std::mt19937 generator(20260927);
    std::uniform_int_distribution<int> key_distribution(0, 8);
    std::uniform_int_distribution<int> value_distribution(-5, 100);
    std::uniform_int_distribution<int> operation_distribution(0, 1);

    for (const int capacity : {1, 2, 5, 9}) {
        Cache cache(capacity);
        ReferenceCache reference(static_cast<std::size_t>(capacity));
        for (int step = 0; step < 20000; ++step) {
            const std::string key = "key-" + std::to_string(key_distribution(generator));
            if (operation_distribution(generator) == 0) {
                const int value = value_distribution(generator);
                cache.put(key, value);
                reference.put(key, value);
            } else {
                expect_equal(cache.get(key), reference.get(key),
                             "randomized capacity " + std::to_string(capacity)
                                 + ", step " + std::to_string(step));
            }

            if (step % 100 == 0) {
                for (int candidate = 0; candidate <= 8; ++candidate) {
                    const std::string candidate_key = "key-" + std::to_string(candidate);
                    expect_equal(cache.get(candidate_key), reference.get(candidate_key),
                                 "randomized full check");
                }
            }
        }
    }
}

void test_integer_keys() {
    BasicCache<int> cache(2);
    cache.put(42, 100);
    expect_equal(cache.get(42), 100, "integer key");
}

void test_copy() {
    Cache original(2);
    original.put("A", 1);
    original.put("B", 2);
    original.get("A");

    Cache copied(original);
    copied.put("C", 3);
    expect_equal(copied.get("B"), -1, "copy constructor order");
    expect_equal(original.get("B"), 2, "copy is independent");

    Cache assigned(1);
    assigned.put("X", 9);
    assigned = original;
    assigned = assigned;
    assigned.put("C", 3);
    expect_equal(assigned.get("A"), -1, "copy assignment order");
    expect_equal(assigned.get("B"), 2, "copy assignment retained key");
}

}  // namespace

int main() {
    try {
        test_given_example();
        test_update_and_miss();
        test_capacity_one_and_negative_value();
        test_invalid_capacity();
        test_randomized_against_reference();
        test_integer_keys();
        test_copy();
        std::cout << "All LRU cache tests passed.\n";
    } catch (const std::exception& error) {
        std::cerr << "Test failed: " << error.what() << '\n';
        return 1;
    }
}
