//
// Created by emilyu on 2022/10/26.
//
//
// Created by emilyu on 2022/9/27.
//
#include "symbol.h"
#include "parseAndSemant.h"
#include "base.h"
#include "MLRGen.h"
#include <cstdio>
#include <string>
#include <algorithm>
#include <iostream>
#include <vector>

using namespace std;

int now_S;

void printParseResult_S(const string &s) {

}

/* end 默认包括; */
string RelExp_S() {
    string tmpStr;
    tmpStr += AddExp_S();
    int tmp = peek();
    while (tmp == LEQ || tmp == GEQ || tmp == LSS || tmp == GRE) {
        printParseResult_S("RelExp");
        now_S = getSym();
        string sym = getStr();
        tmpStr += " " + AddExp_S() + " ";
        tmpStr += " " + sym + " ";
        tmp = peek();
    }

    printParseResult_S("RelExp");
    return tmpStr;
}

string EqExp_S() {
    string tmpStr;
    tmpStr += RelExp_S();
    int tmp = peek();
    while (tmp == EQL || tmp == NEQ) {
        printParseResult_S("EqExp");
        now_S = getSym();
        string sym = getStr();
        tmpStr += " " + RelExp_S() + " ";
        tmpStr += " " + sym + " ";
        tmp = peek();
    }
    printParseResult_S("EqExp");
    return tmpStr;
}

string LAndExp_S() {
    string tmpStr;
    tmpStr += EqExp_S();
    int tmp = peek();
    while (tmp == AND) {
        printParseResult_S("LAndExp_S");
        now_S = getSym();
        string sym = getStr();
        tmpStr += " " + EqExp_S() + " ";
        tmpStr += " " + sym + " ";
        tmp = peek();
    }

    printParseResult_S("LAndExp_S");
    return tmpStr;
}

string LOrExp_S() {
    string tmpStr;
    tmpStr += LAndExp_S();
    int tmp = peek();
    while (tmp == OR) {
        printParseResult_S("LOrExp_S");
        now_S = getSym();
        string sym = getStr();
        tmpStr += " " + LAndExp_S() + " ";
        tmpStr += " " + sym + " ";
        tmp = peek();
    }

    printParseResult_S("LOrExp_S");
    return tmpStr;
}

string Exp_S() {
    string tmpStr;
    tmpStr += AddExp_S();

    printParseResult_S("Exp_S");
    return tmpStr;
}

string Cond_S() {
    string tmpStr;
    tmpStr += LOrExp_S();

    printParseResult_S("Cond_S");
    return tmpStr;
}

string LVal_S() {
    // now_S = IDENFR
    string tmpStr = getStr();
    int tmp = peek();
    if (tmp == LBRACK) {
        now_S = getSym();
        tmpStr += " " + getStr() + " ";
        tmpStr += Exp_S();
        now_S = getSym(); // now_S == RBRACK
        tmpStr += " " + getStr() + " ";
        tmp = peek();
        if (tmp == LBRACK) {
            now_S = getSym();
            tmpStr += " " + getStr() + " ";
            tmpStr += Exp_S();
            now_S = getSym(); // now_S == RBRACK
            tmpStr += " " + getStr() + " ";
        }
    }

    printParseResult_S("LVal_S");
    return tmpStr;
}

string Number_S() {
    string tmpStr;
    now_S = getSym();
    tmpStr += getStr();
    printParseResult_S("Number_S");
    return tmpStr;
}


string FuncRParams_S() {
    string tmpStr;
    int tmp = peek();   // tmp == IDENFR
    while (tmp != RPARENT) {    // TODO dont know how to get type of RPARAMS
        tmpStr += Exp_S();
        now_S = getSym();  // now_S == COMMA
        tmpStr += " " + getStr() + " ";
        tmp = peek();
    }
    printParseResult_S("FuncRParams_S");
    return tmpStr;
}

