//
// Created by emilyu on 2022/10/26.
//

#ifndef MAIN_CPP_PARSEANDSEMANT_H
#define MAIN_CPP_PARSEANDSEMANT_H
#include <string>

std::string parseAndSemant();
void printParseResult_S(std::string s);
std::string RelExp_S();
std::string EqExp_S();
std::string LAndExp_S();
std::string LOrExp_S();
std::string Exp_S();
std::string Cond_S();
std::string LVal_S();
std::string Number_S();
std::string FuncRParams_S();
std::string FuncType_S();
std::string PrimaryExp_S();
std::string UnaryOp_S();
std::string UnaryExp_S();
std::string MulExp_S();
std::string AddExp_S();
std::string ConstExp_S();
std::string ConstInitVal_S();
std::string ConstDef_S();
std::string ConstDecl_S();
std::string InitVal_S();
std::string VarDef_S();
std::string VarDecl_S();
std::string FuncFParam_S();
std::string FuncFParams_S(std::string name);
std::string FuncDef_S(int type, std::string name);
std::string Stmt_S();
std::string Block_S();
std::string MainFuncDef_S();

#endif //MAIN_CPP_PARSEANDSEMANT_H
