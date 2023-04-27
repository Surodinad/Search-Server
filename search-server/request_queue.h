#pragma once

#include <iostream>
#include <vector>
#include <deque>

#include "document.h"
#include "search_server.h"

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server) : server_link_(search_server)
    {
    }

    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate);

    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);

    std::vector<Document> AddFindRequest(const std::string& raw_query);

    int GetNoResultRequests() const;

private:
    struct QueryResult {
        explicit QueryResult(const std::vector<Document>& RES);

        std::vector<Document> result;
        bool is_empty = false;
    };

    std::deque<QueryResult> requests_;
    const SearchServer& server_link_;

    const static int min_in_day_ = 1440;
};

template <typename DocumentPredicate>
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
    using namespace std;
    vector<Document> result = server_link_.FindTopDocuments(raw_query, document_predicate);
    requests_.push_back(QueryResult(result));
    if (requests_.size() > min_in_day_) {
        requests_.pop_front();
    }
    return result;
}