string PrimaryExp_S() {
    string tmpStr;
    int tmp = peek();
    if (tmp == LPARENT) {
        now_S = getSym();
        tmp = peek();
        if (tmp == RPARENT) {
            now_S = getSym();
            return tmpStr;
        }
        tmpStr += Exp_S();
        now_S = getSym();  // now_S == RPARENT
        //tmpStr += " " + getStr() + " ";
    } else {
        if (tmp == INTCON) {
            tmpStr += Number_S();
        } else {
            tmpStr += LVal_S();
        }
    }

    printParseResult_S("PrimaryExp_S");
    return tmpStr;
}

string UnaryOp_S() {
    now_S = getSym();

    printParseResult_S("UnaryOp_S");
    return getStr();
}

string UnaryExp_S() {
    string tmpStr;
    int tmp = peek();
    if (tmp == PLUS || tmp == MINU || tmp == NOT) {
        //now_S = getSym();
        tmpStr += UnaryOp_S();
        tmpStr += UnaryExp_S();
    } else {
        if (tmp == IDENFR) {
            now_S = getSym(); // IDENFR
            tmp = peek();
            if (tmp == LPARENT) {
                tmpStr += " " + getStr() + " ";
                now_S = getSym(); // LPARENT
                tmpStr += " " + getStr() + " ";
                tmp = peek();
                if (tmp != RPARENT) {
                    tmpStr += FuncRParams_S();
                }
                now_S = getSym(); // RPARENT
                tmpStr += " " + getStr() + " ";
            } else {
                tmpStr += " " + PrimaryExp_S() + " ";
            }
        } else {
            tmpStr += " " + PrimaryExp_S() + " ";
        }
    }
    printParseResult_S("UnaryExp_S");
    return tmpStr;
}

string MulExp_S() {
    string tmpStr;
    tmpStr += UnaryExp_S();
    int tmp = peek();
    while (tmp == MULT || tmp == DIV || tmp == MOD) {
        printParseResult_S("MulExp_S");
        now_S = getSym();  // now_S == PLUS
        string sym = getStr();
        tmpStr += " " + UnaryExp_S() + " ";
        tmpStr += " " + sym + " ";
        tmp = peek();
    }

    printParseResult_S("MulExp_S");
    return tmpStr;
}

string AddExp_S() {
    string tmpStr;
    tmpStr += MulExp_S();
    int tmp = peek();
    while (tmp == PLUS || tmp == MINU) {
        printParseResult_S("AddExp_S");
        now_S = getSym();  // now_S == PLUS || MINU
        string sym = getStr();
        tmpStr += " " + MulExp_S() + " ";
        tmpStr += " " + sym + " ";
        tmp = peek();
    }

    printParseResult_S("AddExp_S");
    return tmpStr;
}

string ConstExp_S() {
    string tmpStr;
    tmpStr += AddExp_S();

    printParseResult_S("ConstExp_S");
    return tmpStr;
}

string ConstInitVal_S() {
    // now_S == ASSIGN
    string tmpStr;
    int tmp = peek();
    if (tmp == LBRACE) {
        now_S = getSym();
        tmpStr += " " + getStr() + " ";
        tmp = peek();
        if (tmp != RBRACE) {
            tmpStr += ConstInitVal_S();
            tmp = peek();
            while (tmp == COMMA) {
                now_S = getSym();
                tmpStr += " " + getStr() + " ";
                tmpStr += ConstInitVal_S();
                tmp = peek();
            }
        }
    }
    if (tmp != LBRACE && tmp != RBRACE) {
        tmpStr += " " + ConstExp_S() + " ";
    }

    if (tmp == RBRACE) {
        now_S = getSym();
        tmpStr += " " + getStr() + " ";
    }

    printParseResult_S("ConstInitVal_S");
    return tmpStr;
}

