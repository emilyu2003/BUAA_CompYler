//
// Created by emilyu on 2022/9/27.
//
#include "symbol.h"
#include "parse.h"
#include "base.h"
#include "IdentTable.h"
#include <cstdio>
#include <string>
#include <algorithm>
#include <vector>
#include <iostream>

using namespace std;

int now;
bool haveReturn = false;
bool isFunc = false;
int isWhile = 0;
vector<IDENT> RParams;

void printParseResult(const string &s) {
    cout << "<" << s << ">" << endl;
}

/* end 默认包括; */
int RelExp() {
    AddExp();
    int tmp = peek();
    while (tmp == LEQ || tmp == GEQ || tmp == LSS || tmp == GRE) {
        printParseResult("RelExp");
        now = getSym();
        AddExp();
        tmp = peek();
    }

    printParseResult("RelExp");
    return 1;
}

int EqExp() {
    RelExp();
    int tmp = peek();
    while (tmp == EQL || tmp == NEQ) {
        printParseResult("EqExp");
        now = getSym();
        RelExp();
        tmp = peek();
    }

    printParseResult("EqExp");
    return 1;
}

int LAndExp() {
    EqExp();
    int tmp = peek();
    while (tmp == AND) {
        printParseResult("LAndExp");
        now = getSym();
        EqExp();
        tmp = peek();
    }

    printParseResult("LAndExp");
    return 1;
}

