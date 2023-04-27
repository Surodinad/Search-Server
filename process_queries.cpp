#include "process_queries.h"

#include <vector>
#include <string>
#include <algorithm>
#include <execution>

std::vector<std::vector<Document>> ProcessQueries(
        const SearchServer& search_server,
        const std::vector<std::string>& queries) {
    std::vector<std::vector<Document>> result(queries.size());
    std::transform(std::execution::par,
                   queries.begin(), queries.end(),
                   result.begin(),
                   [&search_server](std::string query) { return search_server.FindTopDocuments(query); });
    return result;
}

DoubleVector ProcessQueriesJoined(
        const SearchServer& search_server,
        const std::vector<std::string>& queries) {
    DoubleVector documents(ProcessQueries(search_server, queries));
    return documents;
}
