//
// Created by emilyu on 2022/10/17.
//
#include "IdentTable.h"
#include "symbol.h"
#include "base.h"
#include <cstdio>
#include <algorithm>
#include <iostream>
#include <utility>
#include <vector>

using namespace std;

vector<IDENT> identTable;
vector<int> identTableCnt;
int maxBlockNum = 0;
vector<int> tmpBlockNums;
vector<string> totalName;

int getBlockNum() {
    //printTotalTable();
    return tmpBlockNums.back();
}

void newBlock() {
    maxBlockNum++;
    tmpBlockNums.push_back(maxBlockNum);
}

void updateValue(IDENT ident) {
    int pos = getIdentPos(ident.name);
    identTable[pos].value_valid = 1;
    identTable[pos].value = ident.value;
}

void appendINT(string name) {
    IDENT tmp = {name, INT_T};
    tmp.value = {0};
    tmp.blockNum = getBlockNum();
    identTable.push_back(tmp);
    totalName.push_back(name);
}

void appendConst(string name) {
    IDENT tmp = {name, CONST_T};
    tmp.value = {0};
    tmp.blockNum = getBlockNum();
    identTable.push_back(tmp);
    totalName.push_back(name);
}

void appendARR1(string name) {
    IDENT tmp = {name, ARRAY_T_D1};
    tmp.value = {0};
    tmp.blockNum = getBlockNum();
    identTable.push_back(tmp);
    totalName.push_back(name);
}

void appendARR2(string name) {
    IDENT tmp = {name, ARRAY_T_D2};
    tmp.value = {0};
    tmp.blockNum = getBlockNum();
    identTable.push_back(tmp);
    totalName.push_back(name);
}

void appendConstARR1(string name) {
    IDENT tmp = {name, CONST_ARR_T_D1};
    tmp.value = {0};
    tmp.blockNum = getBlockNum();
    identTable.push_back(tmp);
    totalName.push_back(name);
}

void appendConstARR2(string name) {
    IDENT tmp = {name, CONST_ARR_T_D2};
    tmp.value = {0};
    tmp.blockNum = getBlockNum();
    identTable.push_back(tmp);
    totalName.push_back(name);
}

void appendFUNC_INT(string name) {
    IDENT tmp = {name, FUNC_T_INT};
    tmp.blockNum = getBlockNum();
    identTable.push_back(tmp);
    identTableCnt.push_back(identTable.size()); // next block
    newBlock();
    //printIdentTable();
    totalName.push_back(name);
}

void appendFUNC_VOID(string name) {
    IDENT tmp = {name, FUNC_T_VOID};
    tmp.blockNum = getBlockNum();
    identTable.push_back(tmp);
    identTableCnt.push_back(identTable.size()); // next block
    newBlock();
    //printIdentTable();
    totalName.push_back(name);
}

void appendIdent(IDENT ident) {
    ident.blockNum = getBlockNum();
    identTable.push_back(ident);
    totalName.push_back(ident.name);
}

void updateFunc(string name, int len) {
    int funcPos = getIdentPos(name);
    IDENT func = identTable[funcPos];
    func.paramLen = len;
    identTable[funcPos] = func;
    totalName.push_back(name);
}

void updateArrD1(string name, string len1) {
    int arrPos = getIdentPos(name);
    IDENT arr = identTable[arrPos];
    arr.len1 = (len1);
    identTable[arrPos] = arr;
}

void updateArrD2(string name, string len1, string len2) {
    int arrPos = getIdentPos(name);
    IDENT arr = identTable[arrPos];
    arr.len1 = len1;
    arr.len2 = len2;
    identTable[arrPos] = arr;
}

bool ifReDefine(string name) {
    int st = 0;
    if (!identTableCnt.empty()) st = identTableCnt.back();
    for (int i = st; i < identTable.size(); i++) {
        if (identTable[i].name == name) return true;
    }
    return false;
}

