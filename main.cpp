#include <cstring>
#include <cstdio>
#include <algorithm>
#include <vector>
#include <string>
#include "symbol.h"
#include "parseAndErr.h"
#include "parseAndSemant.h"
#include "base.h"
#include "TLRGen.h"

using namespace std;

string inputCode;
int readPos = 0, lexerPos = 0;
int inputLen, lexerLen = 0;
char line[500];
Lexer lexerOutput[1000005];
bool correctFlag;

int main() {
    //freopen("testfile.txt", "r", stdin);

    //clear file
    FILE *f = fopen("output.txt", "w");
    fclose(f);
    f = fopen("error.txt", "w");
    fclose(f);
    f = fopen("test.txt", "w");
    fclose(f);
    f = fopen("mips.txt", "w");
    fclose(f);

    f = fopen("testfile.txt", "r");
    // read code
    string tmp;
    while (fgets(line, 500, f) != NULL) {
        tmp.clear();
        for (int i = 0; i < strlen(line); i++) {
            tmp += line[i];
        }
        tmp.erase(std::remove(tmp.begin(), tmp.end(), '\r'), tmp.end());
        inputCode += tmp;
    }
    inputLen = inputCode.size();
    fclose(f);

    // lexer
    initLexer();
    while (readPos < inputLen) {
        lexerOutput[lexerLen++] = lexer();
    }
    lexerLen--;

    // parser
    correctFlag = true;
    parseAndErr();
    if (correctFlag) {
        printf("%s", parseAndSemant().c_str());
    } else {
        printf("plz check your syntax\n");
    }

    // generator
    initGenerator();
    printMiddleCode();
    mipsGen();

    return 0;
}
