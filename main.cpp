#include <cstring>
#include <cstdio>
#include <algorithm>
#include <vector>
#include <string>
#include <iostream>
#include "symbol.h"
#include "parseAndErr.h"
#include "parseAndSemant.h"
#include "base.h"
#include "TLRGen.h"

#define PRINT 0

using namespace std;

string inputCode;
int readPos = 0, lexerPos = 0;
int inputLen, lexerLen = 0;
char line[500];
Lexer lexerOutput[1000005];
bool correctFlag;
bool finishedParsing;

int main() {
    freopen("testfile.txt", "r", stdin);
    freopen("output.txt", "w", stdout);

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
    correctFlag = true;
    finishedParsing = false;
    parseAndErr();
    finishedParsing = true;
    if (correctFlag) {
        //printf("%s", parseAndSemant().c_str());
        parseAndSemant();
    } else {
        printf("plz check your syntax\n");
    }

    // generator
//    initGenerator();
//    mipsGen();
//
//    //f = fopen("mips.txt", "w");
//    for (auto & i : generatedCode) {
//        //fprintf(f, "%s\n", i.c_str());
//        cout << i << endl;
//    }
//    //fclose(f);

    return 0;
}
