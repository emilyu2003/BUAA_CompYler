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
bool generatedCode[100005];
int fpBias = 0;
int gpBias = 0;

// allocate register
void initGenerator() {
    middleCode.pop_back();  // return 0
    middleCode.pop_back();  // t_0 = 0
    memset(generatedCode, 0, sizeof(generatedCode));

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
    for (const auto &s: middleCode) {
        cout << ">" << s << "<" << endl;
    }
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
            FILE *f = fopen("mips.txt", "a");
            fprintf(f, "lw %s, %s\n", target.c_str(), ret.c_str());
            fclose(f);
        }
        return target;
    }
    return ret;
}

void mipsGen() {
    FILE *f = fopen("mips.txt", "a");
    fprintf(f, ".data\n");
    fclose(f);

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

            f = fopen("mips.txt", "a");
            fprintf(f, "%s: .asciiz %s\n", tmp1.c_str(), tmp2.c_str());
            fclose(f);
            stringPrint.insert(tmp1);
            generatedCode[i] = true;
        }
    }

    f = fopen("mips.txt", "a");
    fprintf(f, "\n.text\n");
    fprintf(f, "li $fp, 0x10040000\n");
    fprintf(f, "addi $fp, $fp, %d\n", fpBias);
    fprintf(f, "\nj main\nnop\n\n");
    fclose(f);

    for (int i = 0; i < middleCode.size(); i++) {
        string s = middleCode[i];
        if (s.empty()) continue;
        if (generatedCode[i]) continue;
        generatedCode[i] = true;
        s.erase(std::remove(s.begin(), s.end(), '\n'), s.end());

        // def
        if (s.substr(0, 7) == "var int" || s.substr(0, 7) == "const int") {

            continue;
        }

        // main {
        if (s == "MAIN:") {
            f = fopen("mips.txt", "a");
            fprintf(f, "\nmain:\n");
            fclose(f);
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
            f = fopen("mips.txt", "a");
            fprintf(f, "\n%s:\n", tmp.c_str());
            fclose(f);

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
                    f = fopen("mips.txt", "a");
                    fprintf(f, "lw %s, %d($fp)\n", tmp.c_str(), paramCnt * 4);
                    fclose(f);
                } else {    // lw $t8, ()  sw $t8, toMem
                    f = fopen("mips.txt", "a");
                    fprintf(f, "lw $t8, %d($fp)\nsw $t8, %s\n", paramCnt * 4, toMem.c_str());
                    fclose(f);
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
                    f = fopen("mips.txt", "a");
                    fprintf(f, "sw %s, %d($fp)\n", tmp.c_str(), i * 4);
                    fclose(f);
                } else {
                    f = fopen("mips.txt", "a");
                    fprintf(f, "lw $t8, %s\nsw $t8, %d($fp)\n", toMem.c_str(), i * 4);
                    fclose(f);
                }

                i++;
            }


            // save to stack
            f = fopen("mips.txt", "a");
            fprintf(f, "\naddi $sp, $sp, -%llu\n", (params.size() + 1) * 4);
            fclose(f);

            for (int k = 0; k < params.size(); k++) {
                string tmp = params[k];
                string tmpt = getAddr(tmp, "$t8", false), toMem;
                if (tmpt == "$t8") toMem = getAddr(tmp, "", false);
                tmp = tmpt;

                if (toMem.empty()) { // sw $t, k*4($fp)
                    f = fopen("mips.txt", "a");
                    fprintf(f, "lw %s, %d($sp)\n", tmp.c_str(), k * 4);
                    fclose(f);
                } else {
                    f = fopen("mips.txt", "a");
                    fprintf(f, "lw $t8, %s\nsw $t8, %d($sp)\n", toMem.c_str(), k * 4);
                    fclose(f);
                }
            }
            f = fopen("mips.txt", "a");
            fprintf(f, "\nsw $ra, %llu($sp)\n", params.size() * 4);
            fclose(f);
            if (params.size()) {
                f = fopen("mips.txt", "a");
                fprintf(f, "\naddi $fp, $fp, %llu\n", params.size() * 4);
                fclose(f);
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
                f = fopen("mips.txt", "a");
                fprintf(f, "jal %s\n", tmp.c_str());
                fclose(f);
            }

            // recover from stack
            if (params.size()) {
                f = fopen("mips.txt", "a");
                fprintf(f, "\naddi $fp, $fp, -%llu\n", params.size() * 4);
                fclose(f);
                fpBias -= params.size() * 4;
            }

            for (int k = 0; k < params.size(); k++) {
                string tmp = params[k];
                string tmpt = getAddr(tmp, "$t8", false), toMem;
                if (tmpt == "$t8") toMem = getAddr(tmp, "", false);
                tmp = tmpt;

                if (toMem.empty()) { // sw $t, k*4($fp)
                    f = fopen("mips.txt", "a");
                    fprintf(f, "lw %s, %d($sp)\nsw $t8, %s\n", tmp.c_str(), k * 4);
                    fclose(f);
                } else {
                    f = fopen("mips.txt", "a");
                    fprintf(f, "lw $t8, %d($sp)\nsw $t8, %s\n", k * 4, toMem.c_str());
                    fclose(f);
                }
            }
            f = fopen("mips.txt", "a");
            fprintf(f, "lw $ra, %llu($sp)\n", params.size() * 4);
            fprintf(f, "addi $sp, $sp, %llu\n", (params.size() + 1) * 4);
            fclose(f);

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
                    f = fopen("mips.txt", "a");
                    if (isNum(tmp)) {
                        fprintf(f, "li $v0, %s\n", tmp.c_str());
                    } else {
                        fprintf(f, "move $v0, %s\n", tmp.c_str());
                    }
                    fclose(f);
                }
            }

            f = fopen("mips.txt", "a");
            fprintf(f, "jr $ra\n");
            fclose(f);
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
                f = fopen("mips.txt", "a");
                fprintf(f, "add %s, %s, %s\n", c.c_str(), a.c_str(), b.c_str());
                fclose(f);
            } else if (s[0] == '-') {
                if (isNum(a)) {
                    f = fopen("mips.txt", "a");
                    fprintf(f, "li $t8, %s\n", a.c_str());
                    fclose(f);
                    a = "$t8";
                }
                f = fopen("mips.txt", "a");
                fprintf(f, "sub %s, %s, %s\n", c.c_str(), a.c_str(), b.c_str());
                fclose(f);
            } else if (s[0] == '*') {
                f = fopen("mips.txt", "a");
                fprintf(f, "mul %s, %s, %s\n", c.c_str(), a.c_str(), b.c_str());
                fclose(f);
            } else if (s[0] == '/') {
                if (isNum(a)) {
                    f = fopen("mips.txt", "a");
                    fprintf(f, "li $t8, %s\n", a.c_str());
                    fclose(f);
                    a = "$t8";
                }
                f = fopen("mips.txt", "a");
                fprintf(f, "div %s, %s\n", a.c_str(), b.c_str());
                fprintf(f, "mflo %s\n", c.c_str());
                fclose(f);
            } else if (s[0] == '%') {
                if (isNum(a)) {
                    f = fopen("mips.txt", "a");
                    fprintf(f, "li $t8, %s\n", a.c_str());
                    fclose(f);
                    a = "$t8";
                }
                f = fopen("mips.txt", "a");
                fprintf(f, "add %s, %s\n", a.c_str(), b.c_str());
                fprintf(f, "mfhi %s\n", c.c_str());
                fclose(f);
                if (!toMem.empty()) {
                    f = fopen("mips.txt", "a");
                    fprintf(f, "sw %s, %s\n", c.c_str(), toMem.c_str());
                    fclose(f);
                }
            }

            if (tmpc == "$t8") {
                f = fopen("mips.txt", "a");
                fprintf(f, "sw $t8, %s\n", endc.c_str());
                fclose(f);
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
                        f = fopen("mips.txt", "a");
                        fprintf(f, "li %s, %s\n", a.c_str(), res.c_str());
                        fclose(f);
                    } else if (isNum(res) && !toMem.empty()) {
                        f = fopen("mips.txt", "a");
                        fprintf(f, "li $t8, %s\nsw $t8, %s\n", res.c_str(), toMem.c_str());
                        fclose(f);
                    } else {
                        b = res;
                        if (b == "RET") {
                            b = "$v0";
                        } else {
                            string tmpb = getAddr(b, "$t9", false);
                            if (tmpb == "$t9") asnVal = getAddr(b, "", false);  // 0($xp) = toMem || b
                            b = tmpb;
                        }

                        f = fopen("mips.txt", "a");
                        if (asnVal.empty() && toMem.empty()) {  // $t = $t
                            fprintf(f, "move %s, %s\n", a.c_str(), b.c_str());
                        } else if (asnVal.empty() && !toMem.empty()) {  // 0($xp) = $t
                            fprintf(f, "sw %s, %s\n", b.c_str(), toMem.c_str());
                        } else if (!asnVal.empty() && toMem.empty()) {  // $t = 0($xp)
                            fprintf(f, "lw %s, %s\n", a.c_str(), asnVal.c_str());
                        } else if (!asnVal.empty() && !toMem.empty()) { // 0($xp) = 0($xp)
                            fprintf(f, "lw $t9, %s\n", asnVal.c_str());
                            fprintf(f, "sw $t9, %s\n", toMem.c_str());
                        }
                        fclose(f);
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
            f = fopen("mips.txt", "a");
            fprintf(f, "\n# read %s\n", tmp.c_str());
            fprintf(f, "li $v0, 5\n");
            fprintf(f, "syscall\n");
            if (tmp != "$t8") {
                fprintf(f, "move %s, $v0\n", tmp.c_str());
            } else {
                fprintf(f, "sw $v0, %s\n", tmp.c_str());
            }
            fclose(f);
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
            f = fopen("mips.txt", "a");
            fprintf(f, "\n# printf %s\n", tmp.c_str());
            if (stringPrint.count(tmp)) {
                fprintf(f, "li $v0, 4\n");
                fprintf(f, "la $a0, %s\n", tmp.c_str());
                fprintf(f, "syscall\n");
            } else {
                tmp = getAddr(tmp, "$t8", true);
                fprintf(f, "li $v0, 1\n");
                fprintf(f, "move $a0, %s\n", tmp.c_str());
                fprintf(f, "syscall\n");
            }
            fclose(f);
            continue;
        }
    }

    f = fopen("mips.txt", "a");
    fprintf(f, "\nli $v0, 10\nsyscall\n");
    fclose(f);

}