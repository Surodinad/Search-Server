#pragma once

#include <iostream>
#include <vector>

#include "document.h"

template <typename Iterator>
class IteratorRange {
public:
    IteratorRange(Iterator begin, Iterator end) {
        range_.first = begin;
        range_.second = end;
    }

    auto begin() const {
        return range_.first;
    }
    auto end() const {
        return range_.second;
    }
    std::size_t size() {
        return end() - begin();
    }

private:
    std::pair<Iterator, Iterator> range_;
};

template <typename Iterator>
class Paginator {
public:
    Paginator(Iterator begin, Iterator end, size_t page_size) {
        if (page_size < 1) {
            throw std::invalid_argument(std::string("некорректный размер страницы"));
        }
        for (auto it = begin; distance(begin, it) < distance(begin, end); it += page_size) {
            if (end - it < page_size) {
                pages_.push_back({it, end});
            }
            else {
                pages_.push_back({it, it + page_size});
            }
        }
    }

    auto begin() const {
        return pages_.begin();
    }

    auto end() const {
        return pages_.end();
    }

    size_t size() {
        return end() - begin();
    }

private:
    std::vector<IteratorRange<Iterator>> pages_;
};

template <typename Iterator>
std::ostream& operator<<(std::ostream& os, const IteratorRange<Iterator>& range) {
    for (auto it = range.begin(); it != range.end(); ++it) {
        os << *it;
    }
    return os;
}

template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}
