#pragma once

#include <algorithm>
#include <cstdlib>
#include <future>
#include <map>
#include <numeric>
#include <string>
#include <vector>
#include <mutex>

using namespace std::string_literals;

template <typename Key, typename Value>
class ConcurrentMap {
public:
    static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys");

    struct Access {
        std::lock_guard<std::mutex> guard;
        Value& ref_to_value;
    };

    explicit ConcurrentMap(size_t bucket_count) :
            mutes_(bucket_count),
            concurrent_maps_(bucket_count)
    {}

    Access operator[](const Key& key) {
        uint64_t map_index = static_cast<uint64_t>(key) % concurrent_maps_.size();
        return { std::lock_guard<std::mutex>(mutes_[map_index]), concurrent_maps_[map_index][key] };
    }

    std::map<Key, Value> BuildOrdinaryMap() {
        std::map<Key, Value> result;
        size_t i = 0;

        for (const auto& single_map : concurrent_maps_) {
            std::lock_guard<std::mutex> guard(mutes_[i++]);
            std::for_each(single_map.begin(), single_map.end(),
                          [&result](const auto& pair) { result.insert(pair); });
        }

        return result;
    }

    void Erase(const Key& key) {
        uint64_t map_index = static_cast<uint64_t>(key) % concurrent_maps_.size();
        std::lock_guard<std::mutex> guard(mutes_[map_index]);
        concurrent_maps_[map_index].erase(key);
    }

private:
    std::vector<std::mutex> mutes_;
    std::vector<std::map<Key, Value>> concurrent_maps_;
};
