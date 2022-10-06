//
// Created by emilyu on 2022/9/27.
//
#include "symbol.h"
#include "parse.h"
#include "base.h"
#include <cstdio>
#include <string>
#include <algorithm>
#include <vector>
#include <iostream>

using namespace std;

int now;

void printParseResult(const string &s) {
    cout << "<" << s << ">" << endl;
}

/* end 默认包括; */
int RelExp() {
    AddExp();
    int tmp = peek();
    while (tmp == LEQ || tmp == GEQ || tmp == LSS || tmp == GRE) {
        now = lexer();
        tmp = peek();
        AddExp();
    }

    printParseResult("RelExp");
    return 1;
}

int EqExp() {
    RelExp();
    int tmp = peek();
    while (tmp == EQL || tmp == NEQ) {
        now = lexer();
        tmp = peek();
        RelExp();
    }

    printParseResult("EqExp");
    return 1;
}

int LAndExp() {
    EqExp();
    int tmp = peek();
    while (tmp == AND) {
        now = lexer();
        tmp = peek();
        EqExp();
    }

    printParseResult("LAndExp");
    return 1;
}

int LOrExp() {
    LAndExp();
    int tmp = peek();
    while (tmp == OR) {
        now = lexer();
        tmp = peek();
        LAndExp();
    }

    printParseResult("LOrExp");
    return 1;
}

int Exp() {
    AddExp();

    printParseResult("Exp");
    return 1;
}

int Cond() {
    LOrExp();

    printParseResult("Cond");
    return 1;
}

int LVal() {
    // now = IDENFR
    for (int i = 0; i < 2; i++) {
        int tmp = peek();
        if (tmp != LBRACK) break;
        now = lexer();      // now = LBRACK
        Exp();
        now = lexer();      // now = RBRACK
        if (now != RBRACK) break;
    }

    printParseResult("LVal");
    return 1;
}

int Number() {
    now = lexer();
    printParseResult("Number");
    return 1;
}

int FuncRParams() {
    int tmp = peek();
    while (tmp != RPARENT) {
        Exp();
        tmp = peek();
        if (tmp != RPARENT) {
            now = lexer();  // now == COMMA
        }
    }

    printParseResult("FuncRParams");
    return 1;
}

int PrimaryExp() {
    int tmp = peek();
    if (tmp == LPARENT) {
        now = lexer();
        Exp();
        now = lexer();  // now == RPARENT
    } else {
        if (tmp == INTCON) {
            Number();
        } else {
            LVal();
        }
    }

    printParseResult("PrimaryExp");
    return 1;
}

int UnaryOp() {
    now = lexer();

    printParseResult("UnaryOp");
    return 1;
}

int UnaryExp() {
    int tmp = peek();
    if (tmp == PLUS || tmp == MINU || tmp == NOT) {
        //now = lexer();
        UnaryOp();
        UnaryExp();
    } else {
        if (tmp == IDENFR) {
            int ttmp = peeeek();
            if (ttmp == LPARENT) {
                now = lexer();
                now = lexer(); // LPARENT
                //now = lexer();
                FuncRParams();
                now = lexer(); // RPARENT
            } else {
                now = lexer();
                PrimaryExp();
            }
        } else {
            PrimaryExp();
        }
    }

    printParseResult("UnaryExp");
    return 1;
}

int MulExp() {
    UnaryExp();
    int tmp = peek();
    while (tmp == MULT || tmp == DIV || tmp == MOD) {
        now = lexer();  // now == PLUS
        UnaryExp();
        tmp = peek();
    }

    printParseResult("MulExp");
    return 1;
}

int AddExp() {
    MulExp();
    int tmp = peek();
    while (tmp == PLUS || tmp == MINU) {
        now = lexer();  // now == PLUS || MINU
        MulExp();
        tmp = peek();
    }

    printParseResult("AddExp");
    return 1;
}

int ConstExp() {
    AddExp();

    printParseResult("ConstExp");
    return 1;
}

int ConstInitVal() {
    // now == ASSIGN
    int tmp = peek();
    if (tmp == LBRACE) {
        now = lexer();
        ConstInitVal();
        now = lexer(); // now = RBRACE
    } else {
        ConstExp();
    }

    printParseResult("ConstInitVal");
    return 1;
}

int ConstDef() {
    now = lexer(); //now == INTTK || COMMA
    now = lexer();
    int tmp = peek();
    if (tmp == LBRACK) {
        now = lexer();
        ConstExp();
        now = lexer();
        tmp = peek();
        if (tmp == LBRACK) {
            now = lexer();
            ConstExp();
            now = lexer();
        }
    }
    if (tmp == ASSIGN) {
        now = lexer();
        ConstInitVal();
    }

    printParseResult("ConstDef");
    return 1;
}

int ConstDecl() {
    // now == INTTK

    ConstDef();
    int tmp = peek();
    while (tmp == COMMA) {
        ConstDef();
        tmp = peek();
    }

    now = lexer(); // now == SEMICN

    printParseResult("ConstDecl");
    return 1;
}

int InitVal() {
    int tmp = peek();
    if (tmp == LBRACE) {
        now = lexer();
        InitVal();
        tmp = peek();
        while (tmp == COMMA) {
            now = lexer();
            InitVal();
            tmp = peek();
        }
    }
    if (tmp != LBRACE && tmp != RBRACE) {
        Exp();
    }

//    tmp = peek();
//    if (tmp != SEMICN) {
//        now = lexer();  // now == RBRACE || COMMA
//    }
    if (tmp == RBRACE) {
        now = lexer();
    }
    printParseResult("InitVal");
    return 1;
}

