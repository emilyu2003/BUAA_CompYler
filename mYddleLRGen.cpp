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

#define PRINT 0

using namespace std;

string tAssign;

void printCode(string toFile, string format, string str) {
    if (PRINT) {
        FILE *f = fopen(toFile.c_str(), "a");
        fprintf(f, format.c_str(), str.c_str());
        fclose(f);
    }
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

    // array
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
        return tmp.genName;
    }
    int tmpPos = getIdentPos(name);
    IDENT tmp = identTable[tmpPos];
    if (tmp.type == CONST_ARR_T_D1 || tmp.type == ARRAY_T_D1) {
        string tt = genExpCode(len1);
        if (isNum(tt) && tmp.value_valid) {
            return to_string(tmp.value[stoi(tt)]);
        }
        return tmp.genName + "[" + tt + "]";
    } else if (tmp.type == CONST_ARR_T_D2 || tmp.type == ARRAY_T_D2) {  //TODO
        genAssignCode("t_len1t", len1, 0);
        genAssignCode("t_len2", len2, 0);
        string ttmp = tmp.len2 + " t_len1t  * t_len2 +";
        genAssignCode("t_len1", ttmp, 0);
        return tmp.genName + "[t_len1]";
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
    int flag = 0, funcFlag = 0;
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
                tt = getName("t_");
                if (isNum(a1)) {
                    if (a1 == "0") {
                        a1 = "$0";
                    }
                } else if (isNum(a2)) {
                    if (a2 == "0") {
                        a2 = "$0";
                    }
                }
                poNo.push_back(tt);
                middleCode.push_back(res + " " + a1 + " " + a2 + " " + tt);
                mCode.append(res + " ").append(a1 + " ").append(a2 + " ")
                        .append(tt).append("\n");

                printCode("test.txt", "%s", mCode);
                mCode.clear();
            }

        } else {
            poNo.push_back(res);
        }
    }

    if (!poNo.empty()) {
        return getLvalCode(poNo.back());
    }
    return tt;
}

void genVarCode(string str) {
    int pos = getIdentPos(str);
    IDENT ident = identTable[pos];
    string tmp;
    if (ident.type == INT_T || ident.type == CONST_T) {
        tmp = "t";
    } else if (ident.type == ARRAY_T_D1 || ident.type == CONST_ARR_T_D1) {
        tmp = "arr1";
    } else if (ident.type == ARRAY_T_D2 || ident.type == CONST_ARR_T_D2) {
        tmp = "arr2";
    }

    if (ident.blockNum == 0) tmp += "Global_";
    else tmp += "_";

    tmp = getName(tmp);
    identTable[pos].genName = tmp;
    if (tmp.find("Global") != -1) {
        identTable[pos].value[0] = 0;
        identTable[pos].value_valid = true;
    }

    printCode("test.txt", "var int %s\n", tmp);
    middleCode.push_back("var int " + tmp);
}

void genConstCode(string str) {
    int pos = getIdentPos(str);
    IDENT ident = identTable[pos];
    string tmp;
    if (ident.type == INT_T || ident.type == CONST_T) {
        tmp = getName("t_");
    } else if (ident.type == ARRAY_T_D1 || ident.type == CONST_ARR_T_D1) {
        tmp = getName("arr1_");
    } else if (ident.type == ARRAY_T_D2 || ident.type == CONST_ARR_T_D2) {
        tmp = getName("arr2_");
    }
    identTable[pos].genName = tmp;

    printCode("test.txt", "const int %s\n", tmp);
    middleCode.push_back("const int " + tmp);
}

void genAssignCode(string lval, string exp, int dim) {
    if (dim == 0) {
        string tt = genExpCode(exp);
        IDENT ident = getIdentTemporarily(lval);
        if (ident.genName.empty()) {
            middleCode.push_back(lval + " = " + tt);
            if (PRINT) {
                FILE *f = fopen("test.txt", "a");
                fprintf(f, "%s = %s\n", lval.c_str(), tt.c_str());
                fclose(f);
            }
        } else {
            if (isNum(tt)) {
                ident.value[0] = stoi(tt);
                updateValue(ident);
            } else {
                unvalidateValue(ident);
            }
            lval = ident.genName;
            middleCode.push_back(lval + " = " + tt);
            if (PRINT) {
                FILE *f = fopen("test.txt", "a");
                fprintf(f, "%s = %s\n", lval.c_str(), tt.c_str());
                fclose(f);
            }
        }
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
                IDENT ident = getIdentTemporarily(lval);
                if (isNum(tt)) {
                    ident.value[rCnt] = stoi(tt);
                    updateValue(ident);
                } else {
                    unvalidateValue(ident);
                }
                middleCode.push_back(lval + "[" + to_string(rCnt) + "] = " + tt);
                if (PRINT) {
                    FILE *f = fopen("test.txt", "a");
                    fprintf(f, "%s[%d] = %s\n", lval.c_str(), rCnt, tt.c_str());
                    fclose(f);
                }
                while (i + 1 < exp.size() && exp[i + 1] == ' ') i++;
                pos = i + 1;
            }
        }
        if (rCnt == -1) {    // A[x][y] = a + b;    // TODO
            tt = genExpCode(exp);
            middleCode.push_back(lval + " = " + tt);
            if (PRINT) {
                FILE *f = fopen("test.txt", "a");
                fprintf(f, "%s = %s\n", lval.c_str(), tt.c_str());
                fclose(f);
            }
        }
    }
    if (PRINT) {
        FILE *f = fopen("test.txt", "a");
        fprintf(f, "\n");
        fclose(f);
    }
}

