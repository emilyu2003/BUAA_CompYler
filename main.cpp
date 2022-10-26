#include <iostream>
#include <cstring>
#include <cstdio>
#include <algorithm>
#include <vector>
#include <string>
#include <unordered_map>
#include<fstream>
#include "symbol.h"
#include "parseAndErr.h"
#include "base.h"

using namespace std;

string inputCode;
int readPos = 0, lexerPos = 0;
int inputLen, lexerLen = 0;
char line[500];
Lexer lexerOutput[1000005];

int main() {
    freopen("testfile.txt", "r", stdin);

    //clear file
    FILE *f = fopen("output.txt", "w");
    fclose(f);
    f = fopen("error.txt", "w");
    fclose(f);

    // read code
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

    // lexer
    initLexer();
    while (readPos < inputLen) {
        lexerOutput[lexerLen++] = lexer();
    }
    lexerLen--;

    // parser
    parseAndErr();

    return 0;
}
