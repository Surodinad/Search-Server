#include "read_input_functions.h"

using namespace std;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<int> ReadNumbersLine() {
    vector<int> numbers;
    int counter, n;
    cin >> counter;
    for (int i = 0; i < counter; ++i) {
        cin >> n;
        numbers.push_back(n);
    }
    ReadLine();
    return numbers;
}
