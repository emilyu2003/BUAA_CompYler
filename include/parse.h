#ifndef _PARSE_H
#define _PARSE_H
#include <string>

int parse();
void printParseResult(const std::string& s);
int RelExp(int end);
int EqExp(int end);
int LAndExp(int end);
int LOrExp(int end);
int Exp(int end);
int Cond(int end);
int LVal(int end);
int Number();
int FuncRParams();
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
int FuncFParam();
int FuncFParams();
int FuncDef();
int Stmt();
int Block();
int MainFuncDef();

#endif
