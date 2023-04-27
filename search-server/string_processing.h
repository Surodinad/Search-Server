#pragma once

#include <iostream>
#include <vector>
#include <set>

std::vector<std::string> SplitIntoWords(const std::string& text);

std::vector<std::string_view> SplitIntoWordsView(std::string_view text);

template <typename StringContainer>
std::set<std::string, std::less<>> MakeUniqueNonEmptyStrings(const StringContainer& strings) {
    using namespace std;
    std::set<string, std::less<>> non_empty_strings;
    for (const auto& str : strings) {
        if (!str.empty()) {
            string tmp(str);
            non_empty_strings.insert(tmp);
        }
    }
    return non_empty_strings;
}
