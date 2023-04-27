#include "search_server.h"

using namespace std;

SearchServer::SearchServer(const string& stop_words_text)
        : SearchServer(SplitIntoWordsView(stop_words_text))
{
}

SearchServer::SearchServer(string_view stop_words_text)
        : SearchServer(SplitIntoWordsView(stop_words_text))
{
}

void SearchServer::AddDocument(int document_id, string_view document, DocumentStatus status, const vector<int>& ratings) {
    if ( documents_.count(document_id) ) {
        throw invalid_argument("документ не был добавлен, так как его id совпадает с уже имеющимся"s);
    }
    else if ( document_id < 0 ) {
        throw invalid_argument("документ не был добавлен, так как его id отрицательный"s);
    }
    else if ( !IsValidWord(document) ) {
        throw invalid_argument("документ не был добавлен, так как содержит спецсимволы"s);
    }
    else {
        documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status, string(document)});
        const vector<string_view> words = SplitIntoWordsNoStop(documents_.at(document_id).words);
        const double word_weight = 1.0 / words.size();
        for (const string_view & word : words) {
            word_to_document_freqs_[word][document_id] += word_weight;
            document_to_word_freqs_[document_id][word] += word_weight;
        }
        ids_for_numbers_.insert(document_id);
    }
}

vector<Document> SearchServer::FindTopDocuments(string_view raw_query, DocumentStatus selected_status) const {
    return SearchServer::FindTopDocuments(raw_query, [selected_status](int document_id, DocumentStatus status, int rating) { return status == selected_status; });
}

