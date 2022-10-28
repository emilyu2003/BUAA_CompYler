//
// Created by emilyu on 2022/10/26.
//
#include <cstdio>
#include <algorithm>
#include <vector>
#include <string>
#include <iostream>
#include<sstream>
#include "MLRGen.h"

using namespace std;

vector<string> poNo;
vector<string> utils;

void printCode(const string &toFile, const string &format, const string &str) {
    FILE *f = fopen(toFile.c_str(), "a");
    fprintf(f, format.c_str(), str.c_str());
    fclose(f);
}

void genString(string str) {
    printCode("test.txt", "%s\n", str);
}

void getVarName(string str) {
    int dim = 0;
    for (char i: str) {
        if (str[i] == '[') {
            dim++;
        }
    }
}

string genExpCode(string str) {
    poNo.clear();
    str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());

    int tCnt = -1;
    string res, mCode;
    stringstream input;
    input << str;
    while (input >> res) {
        if (isCalSym(res)) {
            string a2 = poNo.back();
            poNo.pop_back();
            string a1 = poNo.back();
            poNo.pop_back();

            if (isNum(a1) && isNum(a2) && (res != "||" && res != "&&")) {
                int ansNum = calculate(a1, a2, res);
                poNo.push_back(to_string(ansNum));
            } else {
                tCnt++;
                poNo.push_back("t" + to_string(tCnt));
                mCode.append(res + " ").append(a1 + " ").append(a2 + " ")
                        .append("t" + to_string(tCnt)).append("\n");
            }
        } else {
            poNo.push_back(res);
        }
    }

    printCode("test.txt", "%s", mCode);
    if (!poNo.empty()) {
        return poNo.back();
    }
    return "t" + to_string(tCnt);
}

void genVarCode(string str) {
    printCode("test.txt", "VAR int %s\n", str);
}

void genConstCode(string str) {
    printCode("test.txt", "CONST int %s\n", str);
}

void genAssignCode(string lval, string exp, int dim) {
    if (dim == 0) {
        string tt = genExpCode(exp);
        FILE *f = fopen("test.txt", "a");
        fprintf(f, "%s = %s\n", lval.c_str(), tt.c_str());
        fclose(f);
    } else {
        exp.erase(std::remove(exp.begin(), exp.end(), '\n'), exp.end());
        int pos = 1;
        while (pos < exp.size() && (exp[pos] == '{' || exp[pos] == ' ')) pos++;
        string tt;
        int rCnt = -1;
        for (int i = 0; i < exp.size(); i++) {
            if (exp[i] == '{') continue;
            if (i > pos && (exp[i] == ',' || exp[i] == '}')) {
                rCnt++;
                string tmp = exp.substr(pos, i - pos);
                tt = genExpCode(tmp);
                FILE *f = fopen("test.txt", "a");
                fprintf(f, "%s[%d] = %s\n", lval.c_str(), rCnt, tt.c_str());
                fclose(f);
                while (i + 1 < exp.size() && exp[i + 1] == ' ') i++;
                pos = i + 1;
            }
        }
    }
    FILE *f = fopen("test.txt", "a");
    fprintf(f, "\n");
    fclose(f);
}

void genFuncDeclCode(string type, string name) {
    FILE *f = fopen("test.txt", "a");
    fprintf(f, "\nFUNC %s %s\n", type.c_str(), name.c_str());
    fclose(f);
}

void genFuncParamCode(string type, string name) {
    name.erase(std::remove(name.begin(), name.end(), '\n'), name.end());
    name.erase(std::remove(name.begin(), name.end(), ' '), name.end());
    FILE *f = fopen("test.txt", "a");
    fprintf(f, "PARAM %s %s\n", type.c_str(), name.c_str());
    fclose(f);
}

void genCallFuncCode(string name) {
    for (int i = 0; i < utils.size(); i++) {

    }
}

void genPrintfCode(string strCon) {
    int strCnt = 0;
    vector<string> strCons;
    int pos = 1;
    FILE *f = fopen("test.txt", "a");
    for (int i = 1; i < strCon.size(); i++) {
        if (pos >= i) continue;
        if ((i < strCon.size() - 2 && strCon[i] == '%' && strCon[i + 1] == 'd') || strCon[i] == '\"') {
            strCons.push_back(strCon.substr(pos, i - pos));
            strCnt++;
            fprintf(f, "CONST STR %s = \"%s\"\n",
                    ("str" + to_string(strCnt)).c_str(), strCons.back().c_str());
            pos = i + 2;
        }
    }

    strCnt = 0;
    fprintf(f, "PRINTF %s\n", ("str" + to_string(strCnt)).c_str());
    for (auto &var: utils) {
        strCnt++;
        fprintf(f, "PRINTF %s\n", var.c_str());
        fprintf(f, "PRINTF %s\n", ("str" + to_string(strCnt)).c_str());
    }
    fclose(f);
}

void genScanfCode() {
    printCode("test.txt", "READ %s\n", "t0");
}

bool isNum(string str) {
    for (int i = 0; i < str.size(); i++) {
        if (!isdigit(str[i])) return false;
    }
    return true;
}

bool isCalSym(string res) {
    if (res == "+" || res == "-" || res == "/" || res == "%"
        || res == "*" || res == "||" || res == "&&") {
        return true;
    }
    return false;
}

int calculate(string a1, string a2, string sym) {
    int b1 = stoi(a1);
    int b2 = stoi(a2);
    if (sym == "+") return b1 + b2;
    if (sym == "-") return b1 - b2;
    if (sym == "*") return b1 * b2;
    if (sym == "/") return b1 / b2;
    if (sym == "%") return b1 % b2;
    return 0;
}