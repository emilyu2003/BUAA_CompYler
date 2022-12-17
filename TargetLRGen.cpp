//
// Created by emilyu on 2022/10/28.
//
#include <cstring>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <unordered_set>
#include "TLRGen.h"
#include "MLRGen.h"

using namespace std;

vector<string> middleCode;
vector<T_NAME> ssaIdents;
unordered_set<string> stringPrint;
vector<string> generatedCode;
bool generatedStatus[100005];
int fpBias = 0;
int gpBias = 0;

// allocate register
void initGenerator() {
    middleCode.pop_back();  // return 0
    middleCode.pop_back();  // t_0 = 0
    memset(generatedStatus, 0, sizeof(generatedStatus));

    for (const auto &s: middleCode) {
        stringstream input;
        input << s;
        string res;
        while (input >> res) {
            if (res.substr(0, 2) == "t_") {
                int pos = getSsaIdentPos(res);
                if (pos == -1) {
                    T_NAME tName = {res, 1};
                    ssaIdents.push_back(tName);
                } else {
                    ssaIdents[pos].cnt++;
                }
            }
        }
    }

    sort(ssaIdents.begin(), ssaIdents.end());

    for (int i = 0; i < ssaIdents.size(); i++) {
        if (i < 8) {
            ssaIdents[i].addr = ("$t" + to_string(i));
        } else {
            ssaIdents[i] = {};
        }
    }
}

int getSsaIdentPos(const std::string &name) {
    for (int i = 0; i < ssaIdents.size(); i++) {
        if (ssaIdents[i].name == name) {
            return i;
        }
    }
    return -1;
}

void printMiddleCode() {
    for (const auto &s: middleCode) {
        cout << ">" << s << "<" << endl;
    }
}

string getAddr(string str, string target, bool print) {
    if (str == "$0") return str;
    if (str.find("$t") != -1) return str;
    if (isNum(str)) return str;
    bool isParam = false;
    if (str.find("Param") != -1) {  // nothing to do here
        isParam = true;
    }

    int pos = getSsaIdentPos(str);

    string arrBias = "0";
    string arrName;
    bool isArr = false;
    if (pos == -1) { // is an array
        for (int i = 0; i < str.size(); i++) {
            if (str[i] == '[') {
                arrBias = str.substr(i + 1, str.size() - i - 2);
                isArr = true;
                break;
            } else {
                arrName += str[i];
            }
        }
        pos = getSsaIdentPos(arrName);
    }
    if (pos == -1) {    // temporary t_
        newVar("tmp int " + str + " 1", 0);
        pos = getSsaIdentPos(str);
    }

    string ret = ssaIdents[pos].addr;

//    if (ret.empty()) {  // can be optimized
//        newVar("tmp int " + str + " 1", 0);
//        ret = ssaIdents[pos].addr;
//    }


    if (isArr) {
        if (isParam) {
            if (isNum(arrBias)) {
                if (!target.empty()) {
                    generatedCode.push_back("lw $s7, " + ret);
                    int realArrBias = 4 * stoi(arrBias);
                    generatedCode.push_back("addu $s7, $s7, " + to_string(realArrBias));
                }
            } else {
                if (!target.empty()) {
                    string tt = getAddr(arrBias, "$s6", true);
                    generatedCode.push_back("li $s7, " + ret);  // TODO
                    generatedCode.push_back("addu $s7, $s7, " + tt);
                }
            }
            ret = "($s7)";
        } else {
            if (isNum(arrBias)) {
                ret = to_string(stoi(ret) + 4 * stoi(arrBias));
            } else {
                if (!target.empty()) {
                    string tt = getAddr(arrBias, "$s6", true);  // maybe there's a problem, maybe not
                    generatedCode.push_back("li $s7, " + ret);
                    generatedCode.push_back("addu $s7, $s7, " + tt);
                }
                ret = "($s7)";
            }
        }
    }

    if (isNum(ret) && !target.empty()) {
        if (print) {
            generatedCode.push_back("lw " + target + ", " + ret);
        }
        return target;
    } else if (isArr && !target.empty()) {
        if (print) {
            generatedCode.push_back("lw " + target + ", ($s7)");
        }
        return target;
    }
    return ret;
}

