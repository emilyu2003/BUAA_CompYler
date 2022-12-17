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
#include <unordered_map>

#define PRINT 0

using namespace std;

string tAssign;

unordered_map<string, string> outputStrCons;

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

    // ident
    if (cnt1 == -1) {
        int flag = 0;
        if (str.find("!") != -1) {
            flag = 1;
            str = str.substr(str.find("!") + 1);
        }
        if (isNum(str)) {
            int num = stoi(str);
            if (flag) {
                if (num == 0) return to_string(1);
                else return to_string(0);
            } else {
                return str;
            }
        }
        int tmpPos = getIdentPos(str);
        if (tmpPos == -1) { // TODO
            if (flag) {
                string tt = getName("t_");
                middleCode.push_back("seq " + tt + " " + str + " $0 ");
                return tt;
            }
            return str;
        }
        IDENT tmp = identTable[tmpPos];
        if (tmp.type == INT_T || tmp.type == CONST_T) {
            if (tmp.value_valid) {
                if (flag) {
                    return to_string(!tmp.value[0]);
                }
                return to_string(tmp.value[0]);
            }
            if (flag) {
                string tt = getName("t_");
                middleCode.push_back("seq " + tt + " " + tmp.genName + " $0 ");
                return tt;
            }
            return tmp.genName;
        }
    }

    // array
    int tmpPos;
    if (name.empty()) {
        tmpPos = getIdentPos(str);
    } else {
        tmpPos = getIdentPos(name);
    }
    IDENT tmp = identTable[tmpPos];
    if (tmp.type == CONST_ARR_T_D1 || tmp.type == ARRAY_T_D1) {
        if (len1.empty()) {         // a[x] but paramed as a
            //return tmp.genName + "@addr";
            return tmp.genName;
        } else {
            string tt;
            if (isNum(len1)) {
                tt = to_string(stoi(len1));
            } else {
                tt = genExpCode(len1);
            }
            if (isNum(tt) && tmp.value_valid) {
                return to_string(tmp.value[stoi(tt)]) + "@addr";
            }
            return tmp.genName + "[" + tt + "]";
        }
    } else if (tmp.type == CONST_ARR_T_D2 || tmp.type == ARRAY_T_D2) {  //TODO
        if (len1.empty()) {      // a[x][y] but only send param named a
            //return tmp.genName + "@addr";
            return tmp.genName;
        } else if (len2.empty()) {             // a[x][y] but only send param named a[i]
            string tt;
            if (isNum(len1)) {
                len1.erase(std::remove(len1.begin(), len1.end(), ' '), len1.end());
                tt = to_string(stoi(len1) * tmp.len2);
            } else {
                tt = genExpCode(to_string(tmp.len2) + len1 + " * ");
            }
//            if (isNum(tt) && tmp.value_valid) {
//                return to_string(tmp.value[stoi(tt)]) + "@addr";
//            }
//            return tmp.genName + "[" + tt + "]" + "@addr";
            if (isNum(tt) && tmp.value_valid) {
                return to_string(tmp.value[stoi(tt)]);
            }
            return tmp.genName + "[" + tt + "]";
        } else {    // a[x][y] as a[i][j]
            string tt;  //TODO
            if (isNum(len1) && isNum(len2)) {
                len1.erase(std::remove(len1.begin(), len1.end(), ' '), len1.end());
                len2.erase(std::remove(len2.begin(), len2.end(), ' '), len2.end());
                tt = to_string(tmp.len2 * stoi(len1) + stoi(len2));
            } else if (isNum(len1)) {
                len1.erase(std::remove(len1.begin(), len1.end(), ' '), len1.end());
                tt = genExpCode(to_string(tmp.len2 * stoi(len1)) + len2 + " + ");
            } else if (isNum(len2)) {
                len2.erase(std::remove(len2.begin(), len2.end(), ' '), len2.end());
                tt = genExpCode(to_string(tmp.len2) + len1 + " * " + len2 + " +");
            } else {
                tt = genExpCode(to_string(tmp.len2) + len1 + " * " + len2 + " +");
            }
            if (isNum(tt) && tmp.value_valid) {
                return to_string(tmp.value[stoi(tt)]);
            }
            return tmp.genName + "[" + tt + "]";
        }
    }
    return str;
}

