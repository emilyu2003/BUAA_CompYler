//
// Created by emilyu on 2022/10/26.
//
//
// Created by emilyu on 2022/9/27.
//
#include "symbol.h"
#include "parseAndSemant.h"
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
            return tmpStr;
        }
        tmpStr += Exp();
        now = getSym();  // now == RPARENT
        tmpStr += " " + getStr() + " ";
    } else {
        if (tmp == INTCON) {
            tmpStr += Number();
        } else {
            tmpStr += LVal();
        }
    }

    printParseResult("PrimaryExp");
    return tmpStr;
}

string UnaryOp() {
    now = getSym();

    printParseResult("UnaryOp");
    return getStr();
}

string UnaryExp() {
    string tmpStr;
    int tmp = peek();
    if (tmp == PLUS || tmp == MINU || tmp == NOT) {
        //now = getSym();
        tmpStr += UnaryOp() + " ";
        tmpStr += UnaryExp();
    } else {
        if (tmp == IDENFR) {
            now = getSym(); // IDENFR
            tmpStr += " " + getStr() + " ";
            tmp = peek();
            if (tmp == LPARENT) {
                now = getSym(); // LPARENT
                tmpStr += " " + getStr() + " ";
                tmp = peek();
                if (tmp != RPARENT) {
                    tmpStr += FuncRParams();
                }
                now = getSym(); // RPARENT
                tmpStr += " " + getStr() + " ";
            } else {
                tmpStr += " " + PrimaryExp() + " ";
            }
        } else {
            tmpStr += " " + PrimaryExp() + " ";
        }
    }
    printParseResult("UnaryExp");
    return tmpStr;
}

string MulExp() {
    string tmpStr;
    tmpStr += UnaryExp();
    int tmp = peek();
    while (tmp == MULT || tmp == DIV || tmp == MOD) {
        printParseResult("MulExp");
        now = getSym();  // now == PLUS
        tmpStr += " " + getStr() + " ";
        tmpStr += UnaryExp();
        tmp = peek();
    }

    printParseResult("MulExp");
    return tmpStr;
}

string AddExp() {
    string tmpStr;
    tmpStr += MulExp();
    int tmp = peek();
    while (tmp == PLUS || tmp == MINU) {
        printParseResult("AddExp");
        now = getSym();  // now == PLUS || MINU
        tmpStr += " " + getStr() + " ";
        tmpStr += MulExp();
        tmp = peek();
    }

    printParseResult("AddExp");
    return tmpStr;
}

string ConstExp() {
    string tmpStr;
    tmpStr += AddExp();

    printParseResult("ConstExp");
    return tmpStr;
}

string ConstInitVal() {
    // now == ASSIGN
    string tmpStr;
    tmpStr += getStr() + " ";
    int tmp = peek();
    if (tmp == LBRACE) {
        now = getSym();
        tmpStr += " " + getStr() + " ";
        tmp = peek();
        if (tmp != RBRACE) {
            tmpStr += ConstInitVal();
            tmp = peek();
            while (tmp == COMMA) {
                now = getSym();
                tmpStr += " " + getStr() + " ";
                tmpStr += ConstInitVal();
                tmp = peek();
            }
        }
    }
    if (tmp != LBRACE && tmp != RBRACE) {
        tmpStr += " " + ConstExp() + " ";
    }

    if (tmp == RBRACE) {
        now = getSym();
        tmpStr += " " + getStr() + " ";
    }

    printParseResult("ConstInitVal");
    return tmpStr;
}

string ConstDef() {
    string tmpStr;
    now = getSym(); //now == INTTK || COMMA
    tmpStr += " " + getStr() + " ";
    now = getSym();
    tmpStr += " " + getStr() + " ";
    int tmp = peek();
    if (tmp == LBRACK) {
        now = getSym();
        tmpStr += " " + getStr() + " ";
        tmpStr += ConstExp();
        now = getSym(); // now == RBRACK
        tmpStr += " " + getStr() + " ";
        tmp = peek();
        if (tmp == LBRACK) {
            now = getSym();
            tmpStr += " " + getStr() + " ";
            tmpStr += ConstExp();
            now = getSym(); // now == RBRACK
            tmpStr += " " + getStr() + " ";
        }
    }

    tmp = peek();
    if (tmp == ASSIGN) {
        now = getSym();
        tmpStr += " " + getStr() + " ";
        tmpStr += ConstInitVal();
    }
    printParseResult("ConstDef");
    return tmpStr;
}

string ConstDecl() {
    // now == INTTK
    string tmpStr;
    tmpStr = getStr() + " ";

    tmpStr += ConstDef() + " ";
    int tmp = peek();
    while (tmp == COMMA) {
        tmpStr += ConstDef() + " ";
        tmp = peek();
    }

    tmp = peek();
    if (tmp != SEMICN) {
        throwError(ERROR_I, getErrorLine());
    } else {
        now = getSym(); // now == SEMICN
    }

    printParseResult("ConstDecl");
    return tmpStr;
}

string InitVal() {
    string tmpStr;
    int tmp = peek();
    if (tmp == LBRACE) {
        now = getSym();
        tmpStr += " " + getStr() + " ";
        tmpStr += InitVal();
        tmp = peek();
        while (tmp == COMMA) {
            now = getSym();
            tmpStr += " " + getStr() + " ";
            tmpStr += InitVal();
            tmp = peek();
        }
    }
    if (tmp != LBRACE && tmp != RBRACE) {
        tmpStr += " " + Exp() + " ";
    }

    tmp = peek();
    if (tmp == RBRACE) {
        now = getSym();
    }
    printParseResult("InitVal");
    return tmpStr;
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