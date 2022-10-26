//
// Created by emilyu on 2022/10/17.
//

#include "IdentTable.h"
#include "symbol.h"
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <iostream>

using namespace std;

vector<IDENT> identTable;
vector<int> identTableCnt;

void appendINT(string name) {
    IDENT tmp = {name, INT_T, {1, 0}};
    identTable.push_back(tmp);
}

void appendVOID(string name) {
    IDENT tmp = {name, VOID_T, {1, 0}};
    identTable.push_back(tmp);
}

void appendConst(string name) {
    IDENT tmp = {name, CONST_T, {1, 0}};
    identTable.push_back(tmp);
}

void appendARR1(string name) {
    IDENT tmp = {name, ARRAY_T_D1, {0, 0}};
    identTable.push_back(tmp);
}

void appendARR2(string name) {
    IDENT tmp = {name, ARRAY_T_D2, {0, 0}};
    identTable.push_back(tmp);
}

void appendConstARR1(string name) {
    IDENT tmp = {name, CONST_ARR_T_D1, {0, 0}};
    identTable.push_back(tmp);
}

void appendConstARR2(string name) {
    IDENT tmp = {name, CONST_ARR_T_D2, {0, 0}};
    identTable.push_back(tmp);
}

void appendFUNC_INT(string name) {
    IDENT tmp = {name, FUNC_T_INT, {0, 0}};
    identTable.push_back(tmp);
    identTableCnt.push_back(identTable.size()); // next block
    //printIdentTable();
}

//void updateFUNC_INT(string name, vector<IDENT> params) {
//    for (int i = 0; i < identTable.size(); i++) {
//        if (identTable[i].name == name && identTable[i].type == FUNC_T_INT) {
//            identTable[i].paramLen = params.size();
//        }
//    }
//    for (int i = 0; i < params.size(); i++) {
//        identTable.push_back(params[i]);
//    }
//}

void appendFUNC_VOID(string name) {
    IDENT tmp = {name, FUNC_T_VOID, {0, 0}};
    identTable.push_back(tmp);
    identTableCnt.push_back(identTable.size()); // next block
    //printIdentTable();
}

//void updateFUNC_VOID(string name, vector<IDENT> params) {
//    for (int i = 0; i < identTable.size(); i++) {
//        if (identTable[i].name == name && identTable[i].type == FUNC_T_VOID) {
//            identTable[i].paramLen = params.size();
//        }
//    }
//    for (int i = 0; i < params.size(); i++) {
//        identTable.push_back(params[i]);
//    }
//}

void appendIdent(IDENT ident) {
    identTable.push_back(ident);
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
    //printIdentTable();
}

void enterBlock() {
    identTableCnt.push_back(identTable.size());
}

int lastLine = -1;

void throwError(int code, int line) {
    if (line == lastLine) return;
    FILE *f = fopen("error.txt", "a");
    fprintf(f, "%d %c\n", line + 1, 'a' - code - 1);
    fclose(f);
    lastLine = line;
}

IDENT getIdent(std::string name) {
    for (int i = identTable.size() - 1; i >= 0; i--) {
        if (identTable[i].name == name) {
            return identTable[i];
        }
    }
    return {};
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