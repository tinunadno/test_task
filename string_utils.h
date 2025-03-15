#ifndef TEST_TASK_STRING_UTILS_H
#define TEST_TASK_STRING_UTILS_H
#include <string>
using namespace std;
void strip(string &str) {
    if ((int) str.length() != 0) {
        auto w = string(" ");
        auto n = string("\n");
        auto r = string("\t");
        auto t = string("\r");
        auto v = string(1, str.front());
        while ((v == w) || (v == t) || (v == r) || (v == n)) {
            str.erase(str.begin());
            v = string(1, str.front());
        }
        v = string(1, str.back());
        while ((v == w) || (v == t) || (v == r) || (v == n)) {
            str.erase(str.end() - 1);
            v = string(1, str.back());
        }
    }
}
bool starts_with(string &str, const char* beg){
    return str.rfind(beg, 0) == 0;
}
#endif //TEST_TASK_STRING_UTILS_H
