#pragma once

#include <list>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

// LRU Cache using unordered_map + list
// front of list = most recently used, back = least recently used
// map stores key -> iterator so we can jump to that node in O(1)

class LRUCache
{
public:
    explicit LRUCache(int capacity) : capacity(capacity)
    {
        if (capacity <= 0)
            throw std::invalid_argument("Cache capacity must be positive");
    }

    // Stored iterators belong to this instance and must not be copied.
    LRUCache(const LRUCache &) = delete;
    LRUCache &operator=(const LRUCache &) = delete;

    // returns true if found, fills value with cached result
    bool get(const std::string &key, std::string &value)
    {
        auto it = lookup.find(key);

        if (it == lookup.end())
            return false;

        // move to front since this was just used
        items.splice(items.begin(), items, it->second);

        value = it->second->second;
        return true;
    }

    void put(const std::string &key, const std::string &value)
    {
        auto it = lookup.find(key);

        if (it != lookup.end())
        {
            it->second->second = value;
            items.splice(items.begin(), items, it->second);
            return;
        }

        // cache full -> remove least recently used (back of list)
        if ((int)items.size() >= capacity)
        {
            auto last = items.back();
            lookup.erase(last.first);
            items.pop_back();
        }

        items.push_front({key, value});
        lookup[key] = items.begin();
    }

private:
    int capacity;
    std::list<std::pair<std::string, std::string>> items;
    std::unordered_map<std::string, std::list<std::pair<std::string, std::string>>::iterator> lookup;
};
