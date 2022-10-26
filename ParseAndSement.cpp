//
// Created by emilyu on 2022/10/26.
//
//
// Created by emilyu on 2022/9/27.
//
#include "symbol.h"
#include "parseAndSement.h"
#include "base.h"
#include <cstdio>
#include <string>
#include <algorithm>
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;
vector<int> FuncRParamsType;

int now;
int isFunc = 0;
vector<IDENT> RParams;


/* end 默认包括; */
string RelExp() {
    string tmpStr;
    tmpStr += AddExp();
    int tmp = peek();
    while (tmp == LEQ || tmp == GEQ || tmp == LSS || tmp == GRE) {
        printParseResult("RelExp");
        now = getSym();
        tmpStr += " " + getStr() + " ";
        tmpStr += AddExp();
        tmp = peek();
    }

    printParseResult("RelExp");
    return tmpStr;
}

string EqExp() {
    string tmpStr;
    tmpStr += RelExp();
    int tmp = peek();
    while (tmp == EQL || tmp == NEQ) {
        printParseResult("EqExp");
        now = getSym();
        tmpStr += " " + getStr() + " ";
        tmpStr += RelExp();
        tmp = peek();
    }
    printParseResult("EqExp");
    return tmpStr;
}

string LAndExp() {
    string tmpStr;
    tmpStr += EqExp();
    int tmp = peek();
    while (tmp == AND) {
        printParseResult("LAndExp");
        now = getSym();
        tmpStr += " " + getStr() + " ";
        tmpStr += EqExp();
        tmp = peek();
    }

    printParseResult("LAndExp");
    return tmpStr;
}

string LOrExp() {
    string tmpStr;
    tmpStr += LAndExp();
    int tmp = peek();
    while (tmp == OR) {
        printParseResult("LOrExp");
        now = getSym();
        tmpStr += " " + getStr() + " ";
        tmpStr += LAndExp();
        tmp = peek();
    }

    printParseResult("LOrExp");
    return tmpStr;
}

string Exp() {
    string tmpStr;
    tmpStr += AddExp();

    printParseResult("Exp");
    return tmpStr;
}

string Cond() {
    string tmpStr;
    tmpStr += LOrExp();

    printParseResult("Cond");
    return tmpStr;
}

string LVal() {
    // now = IDENFR
    string tmpStr = getStr();
    IDENT tmpLVal = getIdent(tmpStr);
    int tmp = peek();
    if (tmp == LBRACK) {
        now = getSym();
        tmpStr += " " + getStr() + " ";
        tmpStr += Exp();
        now = getSym(); // now == RBRACK
        tmpStr += " " + getStr() + " ";
        tmp = peek();
        if (tmp == LBRACK) {
            now = getSym();
            tmpStr += " " + getStr() + " ";
            tmpStr += Exp();
            now = getSym(); // now == RBRACK
            tmpStr += " " + getStr() + " ";
        }
    }

    printParseResult("LVal");
    return tmpStr;
}

string Number() {
    string tmpStr;
    now = getSym();
    tmpStr += getStr();
    printParseResult("Number");
    return tmpStr;
}


string FuncRParams() {
    string tmpStr;
    int tmp = peek();   // tmp == IDENFR
    while (tmp != RPARENT) {    // TODO dont know how to get type of RPARAMS
        tmpStr += Exp();
        now = getSym();  // now == COMMA
        tmpStr += " " + getStr() + " ";
        tmp = peek();
    }
    printParseResult("FuncRParams");
    return tmpStr;
}

