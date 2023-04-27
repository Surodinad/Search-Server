/*
#include "query_functions.h"

using namespace std;

void MatchDocuments(const SearchServer& server_link, const string& query) {

    LOG_DURATION_STREAM("Operation time", cout);

    cout << "Матчинг документов по запросу: "s << query << endl;
    const int document_count = server_link.GetDocumentCount();
    for (int document_num = 0; document_num < document_count; ++document_num) {
        int document_id = server_link.GetDocumentId(document_num);
        const auto [words, status] = server_link.MatchDocument(query, document_id);
        PrintMatchDocumentResult(document_id, words, status);
    }
}

void FindTopDocuments(const SearchServer& server_link, const string& query) {

    LOG_DURATION_STREAM("Operation time", cout);

    cout << "Результаты поиска по запросу: "s << query << endl;
    auto pages = Paginate(server_link.FindTopDocuments(query), 1);
    for (auto page = pages.begin(); page != pages.end(); ++page) {
        cout << *page << endl;
    }
}
 */