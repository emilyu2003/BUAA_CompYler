#include <iostream>
#include <cstring>
#include <cstdio>
#include <algorithm>
#include <vector>
#include <string>
#include <unordered_map>
#include<fstream>
#include "symbol.h"
#include "parse.h"
#include "base.h"

using namespace std;

string inputCode;
int readPos = 0, lexerPos = 0;
int inputLen, lexerLen = 0;
char line[500];
Lexer lexerOutput[1000005];

int main() {
    freopen("testfile.txt", "r", stdin);
    freopen("output.txt", "w", stdout);

    string tmp;
    while (fgets(line, 500, stdin) != NULL) {
        tmp.clear();
        for (int i = 0; i < strlen(line); i++) {
            tmp += line[i];
        }
        tmp.erase(std::remove(tmp.begin(), tmp.end(), '\r'), tmp.end());
        inputCode += tmp;
    }
    inputLen = inputCode.size();
    initLexer();
    while (readPos < inputLen) {
        lexerOutput[lexerLen++] = lexer();
    }
    lexerLen--;
    parse();

    return 0;
}