int VarDef() {
    // now == INTTK
    now = lexer();  // now = IDENFR
    int tmp = peek();
    if (tmp == LBRACK) {
        now = lexer();
        ConstExp();
        now = lexer();
        tmp = peek();
        if (tmp == LBRACK) {
            now = lexer();
            ConstExp();
            now = lexer();
        }
    }
    tmp = peek();
    if (tmp == ASSIGN) {
        now = lexer();
        InitVal();
    }

    printParseResult("VarDef");
    return 1;
}

int VarDecl() {
    // now == INTTK
    VarDef();
    now = lexer();
    while (now == COMMA) {
        VarDef();
        now = lexer();
    }

    if (now != SEMICN) {
        now = lexer();// now == SEMICN
    }

    printParseResult("VarDecl");
    return 1;
}

int FuncFParam() {
    // now == INTTK
    now = lexer();  // now == IDENFR
    int tmp = peek();
    if (tmp == LBRACK) {
        now = lexer();  // now == LBRACK
        now = lexer();  // now == RBRACK
        tmp = peek();
        if (tmp == LBRACK) {
            now = lexer();  // now == LBRACK
            ConstExp();
            now = lexer();  // now == RBRACK
        }
    }

    printParseResult("FuncFParam");
    return 1;
}

int FuncFParams() {
    int tmp = peek();   // tmp == INTTK || RPARENT
    while (tmp != RPARENT) {
        now = lexer();
        FuncFParam();
        tmp = peek();
        if (tmp != RPARENT) {
            now = lexer();
        }
    }
    printParseResult("FuncFParams");
    return 1;
}

int FuncDef() {
    // now == LPARENT
    FuncFParams();
    now = lexer();  // now == RPARENT
    Block();
    printParseResult("FuncDef");
    return 1;
}

int Stmt() {
    int tmp = peek();
    if (tmp == BREAKTK || tmp == CONTINUETK) {
        now = lexer();
        now = lexer();  // now == SEMICN
    } else if (tmp == RETURNTK) {
        now = lexer();
        //now = lexer();
        if (now != SEMICN) {
            Exp();
        }
    } else if (tmp == PRINTFTK) {
        now = lexer(); // now == PRINTFTK
        now = lexer(); // now == LPARENT
        now = lexer(); // now == STRCON
        tmp = peek();
        if (tmp != RPARENT) {
            now = lexer();
            while (now == COMMA) {
                Exp();
                now = lexer();
            }
        } else {
            now = lexer();
        }
        now = lexer(); // now == SEMICN
    } else if (tmp == IFTK) {
        now = lexer();  // now == IFTK
        now = lexer(); // now == LPARENT
        //now = lexer();
        Cond();
        // now == RPARENT
        now = lexer();
        Stmt();
        tmp = peek();
        if (tmp == ELSETK) {
            now = lexer(); // now == ELSETK
            Stmt();
        }
    } else if (tmp == WHILETK) {
        now = lexer();  // now == WHILETK
        now = lexer(); // now == LPARENT
        //now = lexer();
        Cond();
        // now == RPARENT
        now = lexer();
        Stmt();
    } else if (tmp == LBRACE) {
        Block();
    } else if (tmp == IDENFR) {
        int ttmp = peeeek();
        if (ttmp == LPARENT) {
            Exp();
        } else {
            now = lexer();
            LVal();
            now = lexer(); // now == ASSIGN
            tmp = peek();
            if (tmp == GETINTTK) {
                now = lexer(); // now == GETINTTK
                now = lexer(); // now == LPARENT
                now = lexer(); // now == RPARENT
                now = lexer(); // now == SEMICN
            } else {
                Exp();
            }
        }
    } else {
        Exp();
    }

    tmp = peek();
    if (tmp == SEMICN) {
        now = lexer();
    }

    printParseResult("Stmt");
    return 1;
}

int Block() {
    now = lexer();
    // now == LBRACE
    int tmp = peek();
    while (tmp != RBRACE) {
        if (tmp == CONSTTK) {
            now = lexer();
            ConstDecl();
        } else if (tmp == INTTK) {
            now = lexer();
            VarDecl();
        } else {
            Stmt();
        }
        tmp = peek();
    }
    now = lexer();  // now == RBRACE

    printParseResult("Block");
    return 1;
}

int MainFuncDef() {
    // now == INTTK
    now = lexer(); // now == MAINTK
    now = lexer(); // now == LPARENT
    now = lexer(); // now == RPARENT
    Block();
    // now == RBRACE
    printParseResult("MainFuncDef");
    return 1;
}

int FuncType() {
    printParseResult("FuncType");
    return 1;
}

int parse() {
    while (readPos < inputLen) {
        now = lexer();
        if (now == CONSTTK) {
            ConstDecl();
        } else if (now == INTTK) {
            int tmp = peek();
            if (tmp == MAINTK) {
                MainFuncDef();
            } else if (tmp == IDENFR) {
                //now = lexer(); // now == IDENFR
                tmp = peeeek();
                if (tmp == LPARENT) {
                    FuncType();
                    now = lexer(); // now == IDENFR
                    now = lexer(); // now == (
                    FuncDef();
                } else {
                    // now == INTTK
                    VarDecl();
                }
            }
        } else if (now == VOIDTK) {
            FuncType();
            now = lexer();  // now == IDENFR
            now = lexer();  // now == (
            FuncDef();
        }
    }
    printParseResult("CompUnit");
    return 1;
}