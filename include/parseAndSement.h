//
// Created by emilyu on 2022/10/26.
//

#ifndef MAIN_CPP_PARSEANDSEMENT_H
#define MAIN_CPP_PARSEANDSEMENT_H
#include <string>
#include "IdentTable.h"

std::string parse();
std::string printParseResult(const std::string& s);
std::string RelExp();
std::string EqExp();
std::string LAndExp();
std::string LOrExp();
std::string Exp();
std::string Cond();
std::string LVal();
std::string Number();
std::string FuncRParams();
std::string FuncType();
std::string PrimaryExp();
std::string UnaryOp();
std::string UnaryExp();
std::string MulExp();
std::string AddExp();
std::string ConstExp();
std::string ConstInitVal();
std::string ConstDef();
std::string ConstDecl();
std::string InitVal();
std::string VarDef();
std::string VarDecl();
std::string FuncFParam();
std::string FuncFParams(std::string str);
std::string FuncDef(int type, std::string str);
std::string Stmt();
std::string Block();
std::string MainFuncDef();

#endif //MAIN_CPP_PARSEANDSEMENT_H