bool ifExist(string name) {
    for (auto &i: identTable) {
        if (i.name == name) return true;
    }
    return false;
}

bool ifConst(string name) {
    //printIdentTable();
    if (!identTableCnt.empty()) {
        for (int i = identTableCnt.back(); i < identTable.size(); i++) {
            if (identTable[i].name == name) {
                return identTable[i].type == CONST_T || identTable[i].type == CONST_ARR_T_D1 ||
                       identTable[i].type == CONST_ARR_T_D1;
            }
        }
    }
    for (int i = identTable.size() - 1; i >= 0; i--) {
        if (identTable[i].name == name) {
            return identTable[i].type == CONST_T || identTable[i].type == CONST_ARR_T_D1 ||
                   identTable[i].type == CONST_ARR_T_D1;
        }
    }
    return false;
}

bool ifFormatLegal(string str) {
    for (int i = 1; i < str.size() - 1; i++) {
        if (str[i] == '\\') {
            if ((i < str.size() - 2) && str[i + 1] == 'n') continue;
        }
        if (str[i] == '%') {
            if ((i < str.size() - 2) && str[i + 1] == 'd') continue;
        }
        if (str[i] != '\\' && (str[i] == 32 || str[i] == 33 || (40 <= str[i] && str[i] <= 126))) continue;
        return false;
    }
    return true;
}

bool ifStrConCntCoordinate(string str, int x) {
    int cnt = 0, idx = 0;
    while ((idx = str.find("%d", idx)) < str.length()) {
        cnt++;
        idx++;
    }
    return cnt == x;
}

bool ifParamTypeCoordinate(vector<int> params) {
    for (int i = identTableCnt.back(); i < identTable.size(); i++) {
        if (identTable[i].type != params[i - identTableCnt.back()]) {
            return false;
        }
    }
    return true;
}

bool ifParamCntCoordinate(vector<int> params) {
    int cnt = identTable.size() - identTableCnt.back();
    return cnt == params.size();
}

void endBlock() {
    //printIdentTable();
    int cnt = identTableCnt.back();
    //("-----------------------------------------------------%d %d\n", cnt, identTableCnt.size());
    while (identTable.size() > cnt) {
        identTable.pop_back();
    }
    if (cnt > 0) {
        identTableCnt.pop_back();
    }

    tmpBlockNums.pop_back();

    //printTotalTable();
    //printIdentTable();
}

void enterBlock() {
    identTableCnt.push_back(identTable.size());
    newBlock();
}

int lastLine = -1;

void throwError(int code, int line) {
    if (line == lastLine) return;
    FILE *f = fopen("error.txt", "a");
    fprintf(f, "%d %c\n", line + 1, 'a' - code - 1);
    fclose(f);
    lastLine = line;
    correctFlag = false;
}

IDENT getIdentTemporarily(std::string name) {
    for (int i = identTable.size() - 1; i >= 0; i--) {
        if (identTable[i].name == name) {
            return identTable[i];
        }
    }
    return {};
}

int getIdentPos(std::string name) {
    for (int i = identTable.size() - 1; i >= 0; i--) {
        if (identTable[i].name == name) {
            return i;
        }
    }
    return -1;
}

string getName(string a) {
    int flag = 0;
    for (int t = 0; t < 1000; t++) {
        string str = a + to_string(t);
        for (int i = 0; i < totalName.size(); i++) {
            if (str == totalName[i]) break;
            if (i == totalName.size() - 1) flag = 1;
        }
        if (flag) {
            totalName.push_back(str);
            return str;
        }
    }
    return a;
}

void printIdentTable() {
    cout << "--------------------start size = " << identTable.size() << "----------------------\n";
    int k = 0;
    int len = identTableCnt.size();
    for (int i = 0; i < identTable.size(); i++) {
        if (k < len && identTableCnt[k] == i) {
            cout << "--------------\n";
            k++;
        }
        cout << identTable[i].name << " " << identTable[i].type << "\n";
    }
    cout << "----------------------end-------------------------\n";
}