#pragma once

#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <cmath>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <execution>
#include <list>
#include <thread>

#include "read_input_functions.h"
#include "string_processing.h"
#include "document.h"
#include "concurrent_map.h"

const int MAX_RESULT_DOCUMENT_COUNT = 5;
const double EPSILON = 1e-6;

class SearchServer {

public:
    SearchServer() = default;

    SearchServer(const std::string& stop_words_text);

    SearchServer(std::string_view stop_words_text);

    template <typename StringContainer>
    explicit SearchServer(const StringContainer& stop_words);

    void AddDocument(int document_id, std::string_view document, DocumentStatus status, const std::vector<int>& ratings);

    template<typename Predicate>
    std::vector<Document> FindTopDocuments(std::string_view raw_query, Predicate predicate) const;
    std::vector<Document> FindTopDocuments(std::string_view raw_query, DocumentStatus selected_status) const;
    std::vector<Document> FindTopDocuments(std::string_view raw_query) const;

    template<typename ExecutionPolicy, typename Predicate>
    std::vector<Document> FindTopDocuments(const ExecutionPolicy& policy, std::string_view raw_query, Predicate predicate) const;
    template<typename ExecutionPolicy>
    std::vector<Document> FindTopDocuments(const ExecutionPolicy& policy, std::string_view raw_query, DocumentStatus selected_status) const;
    template<typename ExecutionPolicy>
    std::vector<Document> FindTopDocuments(const ExecutionPolicy&, std::string_view raw_query) const;

    size_t GetDocumentCount() const;

    std::set<int>::const_iterator begin() const;
    std::set<int>::const_iterator end() const;
    std::set<int>::iterator begin();
    std::set<int>::iterator end();

    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(std::string_view raw_query, int document_id) const;
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(const std::execution::sequenced_policy&, std::string_view raw_query, int document_id) const;
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(const std::execution::parallel_policy&, std::string_view raw_query, int document_id) const;

    const std::map<std::string_view, double>& GetWordFrequencies(int document_id) const;

    void RemoveDocument(int document_id);
    void RemoveDocument(const std::execution::sequenced_policy&, int document_id);
    void RemoveDocument(const std::execution::parallel_policy&, int document_id);

private:
    struct QueryWord {
        std::string_view data;
        bool is_minus;
        bool is_stop;
    };

    struct Query {
        std::vector<std::string_view> plus_words;
        std::vector<std::string_view> minus_words;
    };

    struct DocumentData {
        int rating;
        DocumentStatus status;
        std::string words;
    };

    std::map<std::string_view, std::map<int, double>> word_to_document_freqs_;
    std::set<std::string, std::less<>> stop_words_;
    std::map<int, DocumentData> documents_;
    std::set<int> ids_for_numbers_;
    std::map<int, std::map<std::string_view, double>> document_to_word_freqs_;

    static bool IsValidWord(std::string_view word);

    bool IsStopWord(std::string_view word) const;

    static int ComputeAverageRating(const std::vector<int>& ratings);

    std::vector<std::string_view> SplitIntoWordsNoStop(std::string_view text) const;

    QueryWord ParseQueryWord(std::string_view text) const;

    Query ParseQuery(std::string_view text, bool par_flag = false) const;

    double CalculateIDF(std::string_view word) const;

    template<typename ExecutionPolicy, typename Predicate>
    std::vector<Document> FindAllDocuments(const ExecutionPolicy& policy, const Query& query, Predicate predicate) const;

    template<typename Predicate>
    std::vector<Document> FindAllDocuments(const Query& query, Predicate predicate) const;
};

template <typename StringContainer>
SearchServer::SearchServer(const StringContainer& stop_words)
        : stop_words_(MakeUniqueNonEmptyStrings(stop_words))
{
    for (const std::string& word : stop_words_) {
        if (!IsValidWord(word)) {
            throw std::invalid_argument(std::string("стоп-слова содержат спецсимволы"));
        }
    }
}

template<typename ExecutionPolicy, typename Predicate>
std::vector<Document> SearchServer::FindTopDocuments(const ExecutionPolicy& policy, std::string_view raw_query, Predicate predicate) const {
    const Query query = ParseQuery(raw_query);
    auto matched_documents = FindAllDocuments(policy, query, predicate);

    sort(policy,
         matched_documents.begin(), matched_documents.end(),
         [](const Document& lhs, const Document& rhs) {
             if (std::abs(lhs.relevance - rhs.relevance) < EPSILON) {
                 return lhs.rating > rhs.rating;
             }
             else {
                 return lhs.relevance > rhs.relevance;
             }
         });

    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }
    return matched_documents;
}

template<typename ExecutionPolicy>
std::vector<Document> SearchServer::FindTopDocuments(const ExecutionPolicy& policy, std::string_view raw_query, DocumentStatus selected_status) const {
    return SearchServer::FindTopDocuments(policy, raw_query, [selected_status](int document_id, DocumentStatus status, int rating) { return status == selected_status; });
}

template<typename Predicate>
std::vector<Document> SearchServer::FindTopDocuments(std::string_view raw_query, Predicate predicate) const {
    return FindTopDocuments(std::execution::seq, raw_query, predicate);
}

template<typename ExecutionPolicy>
std::vector<Document> SearchServer::FindTopDocuments(const ExecutionPolicy& policy, std::string_view raw_query) const {
    return SearchServer::FindTopDocuments(policy, raw_query, DocumentStatus::ACTUAL);
}

template<typename ExecutionPolicy, typename Predicate>
std::vector<Document> SearchServer::FindAllDocuments(const ExecutionPolicy& policy, const Query& query, Predicate predicate) const {
    using namespace std;

    const size_t thread_count = std::thread::hardware_concurrency();
    ConcurrentMap<int, double> document_to_relevance(thread_count);

    for_each(policy,
             query.plus_words.begin(), query.plus_words.end(),
             [&](string_view word)
             {
                 if (!word_to_document_freqs_.count(word)) {
                     return;
                 }
                 const double inverse_document_freq = CalculateIDF(word);
                 for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
                     if ( predicate(document_id, documents_.at(document_id).status, documents_.at(document_id).rating) ) {
                         document_to_relevance[document_id].ref_to_value += term_freq * inverse_document_freq;
                     }
                 }
             });

    for_each(policy,
             query.minus_words.begin(), query.minus_words.end(),
             [&](string_view word)
             {
                 if (!word_to_document_freqs_.count(word)) {
                     return;
                 }
                 for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
                     document_to_relevance.Erase(document_id);
                 }
             });

    auto document_to_relevance_result = document_to_relevance.BuildOrdinaryMap();
    vector<Document> matched_documents;
    matched_documents.reserve(document_to_relevance_result.size());

    for (const auto [document_id, relevance] : document_to_relevance_result) {
        matched_documents.emplace_back(document_id, relevance, documents_.at(document_id).rating);
    }

    return matched_documents;
}

template<typename Predicate>
std::vector<Document> SearchServer::FindAllDocuments(const Query& query, Predicate predicate) const {
    return FindAllDocuments(std::execution::seq, query, predicate);
}
