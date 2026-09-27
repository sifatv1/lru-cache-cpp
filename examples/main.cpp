#include "cache.hpp"

#include <iostream>

int main() {
    Cache cache(2);
    cache.put("A", 10);
    cache.put("B", 20);

    std::cout << "get(A): " << cache.get("A") << '\n';
    cache.put("C", 30);
    std::cout << "get(B): " << cache.get("B") << '\n';
    std::cout << "get(C): " << cache.get("C") << '\n';
    std::cout << "get(A): " << cache.get("A") << '\n';
}