void genFuncDeclCode(string type, string name) {
    middleCode.push_back("func " + type + " " + name);
    if (PRINT) {
        FILE *f = fopen("test.txt", "a");
        fprintf(f, "\nfunc %s %s\n", type.c_str(), name.c_str());
        fclose(f);
    }
}

void genFuncParamCode(string type, string name) {
    name.erase(std::remove(name.begin(), name.end(), '\n'), name.end());
    name.erase(std::remove(name.begin(), name.end(), ' '), name.end());

    int pos = getIdentPos(name);
    string tmp = getName("tParam_");
    identTable[pos].genName = tmp;
    middleCode.push_back("param " + type + " " + tmp);
    if (PRINT) {
        FILE *f = fopen("test.txt", "a");
        fprintf(f, "param %s %s\n", type.c_str(), tmp.c_str());
        fclose(f);
    }
}

string genCallFuncCode(string name, vector<string> utils) {
    IDENT ident = getIdentTemporarily(name);
    string func = getName("t_");
    vector<string> rParams;
    for (auto &util: utils) {
        string tt = genExpCode(util);
        rParams.push_back(tt);
    }

    for (const auto& tt : rParams) {
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

void genPrintfCode(string strCon, vector<string> vars) {
    vector<string> strCons;
    vector<string> strNames;
    vector<int> arrangement; // 1 : strCons, 2 : %d

    strCon = strCon.substr(1, strCon.size() - 2);
    string tmp;
    for (int i = 0; i < strCon.size(); i++) {
        if (i < strCon.size() - 1 && strCon[i] == '%' && strCon[i + 1] == 'd') {
            if (!tmp.empty()) {
                arrangement.push_back(1);
                strCons.push_back("\"" + tmp + "\"");
                string name = getName("str_");
                strNames.push_back(name);

                middleCode.push_back("const string " + name + " = " + +"\"" + tmp + "\"");
                if (PRINT) {
                    FILE *f = fopen("test.txt", "a");
                    fprintf(f, "const string %s = \"%s\"\n", name.c_str(), tmp.c_str());
                    fclose(f);
                }
            }
            tmp.clear();
            arrangement.push_back(2);
            i++;
        } else {
            tmp += strCon[i];
        }
    }
    if (!tmp.empty()) {
        arrangement.push_back(1);
        strCons.push_back("\"" + tmp + "\"");
        string name = getName("str_");
        strNames.push_back(name);

        middleCode.push_back("const string " + name + " = " + +"\"" + tmp + "\"");
        if (PRINT) {
            FILE *f = fopen("test.txt", "a");
            fprintf(f, "const string %s = \"%s\"\n", name.c_str(), tmp.c_str());
            fclose(f);
        }
    }

    int nameCnt = 0, varCnt = 0;
    for (int i: arrangement) {
        if (i == 1) {
            middleCode.push_back("printf " + strNames[nameCnt]);
            if (PRINT) {
                FILE *f = fopen("test.txt", "a");
                fprintf(f, "printf %s\n", strNames[nameCnt].c_str());
                fclose(f);
            }
            nameCnt++;
        } else {
            string tt = genExpCode(vars[varCnt]);
            middleCode.push_back("printf " + tt);
            if (PRINT) {
                FILE *f = fopen("test.txt", "a");
                fprintf(f, "printf %s\n", tt.c_str());
                fclose(f);
            }
            varCnt++;
        }
    }
}

string genScanfCode() {
    string tt = getName("t_");
    printCode("test.txt", "read %s\n", tt);
    middleCode.push_back("read " + tt);
    return tt;
}

void genReturnCode(string str) {
    string tt, t2;
    if (!str.empty()) {
        tt = genExpCode(str);
        t2 = tt;
        if (isNum(tt)) {
            genAssignCode(tAssign, tt, 0);
            t2 = tAssign;
        }
    }
    if (t2.size()) t2 = " " + t2;
    printCode("test.txt", "ret%s\n", t2);
    middleCode.push_back("ret" + t2);
}

string genCondCode(string str) {
    vector<string> poNo;
    poNo.clear();
    str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());

    //int tCnt = -1;
    string res1, mCode, tt, expCode;
    stringstream input;
    input << str;

    vector<string> expStack;

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
                tt = getName("t_");
                if (res == "<") {
                    mCode.append("slt ").append(a1 + " ").append(a2 + " ").append(tt + "\n");
                } else if (res == ">") {
                    mCode.append("slt ").append(a2 + " ").append(a1 + " ").append(tt + "\n");
                } else if (res == ">=") {    // a >= b  -> a + 1 > b //TODO 2147483647?
                    string tt2 = getName("t_");
                    poNo.push_back(tt2);
                    mCode.append("li ").append(tt2 + " ").append("0x1\n");
                    mCode.append("slt ").append(a1 + " ").append(a2 + " ").append(tt2 + "\n");
                    mCode.append("movz ").append(tt2 + " ").append(tt + " ").append(tt + "\n");
                } else if (res == "<=") {    // a <= b  -> a < b + 1
                    string tt2 = getName("t_");
                    mCode.append("li ").append(tt2 + " ").append("0x1\n");
                    mCode.append("slt ").append(a2 + " ").append(a1 + " ").append(tt2 + "\n");
                    mCode.append("movz ").append(tt2 + " ").append(tt + " ").append(tt + "\n");
                } else if (res == "==") {    // a >= b and a <= b
                    string tt2 = getName("t_");
                    mCode.append("li ").append(tt2 + " ").append("0x1\n");
                    mCode.append("sub ").append(a1 + " ").append(a2 + " ").append(tt + "\n");
                    mCode.append("movz ").append(tt2 + " ").append(tt + " ").append(tt + "\n");
                } else if (res == "!=") {
                    string tt2 = getName("t_");
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
    vector<string> expStack;
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