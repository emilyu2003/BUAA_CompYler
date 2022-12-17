//
// Created by emilyu on 2022/9/27.
//
#include "symbol.h"
#include "parseAndErr.h"
#include "base.h"
#include "IdentTable.h"
#include <cstdio>
#include <string>
#include <algorithm>
#include <utility>
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;

int now;
bool funcVoid = false;
int isFunc = 0;
int isWhile = 0;

void printParseResult(string s) {
    string output = "<" + s + ">" + "\n";
//    FILE *f = fopen("output.txt", "a");
//    fprintf(f, "%s", output.c_str());
//    fclose(f);
//    cout << output;
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
    int rtNum = AddExp();

    printParseResult("Exp");
    return rtNum;
}

int Cond() {
    LOrExp();

    printParseResult("Cond");
    return 1;
}

int LVal() {
    // now = IDENFR
    string name = getStr();
    IDENT tmpLVal;
    int flag = 0;
    if (!ifExist(name)) {
        throwError(ERROR_C, getErrorLine());
        //flag = 2;
    } else {
        tmpLVal = getIdentTemporarily(name);
    }

    int rtNum = INT_T;
    if (ifConst(getStr())) {
        flag = 1;
    }
    int dimension = 0;

    int tmp = peek();
    if (tmp == LBRACK) {
        now = getSym();
        Exp();
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
            Exp();
            tmp = peek();
            dimension++;
            if (tmp != RBRACK) {
                throwError(ERROR_K, getErrorLine());
            } else {
                now = getSym(); // now == RBRACK
            }
        }
    }

    printParseResult("LVal");
    if (flag == 2) return rtNum;
    if (flag) {
        rtNum = ERROR_H;
    } else {
        int dim = tmpLVal.type;
        if (dim == INTTK) {
            if (dimension == 0) rtNum = INTTK;
            else {
                rtNum = ERROR_E;
            }
        } else if (dim == ARRAY_T_D1) {
            if (dimension == 0) rtNum = ARRAY_T_D1;
            else if (dimension == 1) rtNum = INTTK;
            else {
                rtNum = ERROR_E;
            }
        } else if (dim == ARRAY_T_D2) {
            if (dimension == 0) rtNum = ARRAY_T_D2;
            else if (dimension == 1) rtNum = ARRAY_T_D1;
            else rtNum = INTTK;
        }
    }
    return rtNum;
}

int Number() {
    now = getSym();
    printParseResult("Number");
    return INT_T;
}


vector<int> FuncRParams() {
    vector<int> FuncRParamsType;
    int tmp = peek();   // tmp == IDENFR
    int paramCnt = 0;
    while (tmp == IDENFR || tmp == LPARENT || tmp == PLUS || tmp == MINU ||
           tmp == INTCON) {
        FuncRParamsType.push_back(Exp());
        tmp = peek();
        if (tmp == COMMA) {
            now = getSym();  // now == COMMA
        }
        paramCnt++;
        tmp = peek();
    }

    printParseResult("FuncRParams");
    return FuncRParamsType;
}

int PrimaryExp() {
    int rtNum = INT_T;
    int tmp = peek();
    if (tmp == LPARENT) {
        now = getSym();
        tmp = peek();
        if (tmp == RPARENT) {
            now = getSym();
            return 1;
        }
        rtNum = Exp();
        now = getSym();  // now == RPARENT
    } else {
        if (tmp == INTCON) {
            rtNum = Number();
        } else {
            rtNum = LVal();
//            if (rtNum == ERROR_H) {
//                throwError(ERROR_H, getErrorLine());
//            }
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
                if (!ifParamCntCoordinate(name, tmpParams.size())) {
                    throwError(ERROR_D, tmpLine);
                } else if (!ifParamTypeCoordinate(name, tmpParams)) {
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
    int tmp = peek();
    while (tmp == COMMA) {
        now = getSym();
        VarDef();
        tmp = peek();
    }

    if (tmp != SEMICN) {
        throwError(ERROR_I, getErrorLine());
    } else {
        now = getSym();
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
        tmpIdent = {name, INT_T};
    } else if (dimension == 2) {
        tmpIdent = {name, ARRAY_T_D1};
    } else {
        tmpIdent = {name, ARRAY_T_D2};
    }
    appendIdent(tmpIdent);

    printParseResult("FuncFParam");
    return tmpIdent;
}

int FuncFParams(string name) {
    int paramCnt = 0;
    int tmp = peek();   // tmp == INTTK || RPARENT
    while (tmp != RPARENT && tmp != LBRACE) {
        now = getSym(); // tmp == LPARENT
        IDENT tmpIdent = FuncFParam();
        pushParam(name, tmpIdent);
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
    updateFunc((name), paramCnt);
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
        FuncFParams(name);
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
        if (isWhile == 0) {
            throwError(ERROR_M, getErrorLine());
        }
        tmp = peek();
        if (tmp != SEMICN) {
            throwError(ERROR_I, getErrorLine());
        } else {
            now = getSym();  // now == SEMICN
        }
    } else if (tmp == RETURNTK) {
        now = getSym();
        //now = getSym();
        tmp = peek();
        if (tmp == RBRACE) {
            throwError(ERROR_I, getErrorLine());
        } else {
            if (tmp != SEMICN) {
                if (funcVoid) {
                    throwError(ERROR_F, getErrorLine());
                }
                Exp();
            }
            tmp = peek();
            if (tmp == SEMICN) {
                now = getSym();
            } else {
                throwError(ERROR_I, getErrorLine());
            }
        }
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

    if (tmpFunc && !funcVoid && last != 2) {
        //printf("%d %d %d\n", tmpFunc, funcVoid, last);
        throwError(ERROR_G, getErrorLine());
    }
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

int parseAndErr() {
    maxBlockNum = 0;
    tmpBlockNums.clear();
    tmpBlockNums.push_back(0);
    lexerPos = 0;
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
            funcVoid = true;
            FuncDef(VOID_T, name);
            funcVoid = false;
        }
    }
    printParseResult("CompUnit");
    return 1;
}