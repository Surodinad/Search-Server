#include "document.h"

using namespace std;

Document::Document(int ID, double REL, int RAT) :
        id(ID),
        relevance(REL),
        rating(RAT)
{
}

void PrintMatchDocumentResult(int document_id, const vector<string>& words, DocumentStatus status) {
    cout << "{ "s
         << "document_id = "s << document_id << ", "s
         << "status = "s << static_cast<int>(status) << ", "s
         << "words ="s;
    for (const string& word : words) {
        cout << ' ' << word;
    }
    cout << "}"s << endl;
}

ostream& operator<<(ostream& os, const Document& document) {
    os << "{ document_id = "s << document.id
    << ", relevance = "s << document.relevance
    << ", rating = "s << document.rating << " }"s;
    return os;
}
