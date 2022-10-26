#ifndef _PARSE_H
#define _PARSE_H
#include <string>
#include "IdentTable.h"

int parse();
void printParseResult(const std::string& s);
int RelExp();
int EqExp();
int LAndExp();
int LOrExp();
int Exp();
int Cond();
int LVal();
int Number();
std::vector<int> FuncRParams();
int FuncType();
int PrimaryExp();
int UnaryOp();
int UnaryExp();
int MulExp();
int AddExp();
int ConstExp();
int ConstInitVal();
int ConstDef();
int ConstDecl();
int InitVal();
int VarDef();
int VarDecl();
IDENT FuncFParam();
int FuncFParams(std::string str);
int FuncDef(int type, std::string str);
int Stmt();
int Block();
int MainFuncDef();

#endif
