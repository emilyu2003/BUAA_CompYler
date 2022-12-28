#include <cstdio>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <vector>
#include <string>
#include <unordered_map>
#include "symbol.h"
#include "base.h"

using namespace std;

int lineNum;
unordered_map<string, int> FormatString;

string symbol[40] = {
        "IDENFR", "INTCON", "STRCON", "MAINTK", "CONSTTK", "INTTK", "BREAKTK",
        "CONTINUETK", "IFTK", "ELSETK", "NOT", "AND", "OR", "WHILETK",
        "GETINTTK", "PRINTFTK", "RETURNTK", "PLUS", "MINU", "VOIDTK", "MULT",
        "DIV", "MOD", "LSS", "LEQ", "GRE", "GEQ", "EQL",
        "NEQ", "ASSIGN", "SEMICN", "COMMA", "LPARENT", "RPARENT", "LBRACK",
        "RBRACK", "LBRACE", "RBRACE", "BITAND"
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
    FormatString["bitand"] = BITAND;

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
    return lexerOutput[lexerPos].type;
}

int peeeek() {
    return lexerOutput[lexerPos + 1].type;
}

int getSym() {
    //printf("%d : ", lexerOutput[lexerPos].line);
    if (!finishedParsing) {
        //printSymbol(lexerOutput[lexerPos].type, lexerOutput[lexerPos].str);
    }
    return lexerOutput[lexerPos++].type;
}

int getErrorLine() {
    return lexerOutput[lexerPos - 1].line;
}

string getStr() {
    return lexerOutput[lexerPos - 1].str;
}

int getInt() {
    return stoi(lexerOutput[lexerPos - 1].str);
}

int isLVal() {
    int flag = 0;
    int last = -1;
    for (int i = 0;; i++) {
        int k = lexerPos + i;
        if (k > lexerLen) break;
        if (lexerOutput[k].type == ASSIGN) {
            flag = 1;
        }
        if (lexerOutput[k].type == SEMICN) {
            break;
        }
        if ((last == INTCON || last == IDENFR) &&
            (lexerOutput[k].type == INTCON || lexerOutput[k].type == IDENFR)) {
            return false;
        }
        last = lexerOutput[k].type;
    }
    return flag;
}

Lexer lexer() {
    int type = 0;
    string str;

    // read blank lines
    while (readPos < inputLen &&
           (inputCode[readPos] == '\n' || inputCode[readPos] == ' ' || inputCode[readPos] == '\t')) {
        if (inputCode[readPos] == '\n') {
            lineNum++;
        }
        readPos++;
    }
    // extract // and /**/
    while (inputCode[readPos] == '/') {
        if (readPos + 1 == inputLen) {
            // return -1;
        }
        if (inputCode[readPos + 1] == '/') {
            while (readPos < inputLen && inputCode[readPos] != '\n') {
                readPos++;
            }
            if (inputCode[readPos] == '\n') {
                lineNum++;
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
                // return -1;
            }
        } else {
            break;
        }
        // read blank lines
        while (readPos < inputLen &&
               (inputCode[readPos] == '\n' || inputCode[readPos] == ' ' || inputCode[readPos] == '\t')) {
            if (inputCode[readPos] == '\n') {
                lineNum++;
            }
            readPos++;
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
            // return -1;
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
            // return -1;
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
        /// return -1;
    }
    // printSymbol(type, str);
    Lexer tmpLexer = {type, str, lineNum};
    return tmpLexer;
}

void printSymbol(int type, string str) {
    string output = symbol[type - 1] + " " + str + "\n";
//    FILE *f = fopen("output.txt", "a");
//    fprintf(f, "%s", output.c_str());
//    fclose(f);
    cout << output;
}