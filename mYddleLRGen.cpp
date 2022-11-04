//
// Created by emilyu on 2022/10/26.
//
#include <cstdio>
#include <algorithm>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include "MLRGen.h"
#include "IdentTable.h"
#include "TLRGen.h"

using namespace std;

vector<string> utils;

void printCode(string toFile, string format, string str) {
    FILE *f = fopen(toFile.c_str(), "a");
    fprintf(f, format.c_str(), str.c_str());
    fclose(f);
}

void genString(string str) {
    printCode("test.txt", "%s\n", str);
    middleCode.push_back(str);
}

string dealUnaryOp(string str) {    //todo: !-+-a
    string ttmp, res;

    stringstream input;
    input << str;

    while (input >> res) {
        if (res.size() > 1 && (res[0] == '+' || res[0] == '-')) {
            int start = 0;
            string sym = "+";
            for (; start < res.size(); start++) {
                if (!(res[start] == '-' || res[start] == '+')) break;
                if (sym == "+" && res[start] == '-') sym = '-';
                else if (sym == "-" && res[start] == '-') sym = '+';
            }
            if (sym == "+") {
                ttmp += " " + res.substr(start) + " ";
            } else {
                ttmp += " 0 " + res.substr(start) + " - ";
            }
        } else {
            ttmp += " " + res + " ";
        }
    }

    return ttmp;
}

string getLvalCode(string str) {
    string name, len1, len2;
    int cnt1 = -1, cnt2 = -1;
    for (int i = 0; i < str.size(); i++) {
        if (str[i] == '[') {
            if (cnt1 == -1) {
                name = str.substr(0, i);
                name.erase(std::remove(name.begin(), name.end(), ' '), name.end());
            }
            cnt1 = i;
        } else if (str[i] == ']') {
            if (cnt2 == -1) {
                len1 = str.substr(cnt1 + 1, i - cnt1 - 1);
            } else {
                len2 = str.substr(cnt1 + 1, i - cnt1 - 1);
            }
            cnt2 = i;
        }
    }
    if (cnt1 == -1) {
        if (isNum(str)) return str;
        int tmpPos = getIdentPos(str);
        if (tmpPos == -1) return str;
        IDENT tmp = identTable[tmpPos];
        if (tmp.value_valid) {
            return to_string(tmp.value[0]);
        }
        return str;
    }
    int tmpPos = getIdentPos(name);
    IDENT tmp = identTable[tmpPos];
    if (tmp.type == CONST_ARR_T_D1 || tmp.type == ARRAY_T_D1) {
        string tt = genExpCode(len1);
        if (isNum(tt) && tmp.value_valid) {
            return to_string(tmp.value[stoi(tt)]);
        }
        return name + "[" + tt + "]";
    } else if (tmp.type == CONST_ARR_T_D2 || tmp.type == ARRAY_T_D2) {  //TODO
        genAssignCode("t_len1t", len1, 0);
        genAssignCode("t_len2", len2, 0);
        string ttmp = tmp.len2 + " t_len1t  * t_len2 +";
        genAssignCode("t_len1", ttmp, 0);
        return name + "[t_len1]";
    }
    return str;
}

string genExpCode(string str) {
    vector<string> poNo;
    poNo.clear();
    str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());

    string res, mCode, tt;
    str = dealUnaryOp(str);
    stringstream input;
    input << str;
    int flag = 0;
    while (input >> res) {
        if (res == "[") {
            string tmp = poNo.back();
            poNo.pop_back();
            poNo.push_back(tmp + "[");
            flag = 1;
        } else if (flag || res == "]") {
            string tmp = poNo.back();
            poNo.pop_back();
            tmp += " ";
            tmp += res;
            poNo.push_back(tmp);
            if (res == "]") flag = 0;
        } else if (isCalSym(res)) {
            string a2 = poNo.back();
            a2 = getLvalCode(a2);
            poNo.pop_back();
            string a1 = poNo.back();
            a1 = getLvalCode(a1);
            poNo.pop_back();

            if (isNum(a1) && isNum(a2) && (res != "||" && res != "&&")) {
                int ansNum = calculate(a1, a2, res);
                poNo.push_back(to_string(ansNum));
            } else {
                tt = getName("t");
                poNo.push_back(tt);
                middleCode.push_back(res + " " + a1 + " " + a1 + " " + tt);
                mCode.append(res + " ").append(a1 + " ").append(a2 + " ")
                        .append(tt).append("\n");
            }
        } else {
            poNo.push_back(res);
        }
    }

    printCode("test.txt", "%s", mCode);
    if (!poNo.empty()) {
        return getLvalCode(poNo.back());
    }
    return tt;
}

void genVarCode(string str) {
    printCode("test.txt", "var int %s\n", str);
    middleCode.push_back("var int " + str);
}

void genConstCode(string str) {
    printCode("test.txt", "const int %s\n", str);
    middleCode.push_back("const int " + str);
}