void newVar(string s, int isGlobal) {
    string type1, type2, str, res, sz = "1";
    stringstream input;
    input << s;
    while (input >> res) {
        if (type1.empty()) {
            type1 = res;
        } else if (type2.empty()) {
            type2 = res;
        } else if (str.empty()) {
            str = res;
        } else {
            sz = res;
        }
    }

    int pos = getSsaIdentPos(str);
    if (isGlobal) {
        if (pos == -1) {
            T_NAME tName = {str};
            ssaIdents.push_back(tName);
            ssaIdents.back().addr = to_string(gpBias + GP_BASE);
            gpBias += stoi(sz) * 4;
            generatedCode.push_back("addi $gp, $gp, " + to_string(stoi(sz) * 4));
        }
    } else {
        if (pos == -1) {
            T_NAME tName = {str};
            ssaIdents.push_back(tName);
            ssaIdents.back().addr = to_string(fpBias + FP_BASE);
            fpBias += stoi(sz) * 4;
            generatedCode.push_back("addi $fp, $fp, " + to_string(stoi(sz) * 4));
        }
    }
}

void newParam(string s) {
    int pos = getSsaIdentPos(s);
    if (pos == -1) {
        T_NAME tName = {s};
        ssaIdents.push_back(tName);
        ssaIdents.back().addr = to_string(fpBias + FP_BASE);
        fpBias += 4;
        //fpBias += stoi(sz) * 4;
        //generatedCode.push_back("addi $fp, $fp, " + to_string(stoi(sz) * 4)); TODO
    }
}

void mipsGenExp(string s) {
    string a, b, c, res;
    stringstream input;
    input << s;
    string toMem;
    while (input >> res) {
        if (res == "+" || res == "-" || res == "*" || res == "/" || res == "%" || res == "!") continue;
        if (a.empty()) a = res;
        else if (b.empty()) b = res;
        else if (c.empty()) c = res;
    }
    if (s[0] == '+' || s[0] == '*') {
        if (isNum(a)) {
            swap(a, b);
        }
    }
    a = getAddr(a, "$t8", true);
    b = getAddr(b, "$t9", true);
    string tmpc = getAddr(c, "$t8", false);
    string endc = getAddr(c, "", false);
    if (tmpc == "$t8") toMem = getAddr(c, "", false);
    c = tmpc;

    string tmp;
    if (s[0] == '+') {
        generatedCode.push_back("addu " + c + ", " + a + ", " + b);
    } else if (s[0] == '-') {
        if (isNum(a)) {
            generatedCode.push_back("li $t8, " + a);
            a = "$t8";
        }
        generatedCode.push_back("subu " + c + ", " + a + ", " + b);
    } else if (s[0] == '*') {
        generatedCode.push_back("mul " + c + ", " + a + ", " + b);
    } else if (s[0] == '/') {
        if (isNum(a)) {
            generatedCode.push_back("li $t8, " + a);
            a = "$t8";
        } else if (isNum(b)) {
            generatedCode.push_back("li $t9, " + b);
            b = "$t9";
        }
        generatedCode.push_back("div " + a + ", " + b);
        generatedCode.push_back("mflo " + c);
    } else if (s[0] == '%') {
        if (isNum(a)) {
            generatedCode.push_back("li $t8, " + a);
            a = "$t8";
        } else if (isNum(b)) {
            generatedCode.push_back("li $t9, " + b);
            b = "$t9";
        }
        generatedCode.push_back("div " + a + ", " + b);
        generatedCode.push_back("mfhi " + c);
    } else if (s[0] == '!') {
        if (isNum(b)) {
            generatedCode.push_back("li $t9, " + b);
            b = "$t9";
        }
        generatedCode.push_back("seq " + c + ", " + b + ", $0");
    }

    if (tmpc == "$t8") {
        generatedCode.push_back("sw $t8, " + endc);
    }
}

