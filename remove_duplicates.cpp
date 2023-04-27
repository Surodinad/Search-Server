#include "remove_duplicates.h"

using namespace std;

void RemoveDuplicates(SearchServer& search_server) {
    set<set<string_view>> documents_words;
    set<string_view> buf;
    vector<int> id_for_remove;
    id_for_remove.reserve(search_server.GetDocumentCount());

    for (int id : search_server) {
        for(const auto& words : search_server.GetWordFrequencies(id)) {
            buf.insert(words.first);
        }
        if (documents_words.count(buf)) {
            id_for_remove.push_back(id);
        }
        else {
            documents_words.insert(buf);
        }
        buf.clear();
    }

    for (int id : id_for_remove) {
        cout << "Found duplicate document id "s << id << endl;
        search_server.RemoveDocument(id);
    }
}