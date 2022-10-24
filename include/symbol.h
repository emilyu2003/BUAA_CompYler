#ifndef _SYMBOL_H
#define _SYMBOL_H

#define IDENFR      1
#define INTCON      2
#define STRCON      3
#define MAINTK      4
#define CONSTTK     5
#define INTTK       6
#define BREAKTK     7
#define CONTINUETK  8
#define IFTK        9
#define ELSETK      10
#define NOT         11
#define AND         12
#define OR          13
#define WHILETK     14
#define GETINTTK    15
#define PRINTFTK    16
#define RETURNTK    17
#define PLUS        18
#define MINU        19
#define VOIDTK      20
#define MULT        21
#define DIV         22
#define MOD         23
#define LSS         24
#define LEQ         25
#define GRE         26
#define GEQ         27
#define EQL         28
#define NEQ         29
#define ASSIGN      30
#define SEMICN      31
#define COMMA       32
#define LPARENT     33
#define RPARENT     34
#define LBRACK      35
#define RBRACK      36
#define LBRACE      37
#define RBRACE      38

#include <string>


struct Lexer {
    int type;
    std::string str;
    int line;   // lineNum
};

void initLexer();

int getSym();

Lexer lexer();

int peek();

int peeeek();

int isLVal();

int getErrorLine();

int getInt();

std::string getStr();

void printSymbol(int type, std::string str);

#endif