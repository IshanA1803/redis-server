#ifndef REDIS_DATABASE_H
#define REDIS_DATABASE_H

#include <string>
#include <mutex>
#include <unordered_map>

class RedisDatabase {
public:
    static RedisDatabase& getInstance();

    void set(const std::string& key, const std::string& value);
    bool get(const std::string& key, std::string& value);

private:
    RedisDatabase() = default;
    ~RedisDatabase() = default;

    RedisDatabase(const RedisDatabase&) = delete;
    RedisDatabase& operator=(const RedisDatabase&) = delete;

    std::mutex db_mutex;
    std::unordered_map<std::string, std::string> kv_store;
};

#endif