string PrimaryExp() {
    string tmpStr;
    int tmp = peek();
    if (tmp == LPARENT) {
        now = getSym();
        tmp = peek();
        if (tmp == RPARENT) {
            now = getSym();
            return 1;
        }
        tmpStr += Exp();
        now = getSym();  // now == RPARENT
    } else {
        if (tmp == INTCON) {
            rtNum = Number();
        } else {
            rtNum = LVal();
            if (rtNum == ERROR_H) {
                throwError(ERROR_H, getErrorLine());
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
    int rtNum = INT_T;
    int tmp = peek();
    if (tmp == PLUS || tmp == MINU || tmp == NOT) {
        //now = getSym();
        UnaryOp();
        rtNum = UnaryExp();
    } else {
        if (tmp == IDENFR) {
            now = getSym(); // IDENFR
            string name = getStr();
            int tmpLine = getErrorLine();
            if (!ifExist(name)) {
                throwError(ERROR_C, getErrorLine());
            }
            tmp = peek();
            if (tmp == LPARENT) {
                now = getSym(); // LPARENT
                //now = getSym();
                tmp = peek();
                vector<int> tmpParams;
                if (tmp == IDENFR || tmp == LPARENT || tmp == PLUS || tmp == MINU || tmp == INTCON) {
                    tmpParams = FuncRParams();
                }
                tmp = peek();
                if (tmp == RPARENT) {
                    now = getSym(); // RPARENT
                } else {
                    throwError(ERROR_J, getErrorLine());
                }
                if (!ifParamCntCoordinate(tmpParams)) {
                    throwError(ERROR_D, tmpLine);
                } else if (!ifParamTypeCoordinate(tmpParams)) {
                    throwError(ERROR_E, tmpLine);
                }
            } else {
                rtNum = PrimaryExp();
            }
        } else {
            rtNum = PrimaryExp();
        }
    }

    printParseResult("UnaryExp");
    return rtNum;
}

int MulExp() {
    int rtNum = UnaryExp();
    int tmp = peek();
    while (tmp == MULT || tmp == DIV || tmp == MOD) {
        printParseResult("MulExp");
        now = getSym();  // now == PLUS
        UnaryExp();
        tmp = peek();
        rtNum = INT_T;
    }

    printParseResult("MulExp");
    return rtNum;
}

int AddExp() {
    int rtNum = MulExp();
    int tmp = peek();
    while (tmp == PLUS || tmp == MINU) {
        printParseResult("AddExp");
        now = getSym();  // now == PLUS || MINU
        MulExp();
        tmp = peek();
        rtNum = INT_T;
    }

    printParseResult("AddExp");
    return rtNum;
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
    int tmpLine = getErrorLine();
    int tmp = peek();
    int dimension = 0;
    if (tmp == LBRACK) {
        now = getSym();
        ConstExp();
        tmp = peek();
        dimension++;
        if (tmp != RBRACK) {
            throwError(ERROR_K, getErrorLine());
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
                throwError(ERROR_K, getErrorLine());
            } else {
                now = getSym(); // now == RBRACK
            }
        }
    }

    tmp = peek();
    if (tmp == ASSIGN) {
        now = getSym();
        ConstInitVal();
    }

    if (ifReDefine(name)) {
        throwError(ERROR_B, tmpLine);
    } else {
        if (dimension == 0) {
            appendConst(name);
        } else if (dimension == 1) {
            appendConstARR1(name);
        } else if (dimension == 2) {
            appendConstARR2(name);
        }
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
        throwError(ERROR_I, getErrorLine());
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
    int tmpLine = getErrorLine();
    int tmp = peek();
    int dimension = 0;
    if (tmp == LBRACK) {
        now = getSym();
        ConstExp();
        tmp = peek();
        if (tmp != RBRACK) {
            throwError(ERROR_K, getErrorLine());
        } else {
            now = getSym(); // now == RBRACK
        }
        tmp = peek();
        dimension++;
        if (tmp == LBRACK) {
            now = getSym();
            ConstExp();
            tmp = peek();
            if (tmp != RBRACK) {
                throwError(ERROR_K, getErrorLine());
            } else {
                now = getSym(); // now == RBRACK
            }
            dimension++;
        }
    }

    tmp = peek();
    if (tmp == ASSIGN) {
        now = getSym();
        InitVal();
    }

    if (ifReDefine(name)) {
        throwError(ERROR_B, tmpLine);
    } else {
        if (dimension == 0) {
            appendINT(name);
        } else if (dimension == 1) {
            appendARR1(name);
        } else if (dimension == 2) {
            appendARR2(name);
        }
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
        throwError(ERROR_I, getErrorLine());
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
        throwError(ERROR_B, getErrorLine());
    }
    IDENT tmpIdent;
    int dimension;

    int tmp = peek();
    if (tmp == LBRACK) {
        now = getSym();  // now == LBRACK
        tmp = peek();
        if (tmp != RBRACK) {
            throwError(ERROR_K, getErrorLine());
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
                throwError(ERROR_K, getErrorLine());
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
    appendIdent(tmpIdent);

    printParseResult("FuncFParam");
    return tmpIdent;
}

int FuncFParams(int type, string name) {
    int paramCnt = 0;
    int tmp = peek();   // tmp == INTTK || RPARENT
    while (tmp != RPARENT && tmp != LBRACE) {
        now = getSym(); // tmp == LPARENT
        IDENT tmpIdent = FuncFParam();
        tmp = peek();
        if (tmp == LBRACE) {
            throwError(ERROR_J, getErrorLine());
            break;
        }
        if (tmp != RPARENT) {
            now = getSym();
        }
        paramCnt++;
        tmp = peek();
    }

    printParseResult("FuncFParams");
    return 1;
}

int FuncDef(int type, string name) {
    if (ifReDefine(name)) {
        throwError(ERROR_B, getErrorLine());
    }
    if (type == INT_T) {
        appendFUNC_INT(name);
    } else {
        appendFUNC_VOID(name);
    }
    // now == LPARENT
    int tmp = peek();
    if (tmp != RPARENT && tmp != LBRACE) {
        FuncFParams(type, name);
    }
    tmp = peek();
    if (tmp == RPARENT) {
        now = getSym();  // now == RPARENT
    } else {
        throwError(ERROR_J, getErrorLine());
    }
    isFunc = 1;
    Block();
    printParseResult("FuncDef");
    return 1;
}

int Stmt() {
    int tmp = peek();
    int tmpReturn = 1;
    if (tmp == BREAKTK || tmp == CONTINUETK) {
        now = getSym();
        tmp = peek();
        now = getSym();  // now == SEMICN
    } else if (tmp == RETURNTK) {
        now = getSym();
        //now = getSym();
        tmp = peek();

        if (tmp != SEMICN) {
            Exp();
        }
        now = getSym(); // now == SEMICN

        tmpReturn = 2;
    } else if (tmp == PRINTFTK) {
        now = getSym(); // now == PRINTFTK
        int tmpLine = getErrorLine();
        now = getSym(); // now == LPARENT
        now = getSym(); // now == STRCON

        string strCon = getStr();
        if (!ifFormatLegal(strCon)) {
            throwError(ERROR_A, getErrorLine());
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
            throwError(ERROR_L, tmpLine);
        }

        tmp = peek();
        if (tmp != SEMICN) {
            throwError(ERROR_I, getErrorLine());
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
        Stmt();
    } else if (tmp == LBRACE) {
        Block();
    } else if (tmp == IDENFR) {
        if (!isLVal()) {
            Exp();
        } else {
            now = getSym();  //IDENFR
            int rtNum = LVal();
            if (rtNum == ERROR_H) {
                throwError(ERROR_H, getErrorLine());
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
            throwError(ERROR_I, getErrorLine());
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
            throwError(ERROR_I, getErrorLine());
        }
    }

    printParseResult("Stmt");
    return tmpReturn;
}

int Block() {
    int tmpFunc = isFunc;
    if (isFunc != 1) {
        enterBlock();
    }
    isFunc = 0;

    now = getSym();
    // now == LBRACE
    int tmp = peek();
    int last = 0;
    while (tmp != RBRACE) {
        if (tmp == CONSTTK) {
            now = getSym();
            last = ConstDecl();
        } else if (tmp == INTTK) {
            now = getSym();
            last = VarDecl();
        } else {
            last = Stmt();
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
    isFunc = 2;
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
            } else if (tmp == IDENFR) {
                //now = getSym(); // now == IDENFR
                tmp = peeeek();
                if (tmp == LPARENT) {
                    FuncType();
                    now = getSym(); // now == IDENFR
                    string name = getStr();
                    now = getSym(); // now == (
                    FuncDef(INT_T, name);
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
        }
    }
    printParseResult("CompUnit");
    return 1;
}