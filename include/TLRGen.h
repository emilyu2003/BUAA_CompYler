//
// Created by emilyu on 2022/10/28.
//

#ifndef MAIN_CPP_TLRGEN_H
#define MAIN_CPP_TLRGEN_H

#include <string>

#define GP_BASE 0x10008000
#define FP_BASE 0x10040000

void initGenerator();

void printMiddleCode();

void mipsGen();

struct T_NAME {
    std::string name;
    int cnt;
    std::string addr;

    bool operator<(const T_NAME &t) const {
        return cnt > t.cnt;
    }
};

int getSsaIdentPos(const std::string& name);

void newVar(std::string s, int isGlobal);
void newParam(std::string s);

extern std::vector<std::string> middleCode;
extern std::vector<T_NAME> ssaIdents;
extern std::vector<std::string> generatedCode;

#endif //MAIN_CPP_TLRGEN_H
