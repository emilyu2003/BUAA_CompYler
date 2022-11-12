//
// Created by emilyu on 2022/10/26.
//

#ifndef MAIN_CPP_MLRGEN_H
#define MAIN_CPP_MLRGEN_H

#include <vector>

void printCode(std::string toFile, std::string format, std::string str);

std::string genExpCode(std::string str);

std::string getLvalCode(std::string str);

bool isNum(std::string str);

bool isCalSym(std::string s1);

int calculate(std::string s1, std::string s2, std::string s3);

void genFuncDeclCode(std::string type, std::string name);

void genFuncParamCode(std::string type, std::string name);

void genVarCode(std::string str);

void genConstCode(std::string str);

void genAssignCode(std::string str, std::string str1, int dim);

void genPrintfCode(std::string str, std::vector<std::string> vars);

std::string genScanfCode();

std::string genCallFuncCode(std::string name, std::vector<std::string> utils);

void genAndCode(std::string str, std::string endStr);

void genCondCode(std::string str, std::string beginStr, std::string elseStr, std::string endIfStr);

void genArrCode();

void genReturnCode(std::string str);

void genString(std::string str);

std::string getFromStrCons(std::string str);

extern int curBlockNum;
extern std::string tAssign;

#endif //MAIN_CPP_MLRGEN_H