string ConstDef_S() {
    string tmpStr;
    now_S = getSym(); //now_S == INTTK || COMMA
    if (getStr() != "int") {
        tmpStr += " " + getStr() + " ";
    }
    now_S = getSym();
    tmpStr += " " + getStr() + " ";
    int tmp = peek();
    if (tmp == LBRACK) {
        now_S = getSym();
        tmpStr += " " + getStr() + " ";
        tmpStr += ConstExp_S();
        now_S = getSym(); // now_S == RBRACK
        tmpStr += " " + getStr() + " ";
        tmp = peek();
        if (tmp == LBRACK) {
            now_S = getSym();
            tmpStr += " " + getStr() + " ";
            tmpStr += ConstExp_S();
            now_S = getSym(); // now_S == RBRACK
            tmpStr += " " + getStr() + " ";
        }
    }

    tmp = peek();
    if (tmp == ASSIGN) {
        now_S = getSym();
        tmpStr += " " + getStr() + " ";
        tmpStr += ConstInitVal_S();
    }
    printParseResult_S("ConstDef_S");
    return tmpStr;
}

string ConstDecl_S() {
    // now_S == INTTK
    string tmpStr;
    tmpStr += ConstDef_S() + " ";
    int tmp = peek();
    while (tmp == COMMA) {
        tmpStr += ConstDef_S() + " ";
        tmp = peek();
    }

    now_S = getSym(); // now_S == SEMICN
    tmpStr += " " + getStr() + " ";

    printParseResult_S("ConstDecl_S");
    return tmpStr;
}

string InitVal_S() {
    string tmpStr;
    int tmp = peek();
    if (tmp == LBRACE) {
        now_S = getSym();
        tmpStr += " " + getStr() + " ";
        tmpStr += InitVal_S();
        tmp = peek();
        while (tmp == COMMA) {
            now_S = getSym();
            tmpStr += " " + getStr() + " ";
            tmpStr += InitVal_S();
            tmp = peek();
        }
    }
    if (tmp != LBRACE && tmp != RBRACE) {
        tmpStr += " " + Exp_S() + " ";
    }

    tmp = peek();
    if (tmp == RBRACE) {
        now_S = getSym();
    }
    printParseResult_S("InitVal_S");
    return tmpStr;
}

string VarDef_S() {
    // now_S == INTTK
    int dimension = 0;
    string tmpStr;
    now_S = getSym();  // now_S = IDENFR
    genVarCode(getStr());
    string name = getStr();
    tmpStr += " " + getStr() + " ";
    int tmp = peek();
    if (tmp == LBRACK) {
        now_S = getSym();
        tmpStr += " " + getStr() + " ";
        tmpStr += ConstExp_S();
        now_S = getSym(); // now_S == RBRACK
        tmpStr += " " + getStr() + " ";
        tmp = peek();
        dimension++;
        if (tmp == LBRACK) {
            now_S = getSym();
            tmpStr += " " + getStr() + " ";
            tmpStr += ConstExp_S();
            now_S = getSym(); // now_S == RBRACK
            tmpStr += " " + getStr() + " ";
            dimension++;
        }
    }

    tmp = peek();
    string expCode;
    if (tmp == ASSIGN) {
        now_S = getSym();
        tmpStr += " " + getStr() + " ";
        //tmpStr += InitVal_S();
        expCode = InitVal_S();
        tmpStr += expCode;
        genAssignCode(name, expCode, dimension);
    }

    printParseResult_S("VarDef_S");
    return tmpStr;
}

string VarDecl_S() {
    // now_S == INTTK
    string tmpStr;
    tmpStr += " " + getStr() + " ";
    tmpStr += VarDef_S();
    now_S = getSym();
    tmpStr += " " + getStr() + " ";
    while (now_S == COMMA) {
        tmpStr += VarDef_S();
        now_S = getSym();
        tmpStr += " " + getStr() + " ";
    }

    printParseResult_S("VarDecl_S");
    return tmpStr;
}

string FuncFParam_S() {
    // now_S == INTTK
    string tmpStr;
    tmpStr += getStr() + " ";
    now_S = getSym();  // now_S == IDENFR
    tmpStr += " " + getStr() + " ";
    int tmp = peek();
    if (tmp == LBRACK) {
        now_S = getSym();  // now_S == LBRACK
        tmpStr += " " + getStr() + " ";
        now_S = getSym(); // now_S == RBRACK
        tmpStr += " " + getStr() + " ";
        tmp = peek();
        if (tmp == LBRACK) {
            now_S = getSym();  // now_S == LBRACK
            tmpStr += " " + getStr() + " ";
            tmpStr += ConstExp_S();

            now_S = getSym(); // now_S == RBRACK
            tmpStr += " " + getStr() + " ";
        }
    }
    genFuncParamCode("int", tmpStr.substr(3, tmpStr.size() - 3));
    printParseResult_S("FuncFParam_S");
    return tmpStr;
}

