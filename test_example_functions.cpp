#include "test_example_functions.h"

using namespace std;

void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};

    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }

    {
        SearchServer server("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT(server.FindTopDocuments("in"s).empty());
    }
}

void TestMinusWordsSupport() {
    SearchServer server;
    server.AddDocument(40, "black cat in the street our"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
    server.AddDocument(41, "black dog the street"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
    const vector<Document> doc = server.FindTopDocuments("-dog street");
    ASSERT_EQUAL(doc.size(), 1);
    ASSERT_EQUAL(doc[0].id, 40);
}

void TestMatchingDocuments() {
    const int doc_id = 40;
    const string content = "black cat in the street"s;
    const vector<int> ratings = { 1, 2, 3 };

    SearchServer server;
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
    const vector<string> contentWords = { "black"s, "cat"s, "in"s, "the"s, "street"s };
    const vector<string_view> matchedWords = get<vector<string_view>>(server.MatchDocument(content, doc_id));
    ASSERT_EQUAL(matchedWords.size(), contentWords.size());

    const string queryWithMinusWords = "black cat in -the street"s;
    const vector<string_view> emptyListMatchedWords = get<vector<string_view>>(server.MatchDocument(queryWithMinusWords, doc_id));
    ASSERT(emptyListMatchedWords.empty());
}

void TestComputeRating() {
    SearchServer server;
    server.AddDocument(40, "black cat in the street"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
    const auto found_docs = server.FindTopDocuments("black cat"s);
    ASSERT_EQUAL(found_docs.size(), 1);
    ASSERT_EQUAL(found_docs[0].rating, (1 + 2 + 3) / 3);
}

void TestPredicateFilter() {
    SearchServer server;
    server.AddDocument(40, "black cat in the street on hood"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
    server.AddDocument(41, "black dog the black street"s,      DocumentStatus::ACTUAL, { 2, 4, 8 });
    server.AddDocument(42, "black bird on the roof"s,          DocumentStatus::BANNED, { 3, 6, 9 });

    const auto found_docs1 = server.FindTopDocuments("black cat street"s, [](int document_id, DocumentStatus status, int rating) { return status == DocumentStatus::ACTUAL; });
    const auto found_docs2 = server.FindTopDocuments("black cat street"s, [](int document_id, DocumentStatus status, int rating) { return rating > 2; });
    const auto found_docs3 = server.FindTopDocuments("black cat street"s, [](int document_id, DocumentStatus status, int rating) { return document_id != 41; });

    for (const auto& document : found_docs1) {
        ASSERT(document.id != 42);
    }
    for (const auto& document : found_docs2) {
        ASSERT(document.rating > 2);
    }
    for (const auto& document : found_docs3) {
        ASSERT(document.id != 41);
    }
}

void TestComputeRelevance() {
    SearchServer server;
    server.AddDocument(40, "black cat in the street on hood"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
    server.AddDocument(41, "black dog the black street"s,      DocumentStatus::ACTUAL, { 1, 2, 3 });
    server.AddDocument(42, "black bird on the roof"s,          DocumentStatus::ACTUAL, { 1, 2, 3 });
    const auto found_docs = server.FindTopDocuments("black cat street"s);
    ASSERT_EQUAL(found_docs[0].relevance, log(3.0 / 3.0) * (1.0 / 7.0) + log(3.0 / 1.0) * (1.0 / 7.0) + log(3.0 / 2.0) * (1.0 / 7.0));
    ASSERT_EQUAL(found_docs[1].relevance, log(3.0 / 3.0) * (2.0 / 5.0) + log(3.0 / 1.0) * (0.0 / 5.0) + log(3.0 / 2.0) * (1.0 / 5.0));
    ASSERT_EQUAL(found_docs[2].relevance, log(3.0 / 3.0) * (1.0 / 5.0) + log(3.0 / 1.0) * (0.0 / 5.0) + log(3.0 / 2.0) * (0.0 / 5.0));
}

void TestSearchServer() {
    TestExcludeStopWordsFromAddedDocumentContent();
    TestMinusWordsSupport();
    TestMatchingDocuments();
    TestComputeRating();
    TestComputeRelevance();
    TestPredicateFilter();
}
