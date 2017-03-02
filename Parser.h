#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <string>
#include <set>
#include "AST.h"
#include "LexicalAnalysis.h"
#include "SymbolTable.h"
#include "Traverser.h"
#include "Error.h"
#include "Debug.h"

using namespace std;

class CParser
{
    static CASTNode* sm_pAST;
    static string sm_sCode;
    static string sm_sSymbol;
    static string sm_sValue;
    static string sm_sLocation;

    static vector<CSymbolTableItem*> sm_vSymbolTable;
    static vector<CFuncIndex*> sm_vFuncIndex;
    static bool sm_bisLeafFunc;
    static set<string> sm_sStatementFIRST;
    static set<string> sm_sCompStmtFIRST;
    static set<string> sm_sExpressionFIRST;
    static bool sm_bhasReturn;

    static void __SetCode(const string* sCode);
    static void __SetLocation(string sLocation);
    static string __Symbol();
    static string __Value();
    static string __Location();
    static bool __isMatch(string sSymbol);
    static void __SetLeafFunc(bool bLeafFunc);
    static void __SetHasReturn(bool bHasReturn);
    static void __InitialSet();

    static      void __Program();
    static CASTNode* __ConstDec();
    static CASTNode* __VarDec();
    static CASTNode* __ConstDef();
    static CASTNode* __VarDef();
    static CASTNode* __FuncDef();
    static CASTNode* __Identifier();
    static CASTNode* __Integer();
    static CASTNode* __ParaTable();
    static CASTNode* __CompStmt();
    static CASTNode* __StmtList();
    static CASTNode* __Statement();
    static CASTNode* __CondStmt();
    static CASTNode* __Condition();
    static CASTNode* __WhileStmt();
    static CASTNode* __ForStmt();
    static CASTNode* __CallStmt();
    static CASTNode* __AssignStmt();
    static CASTNode* __ScanStmt();
    static CASTNode* __PrintStmt();
    static CASTNode* __ReturnStmt();
    static CASTNode* __Expression();
    static CASTNode* __Term();
    static CASTNode* __Factor();
    
    static void __ConstDecPart(CASTNode* CONSTDECNode);
    static void __ConstDefPart(CASTNode* CONSTDEFNode, string sMode);
    static void __VarDefPart(CASTNode* VARDEFNode);
    static void __ParaTablePart(CASTNode* PARATABLENode, int nParaNum);

    static void __FillOffset();
    static void __ErrorSymbolTableItem(string sName, Catagory eCatagory, string sLocation);

#ifdef DEBUG_SYMBOL_TABLE
    static void PrintSymbolTable(ofstream &fout);
#endif

public:
    CParser();
    ~CParser();
    static void Parser(const string* sCode);
    static vector<CSymbolTableItem*> GetSymbolTable();
    static vector<CFuncIndex*> GetFuncIndex();
    static int FindNameInLocalAndGlobal(string sName, string sLocation);
    static int FindNameInLocal(string sName, string sLocation);
    static int FindNameInAll(string sName);
    static int FindNameInGlobal(string sName);
};

#endif