int LOrExp() {
    LAndExp();
    int tmp = peek();
    while (tmp == OR) {
        printParseResult("LOrExp");
        now = getSym();
        LAndExp();
        tmp = peek();
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
    string name = getStr();
    if (!ifExist(name)) {
        throwError(ERROR_C);
    }
    int rtNum = 1;
    if (ifConst(getStr())) {
        rtNum = ERROR_H;
    }
    for (int i = 0; i < 2; i++) {
        int tmp = peek();
        if (tmp != LBRACK) break;
        now = getSym();      // now = LBRACK
        tmp = peek();
        if (tmp == ASSIGN) {
            throwError(ERROR_K);
            break;
        }
        if (tmp != RBRACK) {
            Exp();
        }
        now = getSym();      // now = RBRACK
        if (now != RBRACK) break;
    }
    printParseResult("LVal");
    return rtNum;
}

int Number() {
    now = getSym();
    printParseResult("Number");
    return 1;
}

int FuncRParams() {
    int tmp = peek();   // tmp == IDENFR
    int paramCnt = 0;
    while (tmp != RPARENT) {    // TODO dont know how to get type of RPARAMS
        Exp();
        tmp = peek();
        if (tmp == COMMA) {
            now = getSym();  // now == COMMA
        }
        // TODO dont know how to get missing )
        paramCnt++;
        tmp = peek();
    }

    printParseResult("FuncRParams");
    return paramCnt;
}

int PrimaryExp() {
    int tmp = peek();
    if (tmp == LPARENT) {
        now = getSym();
        tmp = peek();
        if (tmp == RPARENT) {
            now = getSym();
            return 1;
        }
        Exp();
        now = getSym();  // now == RPARENT
    } else {
        if (tmp == INTCON) {
            Number();
        } else {
            int rtNum = LVal();
            if (rtNum == ERROR_H) {
                throwError(ERROR_H);
            }
        }
    }

    printParseResult("PrimaryExp");
    return 1;
}

int UnaryOp() {
    now = getSym();

    printParseResult("UnaryOp");
    return 1;
}

int UnaryExp() {
    int tmp = peek();
    if (tmp == PLUS || tmp == MINU || tmp == NOT) {
        //now = getSym();
        UnaryOp();
        UnaryExp();
    } else {
        if (tmp == IDENFR) {
            now = getSym(); // IDENFR
            string name = getStr();
            int line = getErrorLine();
            if (!ifExist(name)) {
                throwError(ERROR_C);
            }
            tmp = peek();
            if (tmp == LPARENT) {
                now = getSym(); // LPARENT
                //now = getSym();
                tmp = peek();
                if (tmp != RPARENT) {   // TODO
                    int paramCnt = FuncRParams();
                }
                tmp = peek();
                if (tmp == RPARENT) {
                    now = getSym(); // RPARENT
                }
            } else {
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
        printParseResult("MulExp");
        now = getSym();  // now == PLUS
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
        printParseResult("AddExp");
        now = getSym();  // now == PLUS || MINU
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
        now = getSym();
        tmp = peek();
        if (tmp != RBRACE) {
            ConstInitVal();
            tmp = peek();
            while (tmp == COMMA) {
                now = getSym();
                ConstInitVal();
                tmp = peek();
            }
        }
    }
    if (tmp != LBRACE && tmp != RBRACE) {
        ConstExp();
    }

    if (tmp == RBRACE) {
        now = getSym();
    }

    printParseResult("ConstInitVal");
    return 1;
}

int ConstDef() {
    now = getSym(); //now == INTTK || COMMA
    now = getSym();
    string name = getStr();
    int tmp = peek();
    int dimension = 0;
    if (tmp == LBRACK) {
        now = getSym();
        ConstExp();
        tmp = peek();
        dimension++;
        if (tmp != RBRACK) {
            throwError(ERROR_K);
        } else {
            now = getSym(); // now == RBRACK
        }
        tmp = peek();
        if (tmp == LBRACK) {
            now = getSym();
            ConstExp();
            tmp = peek();
            dimension++;
            if (tmp != RBRACK) {
                throwError(ERROR_K);
            } else {
                now = getSym(); // now == RBRACK
            }
        }
    }

    if (ifReDefine(name)) {
        throwError(ERROR_B);
    } else {
        if (dimension == 0) {
            appendINT(name);
        } else if (dimension == 1) {
            appendARR1(name);
        } else if (dimension == 2) {
            appendARR2(name);
        }
    }

    tmp = peek();
    if (tmp == ASSIGN) {
        now = getSym();
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

    tmp = peek();
    if (tmp != SEMICN) {
        throwError(ERROR_I);
    } else {
        now = getSym(); // now == SEMICN
    }

    printParseResult("ConstDecl");
    return 1;
}

int InitVal() {
    int tmp = peek();
    if (tmp == LBRACE) {
        now = getSym();
        InitVal();
        tmp = peek();
        while (tmp == COMMA) {
            now = getSym();
            InitVal();
            tmp = peek();
        }
    }
    if (tmp != LBRACE && tmp != RBRACE) {
        Exp();
    }

//    tmp = peek();
//    if (tmp != SEMICN) {
//        now = getSym();  // now == RBRACE || COMMA
//    }
    if (tmp == RBRACE) {
        now = getSym();
    }
    printParseResult("InitVal");
    return 1;
}

int VarDef() {
    // now == INTTK
    now = getSym();  // now = IDENFR
    string name = getStr();
    int tmp = peek();
    int dimension = 0;
    if (tmp == LBRACK) {
        now = getSym();
        ConstExp();
        tmp = peek();
        if (tmp != RBRACK) {
            throwError(ERROR_K);
        } else {
            now = getSym(); // now == RBRACK
        }
        tmp = peek();
        dimension++;
        if (tmp == LBRACK) {
            ConstExp();
            tmp = peek();
            if (tmp != RBRACK) {
                throwError(ERROR_K);
            } else {
                now = getSym(); // now == RBRACK
            }
            dimension++;
        }
    }

    if (ifReDefine(name)) {
        throwError(ERROR_B);
    } else {
        if (dimension == 0) {
            appendINT(name);
        } else if (dimension == 1) {
            appendARR1(name);
        } else if (dimension == 2) {
            appendARR2(name);
        }
    }

    tmp = peek();
    if (tmp == ASSIGN) {
        now = getSym();
        InitVal();
    }

    printParseResult("VarDef");
    return 1;
}

int VarDecl() {
    // now == INTTK
    VarDef();
    now = getSym();
    while (now == COMMA) {
        VarDef();
        now = getSym();
    }

    if (now != SEMICN) {
        throwError(ERROR_I);
    } else {
        // now == SEMICN
    }

    printParseResult("VarDecl");
    return 1;
}

IDENT FuncFParam() {
    // now == INTTK
    now = getSym();  // now == IDENFR
    string name = getStr();
    if (ifReDefine(name)) {
        throwError(ERROR_B);
    }
    IDENT tmpIdent;
    int dimension;

    int tmp = peek();
    if (tmp == LBRACK) {
        now = getSym();  // now == LBRACK
        tmp = peek();
        if (tmp != RBRACK) {
            throwError(ERROR_K);
        } else {
            now = getSym(); // now == RBRACK
        }
        dimension++;
        tmp = peek();
        if (tmp == LBRACK) {
            now = getSym();  // now == LBRACK
            ConstExp();
            tmp = peek();
            if (tmp != RBRACK) {
                throwError(ERROR_K);
            } else {
                now = getSym(); // now == RBRACK
            }
            dimension++;
        }
    }
    if (dimension == 0) {
        tmpIdent = {name, INT_T, {0, 0}};
    } else if (dimension == 2) {
        tmpIdent = {name, ARRAY_T_D1, {0, 0}};
    } else {
        tmpIdent = {name, ARRAY_T_D2, {0, 0}};
    }

    printParseResult("FuncFParam");
    return tmpIdent;
}

int FuncFParams(int type, string name) {
    int paramCnt = 0;
    int tmp = peek();   // tmp == INTTK || RPARENT
    vector<IDENT> tmpParams;
    while (tmp != RPARENT && tmp != LBRACE) {
        now = getSym(); // tmp == LPARENT
        IDENT tmpIdent = FuncFParam();
        tmpParams.push_back(tmpIdent);
        tmp = peek();
        if (tmp == LBRACE) {
            throwError(ERROR_J);
            break;
        }
        if (tmp != RPARENT) {
            now = getSym();
        }
        paramCnt++;
    }
    if (type == INT_T) {
        appendFUNC_INT(name, tmpParams);
    } else {
        appendFUNC_VOID(name, tmpParams);
    }
    printParseResult("FuncFParams");
    return 1;
}

int FuncDef(int type, string name) {
    if (ifReDefine(name)) {
        throwError(ERROR_B);
    }
    // now == LPARENT
    int tmp = peek();
    if (tmp != RPARENT) {
        FuncFParams(type, name);
    } else {
        vector<IDENT> tmpParams;
        if (type == INT_T) {
            appendFUNC_INT(name, tmpParams);
        } else {
            appendFUNC_VOID(name, tmpParams);
        }
    }
    now = getSym();  // now == RPARENT
    isFunc = true;
    Block();
    printParseResult("FuncDef");
    return 1;
}

int Stmt() {
    int tmp = peek();
    if (tmp == BREAKTK || tmp == CONTINUETK) {
        if (isWhile == 0) {
            throwError(ERROR_M);
        }
        now = getSym();
        tmp = peek();
        if (tmp != SEMICN) {
            throwError(ERROR_I);
        } else {
            now = getSym();  // now == SEMICN
        }
    } else if (tmp == RETURNTK) {
        now = getSym();
        //now = getSym();
        tmp = peek();
        if (tmp == RBRACE) {
            throwError(ERROR_I);
        } else {
            if (tmp != SEMICN) {
                haveReturn = true;
                Exp();
            }
            tmp = peek();
            if (tmp == SEMICN) {
                now = getSym();
            } else {
                throwError(ERROR_I);
            }
        }
    } else if (tmp == PRINTFTK) {
        now = getSym(); // now == PRINTFTK
        now = getSym(); // now == LPARENT
        now = getSym(); // now == STRCON

        string strCon = getStr();
        if (!ifFormatLegal(strCon)) {
            throwError(ERROR_A);
        }

        int expCnt = 0;

        tmp = peek();
        if (tmp != RPARENT) {
            now = getSym();
            while (now == COMMA) {
                Exp();
                now = getSym();
                expCnt++;
            }
        } else {
            now = getSym();
        }

        if (!ifStrConCntCoordinate(strCon, expCnt)) {
            throwError(ERROR_L);
        }

        tmp = peek();
        if (tmp != SEMICN) {
            throwError(ERROR_I);
        } else {
            now = getSym(); // now == SEMICN
        }
    } else if (tmp == IFTK) {
        now = getSym();  // now == IFTK
        now = getSym(); // now == LPARENT
        //now = getSym();
        Cond();
        now = getSym(); // now == RPARENT
        Stmt();
        tmp = peek();
        if (tmp == ELSETK) {
            now = getSym(); // now == ELSETK
            Stmt();
        }
    } else if (tmp == WHILETK) {
        now = getSym();  // now == WHILETK
        now = getSym(); // now == LPARENT
        //now = getSym();
        Cond();
        // now == RPARENT
        now = getSym();
        isWhile++;
        Stmt();
        isWhile--;
    } else if (tmp == LBRACE) {
        Block();
    } else if (tmp == IDENFR) {
        if (!isLVal()) {
            Exp();
        } else {
            now = getSym();  //IDENFR
            int rtNum = LVal();
            if (rtNum == ERROR_H) {
                throwError(ERROR_H);
            }
            now = getSym(); // now == ASSIGN
            tmp = peek();
            if (tmp == GETINTTK) {
                now = getSym(); // now == GETINTTK
                now = getSym(); // now == LPARENT
                now = getSym(); // now == RPARENT
                //now = getSym(); // now == SEMICN
            } else {
                Exp();
            }
        }
        tmp = peek();
        if (tmp == SEMICN) {
            now = getSym();
        } else {
            throwError(ERROR_I);
        }
    } else {
        tmp = peek();
        if (tmp != SEMICN) {
            Exp();
        }
        tmp = peek();
        if (tmp == SEMICN) {
            now = getSym();
        } else {
            throwError(ERROR_I);
        }
    }

    printParseResult("Stmt");
    return 1;
}

int Block() {
    if (!isFunc) {
        enterBlock();
    } else {
        isFunc = false;
    }
    now = getSym();
    // now == LBRACE
    int tmp = peek();
    while (tmp != RBRACE) {
        if (tmp == CONSTTK) {
            now = getSym();
            ConstDecl();
        } else if (tmp == INTTK) {
            now = getSym();
            VarDecl();
        } else {
            Stmt();
        }
        tmp = peek();
    }
    now = getSym();  // now == RBRACE

    endBlock();  // remove idents in this block
    printParseResult("Block");
    return 1;
}

int MainFuncDef() {
    // now == INTTK
    now = getSym(); // now == MAINTK
    now = getSym(); // now == LPARENT
    now = getSym(); // now == RPARENT
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
    while (lexerPos < lexerLen) {
        now = getSym();
        if (now == CONSTTK) {
            ConstDecl();
        } else if (now == INTTK) {
            int tmp = peek();
            if (tmp == MAINTK) {
                MainFuncDef();
                if (!haveReturn) {
                    throwError(ERROR_G);
                } else {
                    haveReturn = false;
                }
            } else if (tmp == IDENFR) {
                //now = getSym(); // now == IDENFR
                tmp = peeeek();
                if (tmp == LPARENT) {
                    FuncType();
                    now = getSym(); // now == IDENFR
                    string name = getStr();
                    now = getSym(); // now == (
                    FuncDef(INT_T, name);
                    if (!haveReturn) {
                        throwError(ERROR_G);
                    } else {
                        haveReturn = false;
                    }
                } else {
                    // now == INTTK
                    VarDecl();
                }
            }
        } else if (now == VOIDTK) {
            FuncType();
            now = getSym();  // now == IDENFR
            string name = getStr();
            now = getSym();  // now == (
            FuncDef(VOID_T, name);
            if (haveReturn) {
                throwError(ERROR_F);
                haveReturn = false;
            }
        }
    }
    printParseResult("CompUnit");
    return 1;
}