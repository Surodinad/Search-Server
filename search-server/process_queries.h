#pragma once

#include "search_server.h"

#include <cassert>

class DoubleVector {
public:
    class BasicIterator {
        friend class DoubleVector;

        explicit BasicIterator(std::vector<std::vector<Document>>::iterator cont) {
            cont_ = cont;
            elem_ = cont->begin();
            last_ = cont->end();
        }

    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = Document;
        using difference_type = std::ptrdiff_t;
        using pointer = Document*;
        using reference = Document&;

        BasicIterator() = delete;

        BasicIterator(const BasicIterator& other) noexcept {
            cont_ = other.cont_;
            elem_ = other.elem_;
            last_ = other.last_;
        }

        BasicIterator& operator=(const BasicIterator& rhs) = default;

        [[nodiscard]] bool operator==(const BasicIterator& rhs) const noexcept {
            return rhs.elem_ == elem_;
        }

        [[nodiscard]] bool operator!=(const BasicIterator& rhs) const noexcept {
            return rhs.elem_ != elem_;
        }

        BasicIterator& operator++() noexcept {
            elem_++;
            if (elem_ == last_) {
                cont_++;
                elem_ = cont_->begin();
                last_ = cont_->end();
            }
            return *this;
        }

        BasicIterator operator++(int) noexcept {
            auto old_value(*this);
            ++(*this);
            return old_value;
        }

        [[nodiscard]] reference operator*() const noexcept {
            return elem_.operator*();
        }

        [[nodiscard]] pointer operator->() const noexcept {
            return elem_.operator->();
        }

    private:
        std::vector<std::vector<Document>>::iterator cont_;
        std::vector<Document>::iterator elem_, last_;
    };

    DoubleVector() = delete;

    explicit DoubleVector(const std::vector<std::vector<Document>>& other) {
        values_ = other;
    }

    BasicIterator begin() noexcept {
        return BasicIterator(values_.begin());
    }

    BasicIterator end() noexcept {
        return BasicIterator(values_.end());
    }

private:
    std::vector<std::vector<Document>> values_;
};

std::vector<std::vector<Document>> ProcessQueries(
        const SearchServer& search_server,
        const std::vector<std::string>& queries);

DoubleVector ProcessQueriesJoined(
        const SearchServer& search_server,
        const std::vector<std::string>& queries);