void mipsGenAssign(string s) {
    stringstream input;
    input << s;
    string res, a, b;
    string toMem, asnVal;
    while (input >> res) {
        if (res == "=") {
            continue;
        }
        if (a.empty()) {
            a = res;
            string tmpa = getAddr(a, "$t8", false);
            if (tmpa == "$t8") toMem = getAddr(a, "", false);
            a = tmpa;
        } else {
            if (isNum(res) && toMem.empty()) {
                generatedCode.push_back("li " + a + ", " + res);
            } else if (isNum(res) && !toMem.empty()) {
                generatedCode.push_back("li $t8, " + res);
                generatedCode.push_back("sw $t8, " + toMem);
            } else {
                b = res;
                if (b == "RET") {
                    b = "$v0";
                } else {
                    string tmpb = getAddr(b, "$t9", false);
                    if (tmpb == "$t9") asnVal = getAddr(b, "", false);  // 0($xp) = toMem || b
                    b = tmpb;
                }

                if (asnVal.empty() && toMem.empty()) {  // $t = $t
                    generatedCode.push_back("move " + a + ", " + b);
                } else if (asnVal.empty() && !toMem.empty()) {  // 0($xp) = $t
                    generatedCode.push_back("sw " + b + ", " + toMem);
                } else if (!asnVal.empty() && toMem.empty()) {  // $t = 0($xp)
                    generatedCode.push_back("lw " + a + ", " + asnVal);
                } else if (!asnVal.empty() && !toMem.empty()) { // 0($xp) = 0($xp)
                    generatedCode.push_back("lw $t9, " + asnVal);
                    generatedCode.push_back("sw $t9, " + toMem);
                }
            }
        }
    }
}

void mipsGenGetint(string s) {
    int start = 4;
    while (start < s.size()) {
        if (s[start] != ' ') break;
        start++;
    }
    string tmp, toMem;
    for (int j = start; j < s.size(); j++) {
        tmp += s[j];
    }
    string ttmp = getAddr(tmp, "$t8", false);
    if (ttmp == "$t8") toMem = getAddr(tmp, "", false);
    tmp = ttmp;

    generatedCode.push_back("# read %s " + tmp);
    generatedCode.emplace_back("li $v0, 5");
    generatedCode.emplace_back("syscall");
    if (!toMem.empty()) { // move $t8, $v0
        generatedCode.push_back("sw $v0, " + toMem);
    } else {
        generatedCode.push_back("move " + tmp + ", $v0");
    }
}