string genExpCode(string str) {
    vector<string> poNo;
    poNo.clear();
    str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());

    string res, mCode, tt;
    //str = dealUnaryOp(str);
    stringstream input;
    input << str;
    int flag = 0, funcFlag = 0;
    int brackCnt = 0;
    while (input >> res) {
        if (res == "[") {
            string expInIndex;
            brackCnt++;
            while (input >> res) {
                expInIndex += " " + res + " ";
                if (res == "]") {
                    brackCnt--;
                    if (brackCnt == 0) {
                        expInIndex = expInIndex.substr(0, expInIndex.size() - 3);
                        string tmp = poNo.back();
                        poNo.pop_back();
                        if (isNum(expInIndex)) {
                            poNo.push_back(tmp + "[" + expInIndex + "]");
                        } else {
                            string tt1 = getName("t_"); // if is num can be ignored TODO
                            genAssignCode(tt1, expInIndex, 0);
                            poNo.push_back(tmp + "[" + tt1 + "]");
                        }
                        break;
                    }
                } else if (res == "[") {
                    brackCnt++;
                }
            }
            input >> res;
            expInIndex.clear();
            if (res == "[") {   // 2d array
                brackCnt++;
                while (input >> res) {
                    expInIndex += " " + res + " ";
                    if (res == "]") {
                        brackCnt--;
                        if (brackCnt == 0) {
                            expInIndex = expInIndex.substr(0, expInIndex.size() - 3);
                            string tmp = poNo.back();
                            poNo.pop_back();
                            if (isNum(expInIndex)) {
                                poNo.push_back(tmp + "[" + expInIndex + "]");
                            } else {
                                string tt2 = getName("t_");
                                genAssignCode(tt2, expInIndex, 0);
                                poNo.push_back(tmp + "[" + tt2 + "]");
                            }
                            break;
                        }
                    } else if (res == "[") {
                        brackCnt++;
                    }
                }
            }
        }   // already read one, no need to add else
        if (isCalSym(res)) {
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
                poNo.push_back(tt);

                if (res == "<" || res == ">=" || res == ">" || res == "<=" || res == "==" || res == "!=") {
                    if (isNum(a1)) {
                        swap(a1, a2);
                        if (res == "<") res = ">=";
                        else if (res == ">") res = "<=";
                        else if (res == "<=") res = ">";
                        else if (res == ">=") res = "<";
                    }
                    if (res == "<") {
                        middleCode.push_back("slt " + a1 + " " + a2 + " " + tt);
                        mCode.append("slt ").append(a1 + " ").append(a2 + " ").append(tt + "\n");
                    } else if (res == ">") {
                        middleCode.push_back("sgt " + a1 + " " + a2 + " " + tt);
                        mCode.append("sgt ").append(a1 + " ").append(a2 + " ").append(tt + "\n");
                    } else if (res == ">=") {
                        middleCode.push_back("sge " + a1 + " " + a2 + " " + tt);
                        mCode.append("sge ").append(a1 + " ").append(a2 + " ").append(tt + "\n");
                    } else if (res == "<=") {
                        middleCode.push_back("sle " + a1 + " " + a2 + " " + tt);
                        mCode.append("sle ").append(a1 + " ").append(a2 + " ").append(tt + "\n");
                    } else if (res == "==") {
                        middleCode.push_back("seq " + a1 + " " + a2 + " " + tt);
                        mCode.append("seq ").append(a1 + " ").append(a2 + " ").append(tt + "\n");
                    } else if (res == "!=") {
                        middleCode.push_back("sne " + a1 + " " + a2 + " " + tt);
                        mCode.append("sne ").append(a1 + " ").append(a2 + " ").append(tt + "\n");
                    }
                    poNo.push_back(tt);
                } else {
                    if (isNum(a1)) {
                        if (a1 == "0") {
                            a1 = "$0";
                        }
                    } else if (isNum(a2)) {
                        if (a2 == "0") {
                            a2 = "$0";
                        }
                    }
                    middleCode.push_back(res + " " + a1 + " " + a2 + " " + tt);
                    mCode.append(res + " ").append(a1 + " ").append(a2 + " ")
                            .append(tt).append("\n");
                }

                printCode("test.txt", "%s", mCode);
                mCode.clear();
            }
        } else if (res != "]") {
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
    string tmp, sz;
    if (ident.type == INT_T || ident.type == CONST_T) {
        tmp = "int";
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
        for (int i = 0; i < identTable[pos].len1; i++) {
            for (int j = 0; j < identTable[pos].len2; j++) {
                identTable[pos].value.push_back(0);
            }
        }
        identTable[pos].value_valid = true;
    }

    sz = to_string(identTable[pos].len1 * identTable[pos].len2);
    printCode("test.txt", "var int %s\n", tmp);
    middleCode.push_back("var int " + tmp + " " + sz);
}

void genConstCode(string str) {
    int pos = getIdentPos(str);
    IDENT ident = identTable[pos];
    string tmp, sz;
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

    sz = to_string(identTable[pos].len1 * identTable[pos].len2);
    printCode("test.txt", "const int %s\n", tmp);
    middleCode.push_back("const int " + tmp + " " + sz);
}

