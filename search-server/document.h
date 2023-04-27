#pragma once

#include <iostream>
#include <vector>

enum class DocumentStatus { // Оставить определение этого типа здесь?
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

struct Document {
    Document() = default;

    Document(int ID, double REL, int RAT);

    int id = 0;
    double relevance = 0.0;
    int rating = 0;
};

void PrintMatchDocumentResult(int document_id, const std::vector<std::string>& words, DocumentStatus status);

std::ostream& operator<<(std::ostream& os, const Document& document);
