#pragma once

#include <iostream>
#include <vector>

#include "document.h"
#include "search_server.h"

template <typename T>
void AssertImpl(const T& t, const std::string& t_str, const std::string& file,
                const std::string& func, unsigned line, const std::string& hint) {
    using namespace std;
    if (!t) {
        cout << file << "("s << line << "): "s << func << ": "s;
        cout << "ASSERT("s << t_str << ") failed."s;
        if (!hint.empty()) {
            cout << " Hint: "s << hint;
        }
        cout << endl;
        abort();
    }
}

#define ASSERT(expr) AssertImpl((expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_HINT(expr, hint) AssertImpl((expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const std::string& t_str, const std::string& u_str, const std::string& file,
                     const std::string& func, unsigned line, const std::string& hint) {
    using namespace std;
    if (t != u) {
        cout << boolalpha;
        cout << file << "("s << line << "): "s << func << ": "s;
        cout << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
        cout << t << " != "s << u << "."s;
        if (!hint.empty()) {
            cout << " Hint: "s << hint;
        }
        cout << endl;
        abort();
    }
}

#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

void TestExcludeStopWordsFromAddedDocumentContent();

void TestMinusWordsSupport();

void TestMatchingDocuments();

void TestComputeRating();

void TestPredicateFilter();

void TestComputeRelevance();

void TestSearchServer();

template <typename FUNC>
void RunTestImpl(FUNC test, std::string text) {
    test();
    std::cerr << text << std::string(" OK") << std::endl;
}

#define RUN_TEST(func) RunTestImpl((func), #func)
