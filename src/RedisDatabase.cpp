#include "../include/RedisDatabase.h"
#include <algorithm>
// Singleton accessor
RedisDatabase& RedisDatabase::getInstance() {
    static RedisDatabase instance;
    return instance;
}

void RedisDatabase::set(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(db_mutex);
    kv_store[key] = value;
}

bool RedisDatabase::get(const std::string& key, std::string& value) {
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = kv_store.find(key);
    if (it != kv_store.end()) {
        value = it->second;
        return true;
    }
    return false;
}

std::vector<std::string> RedisDatabase::keys() {
    std::lock_guard<std::mutex> lock(db_mutex);
    std::vector<std::string> result;
    for (const auto& pair : kv_store) {
        result.push_back(pair.first);
    }
    return result;
}

std::string RedisDatabase::type(const std::string& key) {
    std::lock_guard<std::mutex> lock(db_mutex);
    if (kv_store.find(key) != kv_store.end())
        return "string";
    return "none";
}

bool RedisDatabase::del(const std::string& key) {
    std::lock_guard<std::mutex> lock(db_mutex);
    return kv_store.erase(key) > 0;
}

bool RedisDatabase::flushAll() {
    std::lock_guard<std::mutex> lock(db_mutex);
    kv_store.clear();
    return true;
}

bool RedisDatabase::rename(const std::string& oldKey, const std::string& newKey) {
    std::lock_guard<std::mutex> lock(db_mutex);

    auto it = kv_store.find(oldKey);
    if (it == kv_store.end())
        return false;

    kv_store[newKey] = it->second;
    kv_store.erase(it);
    return true;
}

void RedisDatabase::lpush(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(db_mutex);
    list_store[key].insert(list_store[key].begin(), value);
}

void RedisDatabase::rpush(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(db_mutex);
    list_store[key].push_back(value);
}

ssize_t RedisDatabase::llen(const std::string& key) {
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = list_store.find(key);
    if (it != list_store.end())
        return it->second.size();
    return 0;
}

std::vector<std::string> RedisDatabase::lget(const std::string& key) {
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = list_store.find(key);
    if (it != list_store.end()) {
        return it->second;
    }
    return {};
}

bool RedisDatabase::lpop(const std::string& key, std::string& value) {
    std::lock_guard<std::mutex> lock(db_mutex);

    auto it = list_store.find(key);
    if (it != list_store.end() && !it->second.empty()) {
        value = it->second.front();
        it->second.erase(it->second.begin());
        return true;
    }

    return false;
}

bool RedisDatabase::rpop(const std::string& key, std::string& value) {
    std::lock_guard<std::mutex> lock(db_mutex);

    auto it = list_store.find(key);
    if (it != list_store.end() && !it->second.empty()) {
        value = it->second.back();
        it->second.pop_back();
        return true;
    }

    return false;
}

bool RedisDatabase::lindex(const std::string& key, int index, std::string& value) {
    std::lock_guard<std::mutex> lock(db_mutex);

    auto it = list_store.find(key);
    if (it == list_store.end())
        return false;

    const auto& lst = it->second;

    if (index < 0)
        index = lst.size() + index;

    if (index < 0 || index >= static_cast<int>(lst.size()))
        return false;

    value = lst[index];
    return true;
}

bool RedisDatabase::lset(const std::string& key, int index, const std::string& value) {
    std::lock_guard<std::mutex> lock(db_mutex);

    auto it = list_store.find(key);
    if (it == list_store.end())
        return false;

    auto& lst = it->second;

    if (index < 0)
        index = lst.size() + index;

    if (index < 0 || index >= static_cast<int>(lst.size()))
        return false;

    lst[index] = value;
    return true;
}

int RedisDatabase::lrem(const std::string& key, int count, const std::string& value) {
    std::lock_guard<std::mutex> lock(db_mutex);

    int removed = 0;
    auto it = list_store.find(key);
    if (it == list_store.end())
        return 0;

    auto& lst = it->second;

    if (count == 0) {
        // Remove all occurrences
        auto new_end = std::remove(lst.begin(), lst.end(), value);//remove only moves elements, doesn't actually erase them
        removed = std::distance(new_end, lst.end()); 
        lst.erase(new_end, lst.end());
    } else if (count > 0) {
        // Remove from head
        for (auto iter = lst.begin(); iter != lst.end() && removed < count; ) {
            if (*iter == value) {
                iter = lst.erase(iter); // erase returns the next iterator
                ++removed;
            } else {
                ++iter;
            }
        }
    } else {
        // Remove from tail
        for (auto riter = lst.rbegin(); riter != lst.rend() && removed < (-count); ) {
            if (*riter == value) {
                auto fwd = riter.base();
                --fwd; // base() returns the next element, so we need to step back to get the correct one
                fwd = lst.erase(fwd);
                ++removed;
                riter = std::reverse_iterator<std::vector<std::string>::iterator>(fwd);
            } else {
                ++riter;
            }
        }
    }

    return removed;
}