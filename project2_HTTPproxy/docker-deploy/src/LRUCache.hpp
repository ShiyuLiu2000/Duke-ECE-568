#ifndef LRUCACHE_H
#define LRUCACHE_H

#include <unordered_map>
#include <list>
#include "response.hpp"

constexpr unsigned int CACHE_CAPACITY = 50;

template <typename KeyType, typename ValueType>
class LRUCache {
private:
    size_t capacity;
    std::unordered_map<KeyType, typename std::list<KeyType>::iterator> keyMap;
    std::list<KeyType> order;  // Maintain the order of keys based on their recent use
    std::unordered_map<KeyType, ValueType> cacheMap;

public:
    LRUCache() : capacity(CACHE_CAPACITY) {}

    ValueType get(const KeyType &key) {
        auto it = cacheMap.find(key);
        if (it != cacheMap.end()) {
            // Move the key to the front of the order list (most recently used)
            order.erase(keyMap[key]);
            order.push_front(key);
            keyMap[key] = order.begin();
            return it->second;
        }
        return ValueType();  // Return default value if key not found
    }

    void put(const KeyType &key, const ValueType &value) {
        auto it = cacheMap.find(key);
        if (it != cacheMap.end()) {
            // If key already exists, update the value and move it to the front
            order.erase(keyMap[key]);
            order.push_front(key);
            keyMap[key] = order.begin();
            it->second = value;
        } else {
            if (cacheMap.size() >= capacity) {
                // If the cache is full, remove the least recently used element
                KeyType lruKey = order.back();
                order.pop_back();
                keyMap.erase(lruKey);
                cacheMap.erase(lruKey);
            }
            // Add the new key-value pair to the front of the order list
            order.push_front(key);
            keyMap[key] = order.begin();
            cacheMap[key] = value;
        }
    }

    void remove(const KeyType &key) {
        auto it = cacheMap.find(key);
        if (it != cacheMap.end()) {
            // Erase the entry from the order list and update keyMap
            order.erase(keyMap[key]);
            keyMap.erase(key);
            // Erase the entry from the cache map
            cacheMap.erase(key);
        }
    }
    
    // Display the cache content
    void display() const {
        std::cout << "Displaying content of cache: " << std::endl;
        std::size_t i = 0;
        for (const KeyType & key : order) {
            std::cout << "-------start of item " << i << "-------" << std::endl;
            std::cout << "(" << key << ", " << cacheMap.at(key) << ") " << std::endl;
            std::cout << "-------end of item " << i << "-------" << std::endl;
            i++;
        }   
        std::cout << std::endl;
    }

    // Display the order list
    void displayOrder() const {
        std::cout << "Order: ";
        for (const KeyType &key : order) {
            std::cout << key << " ";
        }
        std::cout << std::endl;
    }
};
#endif
