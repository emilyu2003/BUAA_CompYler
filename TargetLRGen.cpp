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
//    middleCode.pop_back();  // return 0
//    middleCode.pop_back();  // t_0 = 0
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
            ssaIdents[i].addr = to_string(fpBias) + "($fp)"; // assign when running TODO
            fpBias += 4;
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
    //for (const auto &s: middleCode) {
    //cout << ">" << s << "<" << endl;
    //}
}

string getAddr(string str, string target, bool print) {
    if (str == "$0") return str;
    if (str.find("$t") != -1) return str;
    if (isNum(str)) return str;

    int pos = getSsaIdentPos(str);
    if (pos == -1) {
        T_NAME tName = {str};
        ssaIdents.push_back(tName);
        ssaIdents.back().addr = to_string(gpBias) + "($gp)";
        gpBias += 4;
        pos = getSsaIdentPos(str);
    } else if (ssaIdents[pos].addr.empty()) {
//        ssaIdents[pos].addr = to_string(fpBias) + "($fp)";
//        fpBias += 4;
    }
    string ret = ssaIdents[pos].addr;

    if (ret.find("($f") != -1) {
        string num;
        for (char i: ret) {
            if (i == '(') break;
            num += i;
        }
        int tmpBias = stoi(num);
        int realBias = tmpBias - fpBias;
        ret = to_string(realBias) + "($fp)";
    }

    if (ret.find("($") != -1 && !target.empty()) {
        if (print) {
            generatedCode.push_back("lw " + target + ", " + ret);
        }
        return target;
    }
    return ret;
}