void genAssignCode(string lval, string exp, int dim) {
    if (dim == 0) {
        string tt = genExpCode(exp);
        if (isNum(tt)) {
            IDENT ident = getIdentTemporarily(lval);
            ident.value[0] = stoi(tt);
            updateValue(ident);
        }
        FILE *f = fopen("test.txt", "a");
        middleCode.push_back(lval + " = " + tt);
        fprintf(f, "%s = %s\n", lval.c_str(), tt.c_str());
        fclose(f);
    } else {
        lval = getLvalCode(lval);
        exp.erase(std::remove(exp.begin(), exp.end(), '\n'), exp.end());
        int pos = 1;
        while (pos < exp.size() && (exp[pos] == '{' || exp[pos] == ' ')) pos++;
        string tt;
        int rCnt = -1;
        for (int i = 0; i < exp.size(); i++) {
            if (exp[i] == '{') continue;
            // int A[x][y] = {a, b, c};
            if (i > pos && (exp[i] == ',' || exp[i] == '}')) {
                rCnt++;
                string tmp = exp.substr(pos, i - pos);
                tt = genExpCode(tmp);
                if (isNum(tt)) {
                    IDENT ident = getIdentTemporarily(lval);
                    ident.value[rCnt] = stoi(tt);
                    updateValue(ident);
                }
                FILE *f = fopen("test.txt", "a");
                middleCode.push_back(lval + "[" + to_string(rCnt) + "] = " + tt);
                fprintf(f, "%s[%d] = %s\n", lval.c_str(), rCnt, tt.c_str());
                fclose(f);
                while (i + 1 < exp.size() && exp[i + 1] == ' ') i++;
                pos = i + 1;
            }
        }
        if (rCnt == -1) {    // A[x][y] = a + b;    // TODO
            tt = genExpCode(exp);
            FILE *f = fopen("test.txt", "a");
            middleCode.push_back(lval + " = " + tt);
            fprintf(f, "%s = %s\n", lval.c_str(), tt.c_str());
            fclose(f);
        }
    }
    FILE *f = fopen("test.txt", "a");
    fprintf(f, "\n");
    fclose(f);
}

void genFuncDeclCode(string type, string name) {
    FILE *f = fopen("test.txt", "a");
    middleCode.push_back("func " + type + " " + name);
    fprintf(f, "\nfunc %s %s\n", type.c_str(), name.c_str());
    fclose(f);
}

void genFuncParamCode(string type, string name) {
    name.erase(std::remove(name.begin(), name.end(), '\n'), name.end());
    name.erase(std::remove(name.begin(), name.end(), ' '), name.end());
    FILE *f = fopen("test.txt", "a");
    middleCode.push_back("param " + type + " " + name);
    fprintf(f, "param %s %s\n", type.c_str(), name.c_str());
    fclose(f);
}

string genCallFuncCode(string name) {
    IDENT ident = getIdentTemporarily(name);
    string func = getName("funcall_");
    for (auto &util: utils) {
        string tt = genExpCode(util);
        printCode("test.txt", "push %s\n", tt);
        middleCode.push_back("push " + tt);
    }
    utils.clear();
    printCode("test.txt", "call %s\n", name);
    middleCode.push_back("call " + name);
    if (ident.type == FUNC_T_INT) {
        genAssignCode(func, "RET", 0);
    }
    return func;
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

            middleCode.push_back("const str str" + to_string(strCnt) + " = " + +"\"" + strCons.back() + "\"");
            fprintf(f, "const str %s = \"%s\"\n",
                    ("str" + to_string(strCnt)).c_str(), strCons.back().c_str());
            pos = i + 2;
            strCnt++;
        }
    }

    strCnt = 0;
    middleCode.push_back("printf " + ("str" + to_string(strCnt)));
    fprintf(f, "printf %s\n", ("str" + to_string(strCnt)).c_str());
    for (auto &var: utils) {
        strCnt++;
        middleCode.push_back("printf " + var);
        fprintf(f, "printf %s\n", var.c_str());
        middleCode.push_back("printf " + ("str" + to_string(strCnt)));
        fprintf(f, "printf %s\n", ("str" + to_string(strCnt)).c_str());
    }
    fclose(f);
}

string genScanfCode() {
    string tt = getName("t");
    printCode("test.txt", "read %s\n", tt);
    middleCode.push_back("read " + tt);
    return tt;
}

void genReturnCode(string str) {
    string tt;
    if (!str.empty()) {
        tt = genExpCode(str);
    }
    printCode("test.txt", "ret %s\n", tt);
    middleCode.push_back("ret " + tt);
}

