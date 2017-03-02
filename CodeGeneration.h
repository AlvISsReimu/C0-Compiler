#ifndef CODEGENERATION_H
#define CODEGENERATION_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <set>
#include "Quadruple.h"
#include "SymbolTable.h"
#include "Parser.h"
#include "BasicBlock.h"

using namespace std;

#ifdef DEBUG_OPTIMIZE

#define TEMP_VAR_POOL_SIZE 5
#define GLOBAL_VAR_POOL_SIZE 3
#define RIG_MAX_SIZE 15

class CTempReg
{
    int m_nCounter;
    CSymbolTableItem* m_pTempVar;

public:
    CTempReg();
    ~CTempReg();
    int GetCounter();
    CSymbolTableItem* GetTempVar();
    void SetCounter(int nNum);
    void AddCounter();
    void SetTempVar(CSymbolTableItem* pTempVar);
};

class CGlobalRegAlloc
{
    CSymbolTableItem* m_pVariable;
    string m_sRegister;

public:
    CGlobalRegAlloc(CSymbolTableItem* pVariable, string sRegister);
    ~CGlobalRegAlloc();
    CSymbolTableItem* GetVarItemPtr();
    string GetRegister();
};

class CRIG
{
    vector<string> m_vVars;
    int m_aRIG[RIG_MAX_SIZE][RIG_MAX_SIZE];

    int __GetIndex(string sVarName);
    void __SetEdge(string sNode1, string sNode2);
    int __CalcDegree(string sVarName);
    void __RemoveNode(string sVarName);
    void __PushNode(string sVarName);
    void __PopNode(string sVarName);
    vector<string> __GetNeighbor(string sVarName);
    bool __isRemoved(string sVarName);
    bool __isInVector(string sTest, vector<string> vVector);
    void __GenVars(vector<CBasicBlock*> vBasicBlocks);
    void __GenRIG(vector<CBasicBlock*> vBasicBlocks);
    vector<CGlobalRegAlloc*> __RIGColoring(vector<CBasicBlock*> vBasicBlocks, vector<CSymbolTableItem*> vSymbolTable);

public:
    CRIG();
    ~CRIG();
    vector<CGlobalRegAlloc*> GenGlobalVarAlloc(vector<CBasicBlock*> vBasicBlocks, vector<CSymbolTableItem*> vSymbolTable);
};

class CCodeGeneration
{
    static string sm_sLocation;
    static bool sm_bHasTextInGlobal;
    static CTempReg sm_aTempVarPool[TEMP_VAR_POOL_SIZE];

    static void __GenGlobalCode(ofstream &fout, vector<CSymbolTableItem*> vSymbolTable, vector<CFuncIndex*> vFuncIndex);
    static void __GenSegmentAnnotation(ofstream &fout, string sSegment);
    static string __lwInto(int QuadIndex, string sRegister, string sVarName, ofstream &fout, vector<CQuadruple*> vQuads, vector<CSymbolTableItem*> vSymbolTable, vector<CFuncIndex*> vFuncIndex, vector<CGlobalRegAlloc*> vGlobalVarAlloc);
    static void __lwArrayInto(int QuadIndex, string sRegister, string sVarName, int nIndex, ofstream &fout, vector<CQuadruple*> vQuads, vector<CSymbolTableItem*> vSymbolTable, vector<CFuncIndex*> vFuncIndex, vector<CGlobalRegAlloc*> vGlobalVarAlloc);
    static void __swInto(int QuadIndex, string sRegister, string sVarName, ofstream &fout, vector<CQuadruple*> vQuads, vector<CSymbolTableItem*> vSymbolTable, vector<CFuncIndex*> vFuncIndex, vector<CGlobalRegAlloc*> vGlobalVarAlloc);
    static void __swArrayInto(int QuadIndex, string sRegister, string sVarName, int nIndex, ofstream &fout, vector<CQuadruple*> vQuads, vector<CSymbolTableItem*> vSymbolTable, vector<CFuncIndex*> vFuncIndex, vector<CGlobalRegAlloc*> vGlobalVarAlloc);
    static string __itos(int nNum);
    static void __OutputQuad(int QuadIndex, ofstream &fout, vector<CQuadruple*> vQuads);
public:
    CCodeGeneration();
    ~CCodeGeneration();
    static void GenCode(ofstream &fout, vector<vector<CBasicBlock*>> vBasicBlocks, vector<CSymbolTableItem*> vSymbolTable, vector<CFuncIndex*> vFuncIndex);
};

#else

class CCodeGeneration
{
    static string sm_sLocation;
    static bool sm_bHasTextInGlobal;

    static void __GenGlobalCode(ofstream &fout, vector<CQuadruple*> vQuads, vector<CSymbolTableItem*> vSymbolTable, vector<CFuncIndex*> vFuncIndex);
    static void __GenSegmentAnnotation(ofstream &fout, string sSegment);
    static void __lwInto(int QuadIndex, string sRegister, string sVarName, ofstream &fout, vector<CQuadruple*> vQuads, vector<CSymbolTableItem*> vSymbolTable, vector<CFuncIndex*> vFuncIndex);
    static void __lwArrayInto(int QuadIndex, string sRegister, string sVarName, int nIndex, ofstream &fout, vector<CQuadruple*> vQuads, vector<CSymbolTableItem*> vSymbolTable, vector<CFuncIndex*> vFuncIndex);
    static void __swInto(int QuadIndex, string sRegister, string sVarName, ofstream &fout, vector<CQuadruple*> vQuads, vector<CSymbolTableItem*> vSymbolTable, vector<CFuncIndex*> vFuncIndex);
    static void __swArrayInto(int QuadIndex, string sRegister, string sVarName, int nIndex, ofstream &fout, vector<CQuadruple*> vQuads, vector<CSymbolTableItem*> vSymbolTable, vector<CFuncIndex*> vFuncIndex);
    static string __itos(int nNum);
    static void __OutputQuad(int QuadIndex, ofstream &fout, vector<CQuadruple*> vQuads);
public:
    CCodeGeneration();
    ~CCodeGeneration();
    static void GenCode(ofstream &fout, vector<CQuadruple*> vQuads, vector<CSymbolTableItem*> vSymbolTable, vector<CFuncIndex*> vFuncIndex);
};

#endif

#endif