string FuncFParams_S() {
    string tmpStr;
    int tmp = peek();   // tmp == INTTK || RPARENT
    while (tmp != RPARENT && tmp != LBRACE) {
        now_S = getSym(); // tmp == LPARENT
        tmpStr += " " + getStr() + " ";
        tmpStr += FuncFParam_S();
        tmp = peek();
        if (tmp != RPARENT) {
            now_S = getSym();
            tmpStr += " " + getStr() + " ";
        }
        tmp = peek();
    }

    printParseResult_S("FuncFParams_S");
    return tmpStr;
}

string FuncDef_S() {
    // now_S == LPARENT
    string tmpStr;
    int tmp = peek();
    if (tmp != RPARENT && tmp != LBRACE) {
        tmpStr += FuncFParams_S();
    }
    now_S = getSym();  // now_S == RPARENT
    tmpStr += " " + getStr() + " ";

    tmpStr += Block_S();
    printParseResult_S("FuncDef_S");
    return tmpStr;
}

string Stmt_S() {
    string tmpStr;
    int tmp = peek();
    if (tmp == BREAKTK || tmp == CONTINUETK) {
        now_S = getSym();
        tmpStr += " " + getStr() + " ";
        now_S = getSym();  // now_S == SEMICN
        tmpStr += " " + getStr() + " ";
    } else if (tmp == RETURNTK) {
        now_S = getSym();
        tmpStr += " " + getStr() + " ";
        tmp = peek();
        if (tmp != SEMICN) {
            tmpStr += Exp_S();
        }
        now_S = getSym(); // now_S == SEMICN
        tmpStr += " " + getStr() + " ";
    } else if (tmp == PRINTFTK) {
        now_S = getSym(); // now_S == PRINTFTK
        tmpStr += " " + getStr() + " ";
        now_S = getSym(); // now_S == LPARENT
        tmpStr += " " + getStr() + " ";
        now_S = getSym(); // now_S == STRCON
        tmpStr += " " + getStr() + " ";
        string strCon = getStr();

        utils.clear();
        tmp = peek();
        if (tmp != RPARENT) {
            now_S = getSym();
            tmpStr += " " + getStr() + " ";
            while (now_S == COMMA) {
                string para = Exp_S();
                tmpStr += para;
                utils.push_back(para);
                now_S = getSym();
                tmpStr += " " + getStr() + " ";
            }
        } else {
            now_S = getSym();
            tmpStr += " " + getStr() + " ";
        }
        now_S = getSym(); // now_S == SEMICN
        tmpStr += " " + getStr() + " ";

        genPrintfCode(strCon);
    } else if (tmp == IFTK) {
        now_S = getSym();  // now_S == IFTK
        tmpStr += " " + getStr() + " ";
        now_S = getSym(); // now_S == LPARENT
        tmpStr += " " + getStr() + " ";
        tmpStr += Cond_S();
        now_S = getSym(); // now_S == RPARENT
        tmpStr += " " + getStr() + " ";
        tmpStr += Stmt_S();
        tmp = peek();
        if (tmp == ELSETK) {
            now_S = getSym(); // now_S == ELSETK
            tmpStr += " " + getStr() + " ";
            tmpStr += Stmt_S();
        }
    } else if (tmp == WHILETK) {
        now_S = getSym();  // now_S == WHILETK
        tmpStr += " " + getStr() + " ";
        now_S = getSym(); // now_S == LPARENT
        tmpStr += " " + getStr() + " ";
        tmpStr += Cond_S();
        // now_S == RPARENT
        now_S = getSym();
        tmpStr += " " + getStr() + " ";
        tmpStr += Stmt_S();
    } else if (tmp == LBRACE) {
        tmpStr += Block_S();
    } else if (tmp == IDENFR) {
        if (!isLVal()) {
            tmpStr += Exp_S();
        } else {
            now_S = getSym();  //IDENFR
            tmpStr += LVal_S();   // contains identifier
            now_S = getSym(); // now_S == ASSIGN
            tmpStr += " " + getStr() + " ";
            tmp = peek();
            if (tmp == GETINTTK) {
                now_S = getSym(); // now_S == GETINTTK
                tmpStr += " " + getStr() + " ";
                now_S = getSym(); // now_S == LPARENT
                tmpStr += " " + getStr() + " ";
                now_S = getSym(); // now_S == RPARENT
                tmpStr += " " + getStr() + " ";
                //now_S = getSym(); // now_S == SEMICN
            } else {
                tmpStr += " " + Exp_S() + " ";
            }
        }
        now_S = getSym();
        tmpStr += " " + getStr() + " ";
    } else {
        tmp = peek();
        if (tmp != SEMICN) {
            tmpStr += Exp_S();
        }
        now_S = getSym();
        tmpStr += " " + getStr() + " ";
    }

    printParseResult_S("Stmt_S");
    return tmpStr;
}

