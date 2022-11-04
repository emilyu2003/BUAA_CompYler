//
// Created by emilyu on 2022/10/28.
//
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include "TLRGen.h"

using namespace std;

vector<string> middleCode;

void printMiddleCode() {
    for (auto s: middleCode) {
        cout << ">" << s << "<" << endl;
    }
}

void mipsGen() {
    for (auto s: middleCode) {
        if (s.empty()) continue;
        s.erase(std::remove(s.begin(), s.end(), '\n'), s.end());

        // exp
        if (s[0] == '+') {

        } else if (s[0] == '-') {

        } else if (s[0] == '*') {

        } else if (s[0] == '/') {

        } else if (s[0] == '%') {

        }
    }
}