void mipsGen() {
    generatedCode.emplace_back(".data");

    for (int i = 0; i < middleCode.size(); i++) {
        string s = middleCode[i];
        if (s.empty()) continue;
        s.erase(std::remove(s.begin(), s.end(), '\n'), s.end());

        // string
        if (s.substr(0, 12) == "const string") {
            int start = 12;

            while (start < s.size()) {
                if (s[start] != ' ') break;
                start++;
            }
            string tmp1;
            for (int j = start; j <= s.size(); j++) {
                start = j;
                if (s[j] == ' ') break;
                tmp1 += s[j];
            }

            while (start < s.size()) {
                if (s[start] == '"') break;
                start++;
            }
            string tmp2;
            for (int j = start; j <= s.size(); j++) {
                tmp2 += s[j];
                if (j != start && s[j] == '"') break;
            }
            generatedCode.push_back(tmp1 + ": .asciiz " + tmp2);
            stringPrint.insert(tmp1);
            generatedStatus[i] = true;
            continue;
        }
    }

    generatedCode.emplace_back(".text");
    generatedCode.emplace_back("li $fp, 0x10040000");
    // generatedCode.push_back("addi $fp, $fp, " + to_string(fpBias));

    // global
    for (int i = 0; i < middleCode.size(); i++) {
        string s = middleCode[i];
        if (s.empty()) continue;
        if (generatedStatus[i]) continue;
        s.erase(std::remove(s.begin(), s.end(), '\n'), s.end());

        if (s == "MAIN:") {
            break;
        }

        // def
        if (s.substr(0, 7) == "var int" || s.substr(0, 9) == "const int") {
            generatedStatus[i] = true;
            newVar(s, 1);
            continue;
        }

        // func
        if (s.substr(0, 9) == "func void" || s.substr(0, 8) == "func int") {
            while (i < middleCode.size()) {
                if (middleCode[i].substr(0, 3) == "ret") {
                    break;
                }
                i++;
            }
            continue;
        }

        // push and call
        if (s.substr(0, 4) == "push" || s.substr(0, 4) == "call") {
            vector<string> params;

            // push to heap
            int k = 0, paramCnt = 0;
            while (i < middleCode.size() && middleCode[i].substr(0, 4) == "push") {
                generatedStatus[i] = true;
                s = middleCode[i];
                int start = 4;
                while (start < s.size()) {
                    if (s[start] != ' ') break;
                    start++;
                }
                string tmp;
                for (int j = start; j < s.size(); j++) {
                    tmp += s[j];
                }
                params.push_back(tmp);

                // sw tmp, i*4($fp)
                string tmpt = getAddr(tmp, "$t8", false), toMem;
                if (tmpt == "$t8") toMem = getAddr(tmp, "", false);
                tmp = tmpt;

                if (toMem.empty()) { // sw $t, i*4($fp)
                    if (isNum(tmp)) {
                        generatedCode.push_back("li $t8, " + tmp);
                        generatedCode.push_back("sw $t8, " + to_string(k * 4) + "($fp)");
                    } else {
                        generatedCode.push_back("sw " + tmp + ", " + to_string(k * 4) + "($fp)");
                        paramCnt++;
                    }
                } else {
                    generatedCode.push_back("lw $t8, " + toMem);
                    generatedCode.push_back("sw $t8, " + to_string(k * 4) + "($fp)");
                    paramCnt++;
                }
                i++;
                k++;
            }


            // save to stack
            generatedCode.push_back("addi $sp, $sp, -" + to_string((paramCnt + 1) * 4));

            paramCnt = 0;
            for (k = 0; k < params.size(); k++) {
                string tmp = params[k];
                string tmpt = getAddr(tmp, "$t8", false), toMem;
                if (tmpt == "$t8") toMem = getAddr(tmp, "", false);
                tmp = tmpt;

                if (toMem.empty()) { // sw $t, k*4($sp)
                    if (isNum(tmp)) {
                        // do nothing
//                      you don't need to save a number to a stack, do you?
//                      generatedCode.push_back("li $t8, " + tmp);
//                      generatedCode.push_back("sw $t8, " + to_string(paramCnt * 4) + "($sp)");
                    } else {
                        generatedCode.push_back("sw " + tmp + ", " + to_string(paramCnt * 4) + "($sp)");
                        paramCnt++;
                    }
                } else {
                    generatedCode.push_back("lw $t8, " + toMem);
                    generatedCode.push_back("sw $t8, " + to_string(paramCnt * 4) + "($sp)");
                    paramCnt++;
                }
            }
            generatedCode.push_back("sw $ra, " + to_string(paramCnt * 4) + "($sp)");
            if (!params.empty()) {
                generatedCode.push_back("addi $fp, $fp, " + to_string(params.size() * 4));
                fpBias += params.size() * 4;
            }

            //call
            s = middleCode[i];
            if (s.substr(0, 4) == "call") {
                int start = 4;
                while (start < s.size()) {
                    if (s[start] != ' ') break;
                    start++;
                }
                string tmp;
                for (int j = start; j < s.size(); j++) {
                    tmp += s[j];
                }
                generatedCode.push_back("jal " + tmp);
            }

            // recover from stack
            if (!params.empty()) {
                generatedCode.push_back("addi $fp, $fp, -" + to_string(params.size() * 4));
                fpBias -= params.size() * 4;
            }

            paramCnt = 0;
            for (k = 0; k < params.size(); k++) {
                string tmp = params[k];
                string tmpt = getAddr(tmp, "$t8", false), toMem;
                if (tmpt == "$t8") toMem = getAddr(tmp, "", false);
                tmp = tmpt;

                if (toMem.empty()) { // sw $t, k*4($fp)
                    if (isNum(tmp)) {
                        // nothing to do
                    } else {
                        generatedCode.push_back("lw " + tmp + ", " + to_string(paramCnt * 4) + "($sp)");
                        paramCnt++;
                    }
                } else {
                    generatedCode.push_back("lw $t8, " + to_string(paramCnt * 4) + "($sp)");
                    generatedCode.push_back("sw $t8, " + toMem);
                    paramCnt++;
                }
            }
            generatedCode.push_back("lw $ra, " + to_string(paramCnt * 4) + "($sp)");
            generatedCode.push_back("addi $sp, $sp, " + to_string((paramCnt + 1) * 4));
            generatedStatus[i] = true;
            continue;
        }

        // exp
        if (s[0] == '+' || s[0] == '-' || s[0] == '*' || s[0] == '/' || s[0] == '%' || s[0] == '!') {
            mipsGenExp(s);
            generatedStatus[i] = true;
            continue;
        }

        // assign
        if (s.find('=') != -1) {
            mipsGenAssign(s);
            generatedStatus[i] = true;
            continue;
        }

        // getint
        if (s.substr(0, 4) == "read") {
            mipsGenGetint(s);
            generatedStatus[i] = true;
            continue;
        }

        // j
        if (s.substr(0, 2) == "j ") {
            generatedCode.push_back(s);
            generatedCode.push_back("nop");
            generatedStatus[i] = true;
            continue;
        }

        // label
        if (s.find(":") != -1) {
            generatedCode.push_back(s);
            generatedStatus[i] = true;
            continue;
        }
    }

    generatedCode.emplace_back("j main");
    generatedCode.emplace_back("nop");

    // main
    for (int i = 0; i < middleCode.size(); i++) {
        string s = middleCode[i];
        if (s.empty()) continue;
        if (generatedStatus[i]) continue;
        generatedStatus[i] = true;
        s.erase(std::remove(s.begin(), s.end(), '\n'), s.end());

        // def
        if (s.substr(0, 7) == "var int" || s.substr(0, 9) == "const int") {
            newVar(s, 0);
            continue;
        }

        // main {
        if (s == "MAIN:") {
            generatedCode.emplace_back("main:");
            continue;
        }

        // func
        if (s.substr(0, 9) == "func void" || s.substr(0, 8) == "func int") {
            int start = 9;
            while (start < s.size()) {
                if (s[start] != ' ') break;
                start++;
            }
            string tmp;
            for (int j = start; j < s.size(); j++) {
                tmp += s[j];
            }
            generatedCode.push_back(tmp + ":");

            i++;
            string blockNumAtFunc = middleCode[i]; // @x

            vector<string> params;
            vector<string> paramTypes;
            while (i + 1 < middleCode.size() && middleCode[i + 1].substr(0, 6) == "param ") {
                i++;
                s = middleCode[i];
                stringstream input;
                input << s;
                string type, name, res;
                while (input >> res) {
                    if (res == "param") continue;
                    if (type.empty()) type = res;
                    else if (name.empty()) name = res;
                }

                paramTypes.push_back(type);
                params.push_back(name);
                newParam(name);
            }

            // params
            int paramCnt = params.size();
            for (auto &param: params) {
                tmp = param;
                string toMem;

                // lw tmp, (paramCnt*4)($fp)
                string tmpt = getAddr(tmp, "$t8", false);
                if (tmpt == "$t8") toMem = getAddr(tmp, "", false);
                tmp = tmpt;

                if (toMem.empty()) { // lw $tmp, (paramCnt*4)($fp)
                    generatedCode.push_back("lw " + tmp + ", " + to_string(-paramCnt * 4) + "($fp)");
                    // TODO
                } else {    // lw $t8, ()  sw $t8, toMem
                    generatedCode.push_back("lw $t8, " + to_string(-paramCnt * 4) + "($fp)");
                    generatedCode.push_back("sw $t8, " + toMem);
                }
                paramCnt--;
            }
            continue;
        }

        // push and call TODO
        if (s.substr(0, 4) == "push" || s.substr(0, 4) == "call") {
            vector<string> params;
            vector<bool> isAddr;

            // push to heap
            int k = 0, paramCnt = 0;
            while (i < middleCode.size() && middleCode[i].substr(0, 4) == "push") {
                s = middleCode[i];
                int start = 4, paramIsAddr = 0;
                while (start < s.size()) {
                    if (s[start] != ' ') break;
                    start++;
                }
                string tmp;
                for (int j = start; j < s.size(); j++) {
                    tmp += s[j];
                }
                if (tmp.find("@addr") != -1) {
                    tmp = tmp.substr(0, tmp.size() - 5);
                    isAddr.push_back(true);
                    paramIsAddr = true;
                } else {
                    isAddr.push_back(false);
                }
                params.push_back(tmp);

                // sw tmp, i*4($fp)
                string tmpt = getAddr(tmp, "$t8", false), toMem;
                if (tmpt == "$t8") toMem = getAddr(tmp, "", false);
                tmp = tmpt;

                if (toMem.empty()) { // sw $t, i*4($fp)
                    generatedCode.push_back("sw " + tmp + ", " + to_string(k * 4) + "($fp)");
                } else {
                    if (paramIsAddr) {
                        generatedCode.push_back("li $t8, " + toMem);
                        generatedCode.push_back("sw $t8, " + to_string(k * 4) + "($fp)");
                    } else {
                        generatedCode.push_back("lw $t8, " + toMem);
                        generatedCode.push_back("sw $t8, " + to_string(k * 4) + "($fp)");
                    }
                }
                paramCnt++;
                i++;
                k++;
            }

            // save to stack
            generatedCode.push_back("addi $sp, $sp, -" + to_string((paramCnt + 1) * 4));

            paramCnt = 0;
            for (k = 0; k < params.size(); k++) {
                string tmp = params[k];
                string tmpt = getAddr(tmp, "$t8", false), toMem;
                if (tmpt == "$t8") toMem = getAddr(tmp, "", false);
                tmp = tmpt;

                if (toMem.empty()) { // sw $t, k*4($sp)
                    if (isNum(tmp)) {
                        // do nothing
//                      you don't need to save a number to a stack, do you?
//                      generatedCode.push_back("li $t8, " + tmp);
//                      generatedCode.push_back("sw $t8, " + to_string(paramCnt * 4) + "($sp)");
                    } else {
                        generatedCode.push_back("sw " + tmp + ", " + to_string(paramCnt * 4) + "($sp)");
                        paramCnt++;
                    }
                } else {
                    generatedCode.push_back("lw $t8, " + toMem);
                    generatedCode.push_back("sw $t8, " + to_string(paramCnt * 4) + "($sp)");
                    paramCnt++;
                }
            }
            generatedCode.push_back("sw $ra, " + to_string(paramCnt * 4) + "($sp)");
            if (!params.empty()) {
                generatedCode.push_back("addi $fp, $fp, " + to_string(params.size() * 4));
                fpBias += params.size() * 4;
            }

            //call
            s = middleCode[i];
            if (s.substr(0, 4) == "call") {
                int start = 4;
                while (start < s.size()) {
                    if (s[start] != ' ') break;
                    start++;
                }
                string tmp;
                for (int j = start; j < s.size(); j++) {
                    tmp += s[j];
                }
                generatedCode.push_back("jal " + tmp);
            }

            // recover from stack
            if (!params.empty()) {
                generatedCode.push_back("addi $fp, $fp, -" + to_string(params.size() * 4));
                fpBias -= params.size() * 4;
            }

            paramCnt = 0;
            for (k = 0; k < params.size(); k++) {
                string tmp = params[k];
                string tmpt = getAddr(tmp, "$t8", false), toMem;
                if (tmpt == "$t8") toMem = getAddr(tmp, "", false);
                tmp = tmpt;

                if (toMem.empty()) { // sw $t, k*4($fp)
                    if (isNum(tmp)) {
                        // nothing to do
                    } else {
                        generatedCode.push_back("lw " + tmp + ", " + to_string(paramCnt * 4) + "($sp)");
                        paramCnt++;
                    }
                } else {
                    generatedCode.push_back("lw $t8, " + to_string(paramCnt * 4) + "($sp)");
                    generatedCode.push_back("sw $t8, " + toMem);
                    paramCnt++;
                }
            }
            generatedCode.push_back("lw $ra, " + to_string(paramCnt * 4) + "($sp)");
            generatedCode.push_back("addi $sp, $sp, " + to_string((paramCnt + 1) * 4));
            continue;
        }

        // ret
        if (s.substr(0, 3) == "ret") {
            if (s.size() > 3) {
                int start = 3;
                while (start < s.size()) {
                    if (s[start] != ' ') break;
                    start++;
                }
                string tmp;
                for (int j = start; j < s.size(); j++) {
                    tmp += s[j];
                }
                tmp = getAddr(tmp, "$v0", true);
                if (tmp != "$v0") {
                    if (isNum(tmp)) {
                        generatedCode.push_back("li $v0, " + tmp);
                    } else {
                        generatedCode.push_back("move $v0, " + tmp);
                    }
                }
            }

            generatedCode.emplace_back("jr $ra");
            continue;
        }

        // exp
        if (s[0] == '+' || s[0] == '-' || s[0] == '*' || s[0] == '/' || s[0] == '%' || s[0] == '!') {
            mipsGenExp(s);
            generatedStatus[i] = true;
            continue;
        }

        // assign
        if (s.find('=') != -1) {
            mipsGenAssign(s);
            continue;
        }

        // getint
        if (s.substr(0, 4) == "read") {
            mipsGenGetint(s);
            continue;
        }

        // printf
        if (s.substr(0, 6) == "printf") {
            int start = 6;
            while (start < s.size()) {
                if (s[start] != ' ') break;
                start++;
            }
            string tmp;
            for (int j = start; j < s.size(); j++) {
                tmp += s[j];
            }
            generatedCode.push_back("# printf " + tmp);
            if (stringPrint.count(tmp)) {
                generatedCode.emplace_back("li $v0, 4");
                generatedCode.push_back("la $a0, " + tmp);
                generatedCode.emplace_back("syscall");
            } else {
                tmp = getAddr(tmp, "$t8", true);
                generatedCode.emplace_back("li $v0, 1");
                if (isNum(tmp)) {
                    generatedCode.push_back("li $a0, " + tmp);
                } else {
                    generatedCode.push_back("move $a0, " + tmp);
                }
                generatedCode.emplace_back("syscall");
            }
            continue;
        }

        // if
        if (s.substr(0, 3) == "beq") {
            stringstream input;
            input << s;
            string toMem, res, a, b;
            while (input >> res) {
                if (res == "beq" || res == "$0") continue;
                if (a.empty()) a = res;
                else b = res;
            }
            a = getAddr(a, "$t8", true);
            generatedCode.push_back("beq $0, " + a + ", " + b);
            generatedCode.push_back("nop");
            generatedStatus[i] = true;
            continue;
        }

        // cond
        if (s.substr(0, 3) == "slt" || s.substr(0, 3) == "sgt" || s.substr(0, 3) == "sle"
            || s.substr(0, 3) == "sge" || s.substr(0, 3) == "seq" || s.substr(0, 3) == "sne") {
            stringstream input;
            input << s;
            string toMem, res, op, a, b, c;
            while (input >> res) {
                if (op.empty()) op = res;
                else if (a.empty()) a = res;
                else if (b.empty()) b = res;
                else c = res;
            }
            a = getAddr(a, "$t8", true);
            b = getAddr(b, "$t9", true);
            string tmpc = getAddr(c, "$t8", false);
            string endc = getAddr(c, "", false);
            if (tmpc == "$t8") toMem = getAddr(c, "", false);
            c = tmpc;
            if (isNum(a)) {
                if (a == "0") {
                    a = "$0";
                } else {
                    generatedCode.push_back("li $t8, " + a);
                    a = "$t8";
                }
            }
            if (isNum(b)) {
                if (b == "0") {
                    b = "$0";
                } else {
                    generatedCode.push_back("li $t9, " + b);
                    b = "$t9";
                }
            }
            generatedCode.push_back(op + " " + c + ", " + a + ", " + b);

            if (tmpc == "$t8") {
                generatedCode.push_back("sw $t8, " + endc);
            }
            generatedStatus[i] = true;
            continue;
        }

        // j
        if (s.substr(0, 2) == "j ") {
            generatedCode.push_back(s);
            generatedCode.push_back("nop");
            continue;
        }

        // label
        if (s.find(":") != -1) {
            generatedCode.push_back(s);
            continue;
        }
    }

    generatedCode.emplace_back("li $v0, 10");
    generatedCode.emplace_back("syscall");
}