#include "string_processing.h"

using namespace std;

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }
    return words;
}

std::vector<std::string_view> SplitIntoWordsView(std::string_view text) {
    vector<string_view> result;
    text.remove_prefix(min(text.size(), text.find_first_not_of(" ")));

    while (!text.empty()) {
        int64_t space = text.find(' ');
        result.push_back(text.substr(0, space));
        text.remove_prefix(min(text.size(), text.find_first_not_of(" ", space)));
    }

    return result;
}