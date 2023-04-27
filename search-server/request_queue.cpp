#include "request_queue.h"

using namespace std;

vector<Document> RequestQueue::AddFindRequest(const string& raw_query, DocumentStatus status) {
    vector<Document> result = server_link_.FindTopDocuments(raw_query, status);
    requests_.push_back(QueryResult(result));
    if (requests_.size() > min_in_day_) {
        requests_.pop_front();
    }
    return result;
}

vector<Document> RequestQueue::AddFindRequest(const string& raw_query) {
    return AddFindRequest(raw_query, DocumentStatus::ACTUAL);
}

int RequestQueue::GetNoResultRequests() const {
    return count_if(requests_.begin(), requests_.end(), [](const QueryResult& i) {return i.is_empty;});
    // Тут не уверен, может лучше сделать счетчик отдельным полем класса и менять его, когда происходит добавление/удаление нового запроса?
}

RequestQueue::QueryResult::QueryResult(const vector<Document>& RES) : result(RES)
{
    if (result.empty()) {
        is_empty = true;
    }
}
