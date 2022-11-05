//
// Created by emilyu on 2022/10/28.
//

#ifndef MAIN_CPP_TLRGEN_H
#define MAIN_CPP_TLRGEN_H

#include <string>

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

extern std::vector<std::string> middleCode;
extern std::vector<T_NAME> ssaIdents;

#endif //MAIN_CPP_TLRGEN_H