void mipsGen() {
    generatedCode.emplace_back(".data");

    for (int i = 0; i < middleCode.size() - 1; i++) {
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
        }
    }

    generatedCode.emplace_back(".text");
    generatedCode.emplace_back("li $fp, 0x10040000");
    generatedCode.push_back("addi $fp, $fp, " + to_string(fpBias));
    generatedCode.emplace_back("j main");
    generatedCode.emplace_back("nop");

    for (int i = 0; i < middleCode.size() - 1; i++) {
        string s = middleCode[i];
        if (s.empty()) continue;
        if (generatedStatus[i]) continue;
        generatedStatus[i] = true;
        s.erase(std::remove(s.begin(), s.end(), '\n'), s.end());

        // def
        if (s.substr(0, 7) == "var int" || s.substr(0, 7) == "const int") {

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

            // params
            int paramCnt = 0;
            while (i + 1 < middleCode.size() && middleCode[i + 1].substr(0, 9) == "param int") {
                i++;
                s = middleCode[i];
                start = 9;
                tmp.clear();
                string toMem, tmpt;
                while (start < s.size()) {
                    if (s[start] != ' ') break;
                    start++;
                }
                for (int j = start; j < s.size(); j++) {
                    tmp += s[j];
                }

                // lw tmp, (paramCnt*4)($fp)
                tmpt = getAddr(tmp, "$t8", false);
                if (tmpt == "$t8") toMem = getAddr(tmp, "", false);
                tmp = tmpt;

                if (toMem.empty()) { // lw $tmp, (paramCnt*4)($fp)
                    generatedCode.push_back("lw " + tmp + ", " + to_string(paramCnt * 4) + "($fp)");
                } else {    // lw $t8, ()  sw $t8, toMem
                    generatedCode.push_back("lw $t8, " + to_string(paramCnt * 4) + "($fp)");
                    generatedCode.push_back("sw $t8, " + toMem);
                }
                paramCnt++;
            }
            continue;
        }

        // push and call
        if (s.substr(0, 4) == "push" || s.substr(0, 4) == "call") {
            vector<string> params;

            // push to heap
            while (i < middleCode.size() && middleCode[i].substr(0, 4) == "push") {
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
                    generatedCode.push_back("sw " + tmp + ", " + to_string(i * 4) + "($fp)");
                } else {
                    generatedCode.push_back("lw $t8, " + toMem);
                    generatedCode.push_back("sw $t8, " + to_string(i * 4) + "($fp)");
                }
                i++;
            }


            // save to stack
            generatedCode.push_back("addi $sp, $sp, -" + to_string((params.size() + 1) * 4));

            for (int k = 0; k < params.size(); k++) {
                string tmp = params[k];
                string tmpt = getAddr(tmp, "$t8", false), toMem;
                if (tmpt == "$t8") toMem = getAddr(tmp, "", false);
                tmp = tmpt;

                if (toMem.empty()) { // sw $t, k*4($fp)
                    generatedCode.push_back("lw " + tmp + ", " + to_string(k * 4) + "($sp)");
                } else {
                    generatedCode.push_back("lw $t8, " + toMem);
                    generatedCode.push_back("sw $t8, " + to_string(k * 4) + "($sp)");
                }
            }
            generatedCode.push_back("sw $ra, " + to_string(params.size() * 4) + "($sp)");
            if (params.size()) {
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
            if (params.size()) {
                generatedCode.push_back("addi $fp, $fp, -" + to_string(params.size() * 4));
                fpBias -= params.size() * 4;
            }

            for (int k = 0; k < params.size(); k++) {
                string tmp = params[k];
                string tmpt = getAddr(tmp, "$t8", false), toMem;
                if (tmpt == "$t8") toMem = getAddr(tmp, "", false);
                tmp = tmpt;

                if (toMem.empty()) { // sw $t, k*4($fp)
                    generatedCode.push_back("lw " + tmp + to_string(k * 4) + "($sp)");
                } else {
                    generatedCode.push_back("lw $t8, " + to_string(k * 4) + "($sp)");
                    generatedCode.push_back("sw $t8, " + toMem);
                }
            }
            generatedCode.push_back("lw $ra, " + to_string(params.size() * 4) + "($sp)");
            generatedCode.push_back("addi $sp, $sp, " + to_string((params.size() + 1) * 4));
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
                tmp = getAddr(tmp, "$v0", false);
                if (tmp != "$v0") {
                    if (isNum(tmp)) {
                        generatedCode.push_back("li $v0, " + tmp);
                    } else {
                        generatedCode.push_back("move $v0, " + tmp);
                    }
                }
            }

            generatedCode.emplace_back("jr $ra");
        }

        // exp
        if (s[0] == '+' || s[0] == '-' || s[0] == '*' || s[0] == '/' || s[0] == '%') {
            string a, b, c, res;
            stringstream input;
            input << s;
            string toMem;
            while (input >> res) {
                if (res == "+" || res == "-" || res == "*" || res == "/" || res == "%") continue;
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
                generatedCode.push_back("add " + c + ", " + a + ", " + b);
            } else if (s[0] == '-') {
                if (isNum(a)) {
                    generatedCode.push_back("li $t8, " + a);
                    a = "$t8";
                }
                generatedCode.push_back("sub " + c + ", " + a + ", " + b);
            } else if (s[0] == '*') {
                generatedCode.push_back("mul " + c + ", " + a + ", " + b);
            } else if (s[0] == '/') {
                if (isNum(a)) {
                    generatedCode.push_back("li $t8, " + a);
                    a = "$t8";
                }
                generatedCode.push_back("div " + a + ", " + b);
                generatedCode.push_back("mflo " + c);
            } else if (s[0] == '%') {
                if (isNum(a)) {
                    generatedCode.push_back("li $t8, " + a);
                    a = "$t8";
                }
                generatedCode.push_back("div " + a + ", " + b);
                generatedCode.push_back("mfhi " + c);
            }

            if (tmpc == "$t8") {
                generatedCode.push_back("sw $t8, " + endc);
            }
            continue;
        }

        // assign
        if (s.find('=') != -1) {
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
            continue;
        }

        // getint
        if (s.substr(0, 4) == "read") {
            int start = 4;
            while (start < s.size()) {
                if (s[start] != ' ') break;
                start++;
            }
            string tmp;
            for (int j = start; j < s.size(); j++) {
                tmp += s[j];
            }
            tmp = getAddr(tmp, "$t8", false);

            generatedCode.push_back("# read %s " + tmp);
            generatedCode.emplace_back("li $v0, 5");
            generatedCode.emplace_back("syscall");
            if (tmp != "$t8") {
                generatedCode.push_back("move " + tmp + ", $v0");
            } else {
                generatedCode.push_back("sw $v0, " + tmp);
            }
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
                generatedCode.push_back("la $a0,  " + tmp);
                generatedCode.emplace_back("syscall");
            } else {
                tmp = getAddr(tmp, "$t8", true);
                generatedCode.emplace_back("li $v0, 1");
                generatedCode.push_back("move $a0,  " + tmp);
                generatedCode.emplace_back("syscall");
            }
            continue;
        }
    }
    generatedCode.emplace_back("li $v0, 10");
    generatedCode.emplace_back("syscall");
}