string genCondCode(string str) {
    vector <string> poNo;
    poNo.clear();
    str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());

    //int tCnt = -1;
    string res1, mCode, tt, expCode;
    stringstream input;
    input << str;

    vector <string> expStack;

    string a;
    while (input >> res1) {
        if (res1 == "==" || res1 == "!=" || res1 == ">" || res1 == "<" || res1 == "<=" || res1 == ">=") {
            expStack.push_back(expCode);
            expCode.clear();
            expStack.push_back(res1);
        } else {
            if (isCalSym(res1)) {
                string a2 = poNo.back();
                poNo.pop_back();
                string a1 = poNo.back();
                poNo.pop_back();
            }
        }
    }
    if (!expCode.empty()) {
        expStack.push_back(expCode);
    }

    for (const auto &res: expStack) {
        if (isRelSym(res)) {
            string a2 = poNo.back();
            a2 = genExpCode(a2);
            poNo.pop_back();
            string a1 = poNo.back();
            a1 = genExpCode(a1);
            poNo.pop_back();

            if (isNum(a1) && isNum(a2)) {
                int ansNum = calculate(a1, a2, res);
                poNo.push_back(to_string(ansNum));
            } else {
                tt = getName("t");
                if (res == "<") {
                    mCode.append("slt ").append(a1 + " ").append(a2 + " ").append(tt + "\n");
                } else if (res == ">") {
                    mCode.append("slt ").append(a2 + " ").append(a1 + " ").append(tt + "\n");
                } else if (res == ">=") {    // a >= b  -> a + 1 > b //TODO 2147483647?
                    string tt2 = getName("t");
                    poNo.push_back(tt2);
                    mCode.append("li ").append(tt2 + " ").append("0x1\n");
                    mCode.append("slt ").append(a1 + " ").append(a2 + " ").append(tt2 + "\n");
                    mCode.append("movz ").append(tt2 + " ").append(tt + " ").append(tt + "\n");
                } else if (res == "<=") {    // a <= b  -> a < b + 1
                    string tt2 = getName("t");
                    mCode.append("li ").append(tt2 + " ").append("0x1\n");
                    mCode.append("slt ").append(a2 + " ").append(a1 + " ").append(tt2 + "\n");
                    mCode.append("movz ").append(tt2 + " ").append(tt + " ").append(tt + "\n");
                } else if (res == "==") {    // a >= b and a <= b
                    string tt2 = getName("t");
                    mCode.append("li ").append(tt2 + " ").append("0x1\n");
                    mCode.append("sub ").append(a1 + " ").append(a2 + " ").append(tt + "\n");
                    mCode.append("movz ").append(tt2 + " ").append(tt + " ").append(tt + "\n");
                } else if (res == "!=") {
                    string tt2 = getName("t");
                    mCode.append("xor ").append(a1 + " ").append(a2 + " ").append(tt2 + "\n");
                    mCode.append("slt ").append(tt + " ").append("$0 ").append(tt + "\n");
                    mCode.append("slt ").append(a2 + " ").append("$0 ").append(tt + "\n");
                }
                poNo.push_back(tt);
            }
        } else {
            poNo.push_back(res);
        }
    }

    printCode("test.txt", "%s", mCode);
    middleCode.push_back(mCode);

    if (!poNo.empty()) {
        return getLvalCode(poNo.back());
    }
    return tt;
}

void genAndCode(string str, string endStr) {
    vector<string> expStack;
    stringstream input;
    input << str;
    string res1, mCode;
    string expCode;

    // devide by &&
    while (input >> res1) {
        if (res1 == "&&") {
            expStack.push_back(expCode);
            expCode.clear();
            expStack.push_back(res1);
        } else {
            expCode += " " + res1 + " ";
        }
    }
    if (!expCode.empty()) {
        expStack.push_back(expCode);
    }

    for (auto res: expStack) {
        string tt = genCondCode(res);
        mCode.append("beq $0 ").append(tt + " ").append(endStr + "\n");
    }
    printCode("test.txt", "%s", mCode);
    middleCode.push_back(mCode);
}

void genIfCode(string str, string beginStr, string elseStr, string endIfStr) {
    vector <string> expStack;
    stringstream input;
    input << str;
    string expCode, mCode, res1;

    string endStr = (elseStr.empty() ? endIfStr : elseStr);

    // devide by ||
    while (input >> res1) {
        if (res1 == "||") {
            expStack.push_back(expCode);
            expCode.clear();
        } else {
            expCode += " " + res1 + " ";
        }
    }
    if (!expCode.empty()) {
        expStack.push_back(expCode);
    }


    for (auto expStr: expStack) {
        genAndCode(expStr, endStr);
        mCode.append("j ").append(beginStr + "\n");
    }
    printCode("test.txt", "%s", mCode);
    middleCode.push_back(mCode);
}

bool isNum(string str) {
    int start = 0;
    if (str[0] == '-') start = 1;

    for (int i = start; i < str.size(); i++) {
        if (!isdigit(str[i])) return false;
    }
    return true;
}

bool isCalSym(string res) {
    if (res == "+" || res == "-" || res == "/" || res == "%"
        || res == "*") {
        return true;
    }
    return false;
}

bool isRelSym(string res) {
    if (res == ">" || res == "<" || res == "!=" || res == "=="
        || res == "<=" || res == ">=") {
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
    if (sym == "<") return b1 < b2;
    if (sym == "<=") return b1 <= b2;
    if (sym == ">") return b1 > b2;
    if (sym == ">=") return b1 >= b2;
    if (sym == "==") return b1 == b2;
    if (sym == "!=") return b1 != b2;

    return 0;
}