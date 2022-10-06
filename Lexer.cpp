#include <cstdio>
#include <iostream>
#include <cstring>
#include <algorithm>
#include <cctype>
#include <vector>
#include <string>
#include <unordered_map>
#include "symbol.h"
#include "base.h"

using namespace std;

int lineNum;
unordered_map<string, int> Ident;
unordered_map<string, int> FormatString;
int printFlag = 1;

string symbol[40] = {
        "IDENFR", "INTCON", "STRCON", "MAINTK", "CONSTTK", "INTTK", "BREAKTK",
        "CONTINUETK", "IFTK", "ELSETK", "NOT", "AND", "OR", "WHILETK",
        "GETINTTK", "PRINTFTK", "RETURNTK", "PLUS", "MINU", "VOIDTK", "MULT",
        "DIV", "MOD", "LSS", "LEQ", "GRE", "GEQ", "EQL",
        "NEQ", "ASSIGN", "SEMICN", "COMMA", "LPARENT", "RPARENT", "LBRACK",
        "RBRACK", "LBRACE", "RBRACE"
};


void initLexer() {
    lineNum = 0;
    FormatString["main"] = MAINTK;
    FormatString["const"] = CONSTTK;
    FormatString["int"] = INTTK;
    FormatString["break"] = BREAKTK;
    FormatString["continue"] = CONTINUETK;
    FormatString["if"] = IFTK;
    FormatString["else"] = ELSETK;
    FormatString["while"] = WHILETK;
    FormatString["getint"] = GETINTTK;
    FormatString["printf"] = PRINTFTK;
    FormatString["return"] = RETURNTK;
    FormatString["void"] = VOIDTK;

    FormatString["!"] = NOT;
    FormatString["&&"] = AND;
    FormatString["||"] = OR;
    FormatString["+"] = PLUS;
    FormatString["-"] = MINU;
    FormatString["*"] = MULT;
    FormatString["/"] = DIV;
    FormatString["%"] = MOD;
    FormatString["<"] = LSS;
    FormatString["<="] = LEQ;
    FormatString[">"] = GRE;
    FormatString[">="] = GEQ;
    FormatString["=="] = EQL;
    FormatString["!="] = NEQ;
    FormatString["="] = ASSIGN;
    FormatString[";"] = SEMICN;
    FormatString[","] = COMMA;
    FormatString["("] = LPARENT;
    FormatString[")"] = RPARENT;
    FormatString["["] = LBRACK;
    FormatString["]"] = RBRACK;
    FormatString["{"] = LBRACE;
    FormatString["}"] = RBRACE;
}

int peek() {
    int tmpPos = readPos;
    int tmpLine = lineNum;
    printFlag = 0;
    int ans = lexer();
    printFlag = 1;
    readPos = tmpPos;
    lineNum = tmpLine;
    return ans;
}

int peeeek() {
    int tmpPos = readPos;
    int tmpLine = lineNum;
    int ans;
    printFlag = 0;
    ans = lexer();
    ans = lexer();
    printFlag = 1;
    readPos = tmpPos;
    lineNum = tmpLine;
    return ans;
}

int lexer() {
    int type = 0;
    string str;

    // read blank lines
    while (readPos < inputLen && (inputCode[readPos] == '\n' || inputCode[readPos] == ' ')) {
        readPos++;
        if (inputCode[readPos] == '\n') {
            lineNum++;
        }
    }
    // extract // and /**/
    while (inputCode[readPos] == '/') {
        if (readPos + 1 == inputLen) {
            return -1;
        }
        if (inputCode[readPos + 1] == '/') {
            while (readPos < inputLen && inputCode[readPos] != '\n') {
                readPos++;
            }
            if (inputCode[readPos] == '\n') {
                readPos++;
            } else {
                break;
            }
        } else if (inputCode[readPos + 1] == '*') {
            readPos += 2;
            while (readPos + 1 < inputLen &&
                   !(inputCode[readPos] == '*' && inputCode[readPos + 1] == '/')) {
                readPos++;
            }
            if (readPos + 1 < inputLen &&
                inputCode[readPos] == '*' && inputCode[readPos + 1] == '/') {
                readPos += 2;
            } else {
                return -1;
            }
        } else {
            break;
        }
        // read blank lines
        while (readPos < inputLen && (inputCode[readPos] == '\n' || inputCode[readPos] == ' ')) {
            readPos++;
            if (inputCode[readPos] == '\n') {
                lineNum++;
            }
        }
    }

    // extract ""
    if (inputCode[readPos] == '"') {
        str += inputCode[readPos];
        readPos++;
        while (readPos < inputLen && inputCode[readPos] != '"') {
            str += inputCode[readPos];
            readPos++;
        }
        if (readPos >= inputLen || inputCode[readPos] != '"') {
            return -1;
        }
        str += inputCode[readPos];
        type = STRCON;
    }

        // extract string
    else if (isalpha(inputCode[readPos]) || inputCode[readPos] == '_') {
        while (readPos < inputLen && (isalnum(inputCode[readPos]) || inputCode[readPos] == '_')) {
            str += inputCode[readPos];
            readPos++;
        }
        readPos--;
        type = IDENFR;
    }

        // extract num
    else if (isdigit(inputCode[readPos])) {
        while (readPos < inputLen && isdigit(inputCode[readPos])) {
            str += inputCode[readPos];
            readPos++;
        }
        readPos--;
        type = INTCON;
    }

        // extract ! and != / < and <= / > and >= / = and ==
    else if (inputCode[readPos] == '<' || inputCode[readPos] == '>' ||
             inputCode[readPos] == '=' || inputCode[readPos] == '!') {
        str += inputCode[readPos];
        if (readPos + 1 < inputLen && inputCode[readPos + 1] == '=') {
            str += '=';
            readPos++;
        }
    }

        // extract || / &&
    else if (inputCode[readPos] == '|' || inputCode[readPos] == '&') {
        str += inputCode[readPos];
        if (readPos + 1 < inputLen && inputCode[readPos + 1] == str[0]) {
            str += inputCode[readPos + 1];
        } else {
            return -1;
        }
        readPos++;
    }

    // extract + - * / % ; , ( ) [ ] { }
    char c = inputCode[readPos];
    if (c == '+' || c == '-' || c == '*' || c == '/' ||
        c == '%' || c == ';' || c == ',' ||
        c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}') {
        str = c;
    }

    readPos++;
    // print and return
    if (FormatString.count(str)) {
        type = FormatString[str];
    } else if (type == 0) {
        return -1;
    }
    if (printFlag) printSymbol(type, str);
    return type;
}

void printSymbol(int type, string str) {
    cout << symbol[type - 1] << " " << str << endl;
}