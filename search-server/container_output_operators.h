#pragma once

#include <iostream>
#include <vector>
#include <set>
#include <map>

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& container) {
    using namespace std;
    if (!container.empty()) {
        bool is_first = true;
        os << '[';
        for (const auto& elem : container) {
            if (is_first) {
                os << elem;
            }
            else {
                os << ", "s << elem;
            }
            is_first = false;
        }
        os << ']';
    }
    return os;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::set<T>& container) {
    using namespace std;
    if (!container.empty()) {
        bool is_first = true;
        os << '{';
        for (const auto& elem : container) {
            if (is_first) {
                os << elem;
            }
            else {
                os << ", "s << elem;
            }
            is_first = false;
        }
        os << '}';
    }
    return os;
}

template <typename T, typename U>
std::ostream& operator<<(std::ostream& os, const std::map<T, U>& container) {
    using namespace std;
    if (!container.empty()) {
        bool is_first = true;
        os << '{';
        for (const auto& [key, value] : container) {
            if (is_first) {
                os << key << ": "s << value;
            }
            else {
                os << ", "s << key << ": "s << value;
            }
            is_first = false;
        }
        os << '}';
    }
    return os;
}
