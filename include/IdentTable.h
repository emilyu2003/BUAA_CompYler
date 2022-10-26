//
// Created by emilyu on 2022/10/17.
//

#ifndef MAIN_CPP_IDENTTABLE_H
#define MAIN_CPP_IDENTTABLE_H

#define INT_T 1
#define ARRAY_T_D1 2
#define ARRAY_T_D2 3
#define VOID_T 4
#define FUNC_T_INT 5
#define FUNC_T_VOID 6
#define CONST_T 7
#define CONST_ARR_T_D1 8
#define CONST_ARR_T_D2 8

#define ERROR_A -1
#define ERROR_B -2
#define ERROR_C -3
#define ERROR_D -4
#define ERROR_E -5
#define ERROR_F -6
#define ERROR_G -7
#define ERROR_H -8
#define ERROR_I -9
#define ERROR_J -10
#define ERROR_K -11
#define ERROR_L -12
#define ERROR_M -13

#include <string>
#include <vector>
#include <unordered_map>

struct IDENT {
    std::string name;
    int type;   // INT VOID ARR[], ARR[][], INT FUNC, VOID FUNC
    int len[2]; // INT == [1, 0], VOID == [1, 0], ARR[n] == [n, 0], ARR[n][m] = [n, m], FUNC = [0, 0]
    int paramLen;  // len of params
};

void appendINT(std::string name);

void appendConst(std::string name);

void appendFUNC_INT(std::string name);

void appendFUNC_VOID(std::string name);

//void updateFUNC_INT(std::string name, std::vector<IDENT> tmp);

//void updateFUNC_VOID(std::string name, std::vector<IDENT> tmp);

void appendARR1(std::string name);

void appendARR2(std::string name);

void appendConstARR1(std::string name);

void appendConstARR2(std::string name);

void appendIdent(IDENT ident);

bool ifReDefine(std::string name);

bool ifExist(std::string name);

bool ifConst(std::string name);

bool ifFormatLegal(std::string str);

bool ifStrConCntCoordinate(std::string str, int x);

bool ifParamTypeCoordinate(std::vector<int> a);

bool ifParamCntCoordinate(std::vector<int> a);

void endBlock();

void enterBlock();

void throwError(int code, int line);

IDENT getIdent(std::string str);

void printIdentTable();

extern std::vector<IDENT> identTable;
extern std::vector<int> identTableCnt;

#endif //MAIN_CPP_IDENTTABLE_H
