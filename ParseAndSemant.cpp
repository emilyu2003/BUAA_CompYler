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
#include "IdentTable.h"
#include <cstdio>
#include <string>
#include <algorithm>
#include <iostream>
#include <vector>

using namespace std;

int now_S;
int isFunc_S;
vector<string> whileEndCnt;
vector<string> whileStartCnt;
vector<string> whileDoCnt;

void printParseResult_S(string s) {

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
        tmpStr += getStr();
        tmpStr += " " + EqExp_S() + " ";
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
        tmpStr += getStr();
        tmpStr += " " + LAndExp_S() + " ";
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

vector<string> FuncRParams_S() {
    vector<string> utils;
    string tmpStr;
    int tmp = peek();   // tmp == IDENFR
    while (tmp != RPARENT) {    // TODO dont know how to get type of RPARAMS
        string expCode;
        expCode = Exp_S();
        tmpStr += expCode;
        utils.push_back(expCode);
        tmp = peek();
        if (tmp == COMMA) {
            now_S = getSym();  // now_S == COMMA
            tmpStr += " " + getStr() + " ";
        }
        tmp = peek();
    }
    printParseResult_S("FuncRParams_S");
    return utils;
}

string PrimaryExp_S() {
    string tmpStr;
    int tmp = peek();
    if (tmp == LPARENT) {
        tmpStr += " ";
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
    string tmpStr, ttmpStr;
    int flagFunc = 0;
    int tmp = peek();
    if (tmp == PLUS || tmp == MINU || tmp == NOT) {
        //now_S = getSym();
        string sym = UnaryOp_S();
        //tmpStr += UnaryOp_S();
        tmpStr += " 0 ";
        tmpStr += UnaryExp_S() + " ";
        tmpStr += " " + sym + " ";
    } else {
        if (tmp == IDENFR) {
            string name;
            now_S = getSym(); // IDENFR
            name = getStr();
            tmp = peek();
            if (tmp == LPARENT) {
                //tmpStr += getStr() + " ";
                now_S = getSym(); // LPARENT
                //tmpStr += " " + getStr() + " ";
                tmp = peek();
                vector<string> utils;
                if (tmp != RPARENT) {
                    utils = FuncRParams_S();
                    //tmpStr += FuncRParams_S() + " ";
                    //tmpStr += genCallFuncCode(name);
                }
                flagFunc = 1;
                tmpStr += genCallFuncCode(name, utils);
                now_S = getSym(); // RPARENT
                //tmpStr += " " + getStr() + " ";
            } else {
                tmpStr += PrimaryExp_S() + " ";
            }
        } else {
            tmpStr += PrimaryExp_S() + " ";
        }
    }
    printParseResult_S("UnaryExp_S");
    //if (flagFunc) return ttmpStr;
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
    string tmpStr, expCode;
    int dimension = 0;
    now_S = getSym(); //now_S == INTTK || COMMA
    if (getStr() != "int") {
        tmpStr += " " + getStr() + " ";
    }
    now_S = getSym();
    tmpStr += " " + getStr() + " ";
    string name = getStr();
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

    if (dimension == 0) {
        appendConst(name);
    } else if (dimension == 1) {
        appendConstARR1(name);
    } else if (dimension == 2) {
        appendConstARR2(name);
    }
    genConstCode(name);

    tmp = peek();
    if (tmp == ASSIGN) {
        now_S = getSym();
        tmpStr += " " + getStr() + " ";
        expCode = ConstInitVal_S();
        tmpStr += expCode;
        genAssignCode(name, expCode, dimension);
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
    string name = getStr();
    tmpStr += " " + getStr() + " ";
    int tmp = peek();
    string len1, len2;
    if (tmp == LBRACK) {
        now_S = getSym();
        tmpStr += " " + getStr() + " ";
        len1 = ConstExp_S();
        tmpStr += len1;
        now_S = getSym(); // now_S == RBRACK
        tmpStr += " " + getStr() + " ";
        tmp = peek();
        dimension++;
        if (tmp == LBRACK) {
            now_S = getSym();
            tmpStr += " " + getStr() + " ";
            len2 = ConstExp_S();
            tmpStr += len2;
            now_S = getSym(); // now_S == RBRACK
            tmpStr += " " + getStr() + " ";
            dimension++;
        }
    }

    if (dimension == 0) {
        appendINT(name);
    } else if (dimension == 1) {
        appendARR1(name);
        updateArrD1(name, len1);
    } else if (dimension == 2) {
        appendARR2(name);
        updateArrD2(name, len1, len2);
    }

    genVarCode(name);

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
    int dimension = 0;
    string tmpStr;
    tmpStr += getStr() + " ";
    now_S = getSym();  // now_S == IDENFR
    string name = getStr();
    tmpStr += " " + getStr() + " ";
    int tmp = peek();
    if (tmp == LBRACK) {
        now_S = getSym();  // now_S == LBRACK
        tmpStr += " " + getStr() + " ";
        now_S = getSym(); // now_S == RBRACK
        tmpStr += " " + getStr() + " ";
        tmp = peek();
        dimension++;
        if (tmp == LBRACK) {
            now_S = getSym();  // now_S == LBRACK
            tmpStr += " " + getStr() + " ";
            tmpStr += ConstExp_S();

            now_S = getSym(); // now_S == RBRACK
            tmpStr += " " + getStr() + " ";
            dimension++;
        }
    }

    IDENT tmpIdent;
    if (dimension == 0) {
        tmpIdent = {name, INT_T};
    } else if (dimension == 2) {
        tmpIdent = {name, ARRAY_T_D1};
    } else {
        tmpIdent = {name, ARRAY_T_D2};
    }
    appendIdent(tmpIdent);

    genFuncParamCode("int", tmpStr.substr(3, tmpStr.size() - 3));
    printParseResult_S("FuncFParam_S");
    return tmpStr;
}

string FuncFParams_S(string name) {
    int paramCnt = 0;
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
        paramCnt++;
        tmp = peek();
    }

    updateFunc((name), paramCnt);
    printParseResult_S("FuncFParams_S");
    return tmpStr;
}

string FuncDef_S(int type, string name) {
    // now_S == LPARENT
    if (type == INT_T) {
        appendFUNC_INT(name);
    } else {
        appendFUNC_VOID(name);
    }

    string tmpStr;
    int tmp = peek();
    if (tmp != RPARENT && tmp != LBRACE) {
        tmpStr += FuncFParams_S(name);
    }
    now_S = getSym();  // now_S == RPARENT
    tmpStr += " " + getStr() + " ";

    isFunc_S = 1;
    tmpStr += Block_S();
    printParseResult_S("FuncDef_S");
    return tmpStr;
}

string Stmt_S() {
    string tmpStr;
    int tmp = peek();
    if (tmp == BREAKTK || tmp == CONTINUETK) {
        if (tmp == BREAKTK) {
            genString("j " + whileEndCnt.back());
        } else {
            genString("j " + whileStartCnt.back());
        }
        now_S = getSym();
        tmpStr += " " + getStr() + " ";
        now_S = getSym();  // now_S == SEMICN
        tmpStr += " " + getStr() + " ";
    } else if (tmp == RETURNTK) {
        now_S = getSym();
        tmpStr += " " + getStr() + " ";
        tmp = peek();
        string expCode;
        if (tmp != SEMICN) {
            expCode = Exp_S();
            tmpStr += expCode;
        }
        now_S = getSym(); // now_S == SEMICN
        tmpStr += " " + getStr() + " ";
        genReturnCode(expCode);
    } else if (tmp == PRINTFTK) {
        now_S = getSym(); // now_S == PRINTFTK
        tmpStr += " " + getStr() + " ";
        now_S = getSym(); // now_S == LPARENT
        tmpStr += " " + getStr() + " ";
        now_S = getSym(); // now_S == STRCON
        tmpStr += " " + getStr() + " ";
        string strCon = getStr();

        vector<string> k;
        tmp = peek();
        if (tmp != RPARENT) {
            now_S = getSym();
            tmpStr += " " + getStr() + " ";
            while (now_S == COMMA) {
                string para = Exp_S();
                tmpStr += para;
                k.push_back(para);
                now_S = getSym();
                tmpStr += " " + getStr() + " ";
            }
        } else {
            now_S = getSym();
            tmpStr += " " + getStr() + " ";
        }
        now_S = getSym(); // now_S == SEMICN
        tmpStr += " " + getStr() + " ";

        genPrintfCode(strCon, k);
    } else if (tmp == IFTK) {
        string ifStartStr = getName("START_IF_");
        string ifEndStr = getName("END_IF_");
        string elseStartStr = getName("START_ELSE_");
        string cond;
        now_S = getSym();  // now_S == IFTK
        tmpStr += " " + getStr() + " ";
        now_S = getSym(); // now_S == LPARENT
        tmpStr += " " + getStr() + " ";
        cond = Cond_S();
        tmpStr += cond;
        now_S = getSym(); // now_S == RPARENT

        genCondCode(cond, ifStartStr, elseStartStr, ifEndStr);
        genString(ifStartStr + ":");

        tmpStr += " " + getStr() + " ";
        tmpStr += Stmt_S();
        tmp = peek();
        genString("j " + ifEndStr);
        genString(elseStartStr + ":");
        if (tmp == ELSETK) {
            now_S = getSym(); // now_S == ELSETK
            tmpStr += " " + getStr() + " ";
            tmpStr += Stmt_S();
        }
        genString(ifEndStr + ":");
    } else if (tmp == WHILETK) {
        string startWhileStr = getName("START_WHILE_");
        string doWhileStr = getName("DO_WHILE_");
        string endWhileStr = getName("END_WHILE_");
        whileStartCnt.push_back(startWhileStr);
        whileDoCnt.push_back(doWhileStr);
        whileEndCnt.push_back(endWhileStr);
        string cond;

        genString(whileStartCnt.back() + ":");
        now_S = getSym();  // now_S == WHILETK
        tmpStr += " " + getStr() + " ";
        now_S = getSym(); // now_S == LPARENT
        tmpStr += " " + getStr() + " ";
        cond = Cond_S(); // TODO
        tmpStr += cond;

        genCondCode(cond, whileDoCnt.back(), "", whileEndCnt.back());
        genString(doWhileStr + ":");

        // now_S == RPARENT
        now_S = getSym();
        tmpStr += " " + getStr() + " ";
        tmpStr += Stmt_S();
        genString("j " + whileStartCnt.back());
        genString(whileEndCnt.back() + ":");
        whileEndCnt.pop_back();
        whileStartCnt.pop_back();
    } else if (tmp == LBRACE) {
        tmpStr += Block_S();
    } else if (tmp == IDENFR) {
        if (!isLVal()) {
            tmpStr += Exp_S();
        } else {
            now_S = getSym();  //IDENFR
            string name;
            name = LVal_S();   // contains identifier
            tmpStr += name;
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
                string tt = genScanfCode();
                genAssignCode(name, tt, 0);
            } else {
                int dim = 0;
                for (char i: name) {
                    if (i == '[') dim++;
                }
                string expCode;
                expCode = " " + Exp_S() + " ";
                tmpStr += " " + expCode + " ";
                genAssignCode(name, expCode, dim);
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
    int tmpFunc = isFunc_S;
    if (isFunc_S != 1) {
        enterBlock();
    }
    isFunc_S = 0;
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

    if (tmpFunc) {
        genReturnCode("");
    }

    endBlock();
    printParseResult_S("Block_S");
    return tmpStr;
}

string MainFuncDef_S() {
    // now_S == INTTK
    string tmpStr;
    now_S = getSym(); // now_S == MAINTK
    genString("\nMAIN:");
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
    maxBlockNum = 0;
    tmpBlockNums.clear();
    tmpBlockNums.push_back(0);
    identTable.clear();
    identTableCnt.clear();
    tAssign = getName("t_");
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
                    string name = getStr();
                    now_S = getSym(); // now_S == (
                    tmpStr += " " + getStr() + " ";
                    tmpStr += FuncDef_S(INT_T, name);
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
            string name = getStr();
            now_S = getSym();  // now_S == (
            tmpStr += " " + getStr() + " ";
            tmpStr += FuncDef_S(VOID_T, name);
        }
    }
    printParseResult_S("CompUnit");
    return tmpStr;
}