void genAssignCode(string lval, string exp, int dim) {
    if (dim == 0) {
        string tt = genExpCode(exp);
        IDENT ident = getIdentTemporarily(lval);
        if (!ident.genName.empty()) {
            if (isNum(tt)) {
                //ident.value[0] = stoi(tt);
                updateValue(ident, 0, stoi(tt));
            } else {
                unvalidateValue(ident);
            }
            lval = ident.genName;
        }
        middleCode.push_back(lval + " = " + tt);
    } else {
        exp.erase(std::remove(exp.begin(), exp.end(), '\n'), exp.end());
        int pos = 1;
        while (pos < exp.size() && (exp[pos] == '{' || exp[pos] == ' ')) pos++;
        string tt;
        int rCnt = -1;
        for (int i = pos; i < exp.size(); i++) {
            if (exp[i] == '{') continue;
            // int A[x][y] = {a, b, c};
            if (i > pos && (exp[i] == ',' || exp[i] == '}')) {
                rCnt++;
                string tmp = exp.substr(pos, i - pos);
                tt = genExpCode(tmp);
                IDENT ident = getIdentTemporarily(lval);
                if (isNum(tt)) {
//                    if (ident.value.size() < rCnt + 1) {
//                        int k = ident.value.size();
//                        ident.value.push_back(stoi(tt));    // TODO initialize
//                    } else {
//                        ident.value[rCnt] = stoi(tt);
//                    }
                    updateValue(ident, rCnt, stoi(tt));
                } else {
                    unvalidateValue(ident);
                }
                middleCode.push_back(getLvalCode(lval) + "[" + to_string(rCnt) + "] = " + tt);
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
            lval = getLvalCode(lval);
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

void genFuncParamBlockCode() {
    middleCode.push_back("@ " + to_string(getBlockNum()));
}

void genFuncParamCode(string type, string str) {
    str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());

    stringstream input;
    string res, name;
    int len1 = -1, len2 = -1;
    input << str;
    while (input >> res) {
        if (name.empty()) {
            name = res;
        } else if (res == "[") {
            string tmpStr;
            while (input >> res) {
                if (res == "]") {
                    if (tmpStr.empty()) {
                        len1 = 1;//a[] a[][1]
                    } else {
                        string tt = genExpCode(tmpStr);
                        if (isNum(tt)) {
                            if (len1 == -1) len1 = stoi(tt);
                            else len2 = stoi(tt);
                        } // no else here, pay attention
                    }
                    tmpStr.clear();
                    break;
                }
                tmpStr += " " + res + " ";
            }
        }
    }

    int pos = getIdentPos(name);
    string tmp = getName("tParam_");
    identTable[pos].genName = tmp;
    middleCode.push_back("param " + type + " " + tmp);

    identTable[pos].len1 = identTable[pos].len2 = 1;
    if (type == "arr1") {
        identTable[pos].len1 = len1;
    } else if (type == "arr2") {
        identTable[pos].len1 = len1;
        identTable[pos].len2 = len2;
    }
    if (PRINT) {
        FILE *f = fopen("test.txt", "a");
        fprintf(f, "param %s %s %d\n", type.c_str(), tmp.c_str(), len2);
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

    for (int i = 0; i < rParams.size(); i++) {
        string tt = rParams[i];
        if (ident.params[i].type != INT_T && ident.params[i].type != CONST_T) tt += "@addr";
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
                string name = getFromStrCons(tmp);
                strNames.push_back(name);

                // middleCode.push_back("const string " + name + " = " + +"\"" + tmp + "\"");
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
        string name = getFromStrCons(tmp);
        strNames.push_back(name);

        // middleCode.push_back("const string " + name + " = " + +"\"" + tmp + "\"");
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
            string x = vars[varCnt];
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
            //expStack.push_back(res1);
        } else {
            expCode += " " + res1 + " ";
        }
    }
    if (!expCode.empty()) {
        expStack.push_back(expCode);
    }

    for (auto res: expStack) {
        string tt = genExpCode(res);
        middleCode.push_back("beq $0 " + tt + " " + endStr);
        mCode.append("beq $0 ").append(tt + " ").append(endStr + "\n");
    }
    printCode("test.txt", "%s", mCode);
}

void genCondCode(string str, string beginStr, string elseStr, string endIfStr) {
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
        middleCode.push_back("j " + beginStr);
    }
    printCode("test.txt", "%s", mCode);
}

bool isNum(string str) {
    str.erase(std::remove(str.begin(), str.end(), ' '), str.end());
    if (str.empty()) return false;
    int start = 0;
    if (str[0] == '-') start = 1;

    for (int i = start; i < str.size(); i++) {
        if (!isdigit(str[i])) return false;
    }
    return true;
}

bool isCalSym(string res) {
    if (res == "+" || res == "-" || res == "/" || res == "%"
        || res == "*" || res == "!" || res == ">=" || res == ">"
        || res == "<=" || res == "<" || res == "==" || res == "!=") {
        return true;
    }
    return false;
}

string getFromStrCons(string a) {
    if (outputStrCons.count(a) == 0) {
        outputStrCons[a] = getName("str_");
        middleCode.push_back("const string " + outputStrCons[a] + " = " + +"\"" + a + "\"");
    }
    return outputStrCons[a];
}

int calculate(string a1, string a2, string sym) {
    int b1 = stoi(a1);
    int b2 = stoi(a2);
    if (sym == "+") return b1 + b2;
    if (sym == "-") return b1 - b2;
    if (sym == "*") return b1 * b2;
    if (sym == "/") return b1 / b2;
    if (sym == "%") return b1 % b2;
    if (sym == "!") return b2 == 0;
    if (sym == "<") return b1 < b2;
    if (sym == "<=") return b1 <= b2;
    if (sym == ">") return b1 > b2;
    if (sym == ">=") return b1 >= b2;
    if (sym == "==") return b1 == b2;
    if (sym == "!=") return b1 != b2;

    return 0;
}