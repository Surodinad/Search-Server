#include "test_functions.h"

using namespace std;

void AddDocument(SearchServer& server, int document_id, const string& document, DocumentStatus status, const vector<int>& ratings) {
    server.AddDocument(document_id, document, status, ratings);
}