string Block_S() {
    string tmpStr;
    now_S = getSym(); // now_S == LBRACE
    tmpStr += " " + getStr() + " ";
    int tmp = peek();
    while (tmp != RBRACE) {
        if (tmp == CONSTTK) {
            now_S = getSym();
            tmpStr += ConstDecl_S();
        } else if (tmp == INTTK) {
            now_S = getSym();
            tmpStr += VarDecl_S();
        } else {
            tmpStr += Stmt_S();
        }
        tmp = peek();
    }
    now_S = getSym();  // now_S == RBRACE
    tmpStr += " " + getStr() + " ";

    printParseResult_S("Block_S");
    return tmpStr;
}

string MainFuncDef_S() {
    // now_S == INTTK
    string tmpStr;
    now_S = getSym(); // now_S == MAINTK
    genString("\nMAIN");
    tmpStr += " " + getStr() + " ";
    now_S = getSym(); // now_S == LPARENT
    tmpStr += " " + getStr() + " ";
    now_S = getSym(); // now_S == RPARENT
    tmpStr += " " + getStr() + " ";
    tmpStr += Block_S();
    // now_S == RBRACE
    printParseResult_S("MainFuncDef_S");
    return tmpStr;
}

string FuncType_S() {
    printParseResult_S("FuncType_S");
    return " ";
}

string parseAndSemant() {
    lexerPos = 0;
    string tmpStr;
    while (lexerPos < lexerLen) {
        now_S = getSym();
        if (now_S == CONSTTK) {
            tmpStr += " " + getStr() + " ";
            tmpStr += ConstDecl_S();
        } else if (now_S == INTTK) {
            int tmp = peek();
            if (tmp == MAINTK) {
                tmpStr += " " + getStr() + " ";
                tmpStr += " " + MainFuncDef_S() + " ";
            } else if (tmp == IDENFR) {
                //now_S = getSym(); // now_S == IDENFR
                tmp = peeeek();
                if (tmp == LPARENT) {
                    tmpStr += " " + getStr() + " ";
                    tmpStr += FuncType_S();
                    now_S = getSym(); // now_S == IDENFR
                    tmpStr += " " + getStr() + " ";
                    genFuncDeclCode("int", getStr());
                    now_S = getSym(); // now_S == (
                    tmpStr += " " + getStr() + " ";
                    tmpStr += FuncDef_S();
                } else {
                    // now_S == INTTK
                    tmpStr += VarDecl_S();
                }
            }
        } else if (now_S == VOIDTK) {
            tmpStr += FuncType_S();
            now_S = getSym();  // now_S == IDENFR
            tmpStr += " " + getStr() + " ";
            genFuncDeclCode("void", getStr());
            now_S = getSym();  // now_S == (
            tmpStr += " " + getStr() + " ";
            tmpStr += FuncDef_S();
        }
    }
    printParseResult_S("CompUnit");
    return tmpStr;
}