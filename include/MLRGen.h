//
// Created by emilyu on 2022/10/26.
//

#ifndef MAIN_CPP_MLRGEN_H
#define MAIN_CPP_MLRGEN_H
#include <vector>

std::string genExpCode(std::string str);

bool isNum(std::string str);

bool isCalSym(std::string s1);

int calculate(std::string s1, std::string s2, std::string s3);

void genFuncDeclCode(std::string type, std::string name);

void genFuncParamCode(std::string type, std::string name);

void genVarCode(std::string str);

void genAssignCode(std::string str, std::string str1, int dim);

void genPrintfCode(std::string str);

void addIfCode();

void addWhileCode();

void addArrCode();

void genString(std::string str);

extern std::vector<std::string> utils;
#endif //MAIN_CPP_MLRGEN_H