vector<Document> SearchServer::FindTopDocuments(string_view raw_query) const {
    return SearchServer::FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

size_t SearchServer::GetDocumentCount() const {
    return documents_.size();
}

std::set<int>::const_iterator SearchServer::begin() const
{
    return ids_for_numbers_.begin();
}

std::set<int>::const_iterator SearchServer::end() const
{
    return ids_for_numbers_.end();
}

std::set<int>::iterator SearchServer::begin()
{
    return ids_for_numbers_.begin();
}

std::set<int>::iterator SearchServer::end()
{
    return ids_for_numbers_.end();
}

std::tuple<std::vector<string_view>, DocumentStatus> SearchServer::MatchDocument(string_view raw_query, int document_id) const {
    Query query = ParseQuery(raw_query);
    std::vector<string_view> words;
    words.reserve(query.plus_words.size());
    for (const string_view& word : query.minus_words) {
        if (word_to_document_freqs_.count(word) && word_to_document_freqs_.at(word).count(document_id)) {
            return {words, documents_.at(document_id).status};
        }
    }
    for (const string_view& word : query.plus_words) {
        if (word_to_document_freqs_.count(word))
            if (word_to_document_freqs_.at(word).count(document_id) && count(words.begin(), words.end(), word) == 0) {
                words.push_back(word);
            }
    }

    return {words, documents_.at(document_id).status};
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(const std::execution::sequenced_policy&, string_view raw_query, int document_id) const {
    return SearchServer::MatchDocument(raw_query, document_id);
}

std::tuple<std::vector<string_view>, DocumentStatus> SearchServer::MatchDocument(const std::execution::parallel_policy&, string_view raw_query, int document_id) const {
    Query query = ParseQuery(raw_query, true);

    if (any_of(std::execution::par,
               query.minus_words.begin(), query.minus_words.end(),
               [this, document_id](const string_view& word) { return word_to_document_freqs_.count(word) && word_to_document_freqs_.at(word).count(document_id); })) {
        return {std::vector<string_view>({}), documents_.at(document_id).status};
    }

    std::vector<string_view> words(query.plus_words.size());

    auto it = copy_if(std::execution::par,
                      query.plus_words.begin(), query.plus_words.end(),
                      words.begin(),
                      [document_id, this](const string_view& word) { return word_to_document_freqs_.count(word) && word_to_document_freqs_.at(word).count(document_id); });

    sort(words.begin(), it);
    words.erase(unique(words.begin(), it), words.end());

    return {words, documents_.at(document_id).status};
}

const map<string_view, double>& SearchServer::GetWordFrequencies(int document_id) const {
    static map<string_view, double> empty_res;
    map<string, double> res;
    if (document_to_word_freqs_.count(document_id)) {
        return document_to_word_freqs_.at(document_id);
    }
    return empty_res;
}

void SearchServer::RemoveDocument(int document_id) {
    for_each(word_to_document_freqs_.begin(), word_to_document_freqs_.end(),
             [document_id, this](const auto& word) { word_to_document_freqs_.at(word.first).erase(document_id); });

    document_to_word_freqs_.erase(document_id);
    ids_for_numbers_.erase(document_id);
    documents_.erase(document_id);
}

void SearchServer::RemoveDocument(const std::execution::sequenced_policy&, int document_id) {
    SearchServer::RemoveDocument(document_id);
}

void SearchServer::RemoveDocument(const std::execution::parallel_policy&, int document_id) {
    std::vector<const std::string_view*> tmp(document_to_word_freqs_.at(document_id).size());

    transform(std::execution::par,
              document_to_word_freqs_.at(document_id).begin(),
              document_to_word_freqs_.at(document_id).end(),
              tmp.begin(),
              [](const auto& str_fr) { return &(str_fr.first); });

    for_each(std::execution::par,
             tmp.begin(), tmp.end(),
             [document_id, this](const std::string_view* word) { word_to_document_freqs_.at(*word).erase(document_id); });

    document_to_word_freqs_.erase(document_id);
    ids_for_numbers_.erase(document_id);
    documents_.erase(document_id);
}

bool SearchServer::IsValidWord(string_view word) {
    // A valid word must not contain special characters
    return none_of(word.begin(), word.end(), [](char c) {
        return c >= '\0' && c < ' ';
    });
}

bool SearchServer::IsStopWord(string_view word) const {
    return stop_words_.count(string(word)) > 0;
}

int SearchServer::ComputeAverageRating(const vector<int>& ratings) {
    if (!ratings.empty()) {
        int size = static_cast<int>(ratings.size());
        return accumulate(ratings.begin(), ratings.end(), 0) / size;
    }

    return 0;
}

vector<string_view> SearchServer::SplitIntoWordsNoStop(string_view text) const { //Разделение строки на слова
    vector<string_view> words;
    for (const auto& word : SplitIntoWordsView(text)) {
        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    return words;
}

SearchServer::QueryWord SearchServer::ParseQueryWord(string_view text) const {
    bool is_minus = false;
    if (text[0] == '-') {
        is_minus = true;
        text.remove_prefix(1);
    }

    if ( text.empty() || text[0] == '-' )
        throw invalid_argument("ошибка в поисковом запросе"s);

    return {text, is_minus, IsStopWord(text)};
}

SearchServer::Query SearchServer::ParseQuery(string_view text, bool par_flag) const {
    if (!IsValidWord(text)) {
        throw invalid_argument("ошибка в поисковом запросе"s);
    }

    Query query;

    for (string_view word : SplitIntoWordsView(text)) {
        const QueryWord query_word = SearchServer::ParseQueryWord(word);
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                query.minus_words.push_back(query_word.data);
            } else {
                query.plus_words.push_back(query_word.data);
            }
        }
    }

    if (!par_flag) {
        sort(query.minus_words.begin(), query.minus_words.end());
        query.minus_words.erase(unique(query.minus_words.begin(), query.minus_words.end()), query.minus_words.end());
        sort(query.plus_words.begin(), query.plus_words.end());
        query.plus_words.erase(unique(query.plus_words.begin(), query.plus_words.end()), query.plus_words.end());
    }

    return query;
}

double SearchServer::CalculateIDF(string_view word) const {
    return log( GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size() );
}
