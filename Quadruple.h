#ifndef QUADRUPLE_H
#define QUADRUPLE_H

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include "Debug.h"
#include "SymbolTable.h"

using namespace std;

#define QUAD_EMPTY "_"
#define MAX_TEMP_INDEX 102400
#define TEMP_USED 1
#define TEMP_UNUSED 0
#define TEMP_PREFIX "_tmp"
#define LABEL_PREFIX "_L"
#define FUNC_LABEL_PREFIX "_"

typedef enum QuadType{
    QUAD_TYPE_CALC, QUAD_TYPE_ASSIGN, QUAD_TYPE_JUMP, QUAD_TYPE_CONDJUMP, QUAD_TYPE_LABEL, QUAD_TYPE_FUNCLABEL,
    QUAD_TYPE_LCALL, QUAD_TYPE_SYSCALL, QUAD_TYPE_POP, QUAD_TYPE_PUSH, QUAD_TYPE_BEGINFUNC, QUAD_TYPE_ENDFUNC, QUAD_TYPE_RETURN, QUAD_TYPE_PRINTPUSH, QUAD_TYPE_SCANPUSH, QUAD_TYPE_SCANPOP
};

class CQuadruple
{
    string m_sOp;
    string m_sArg1;
    string m_sArg2;
    string m_sResult;
    QuadType m_eQuadType;
    int m_nFuncIndex;

    static int sm_iLabelIndex;
    static int sm_gTempIndex[MAX_TEMP_INDEX];
    static int sm_gFuncTempDirty[MAX_TEMP_INDEX];

public:
    CQuadruple(string sOp, string sArg1, string sArg2, string sResult);
    ~CQuadruple();
    string GetOp();
    string GetArg1();
    string GetArg2();
    string GetResult();
    QuadType GetQuadType();
    int GetFuncIndex();
    void SetArg1(string sArg1);
    void SetArg2(string sArg2);
    void SetResult(string sResult);
    void SetFuncIndex(int nFuncIndex);
    string TransQuad();

    static string GenTemp();
    static void DropTemp(string sTemp);
    static string GenLabel();
    static void ResetFuncTempDirty();
    static int SumFuncTempDirty();
    static bool isLabel(string sTest);
    static bool isFunc(string sTest);
    static bool isTemp(string sTest);
    static bool isString(string sTest);
    static bool isNum(string sTest);

#ifdef DEBUG_TEST_QUAD
    void PrintQuad(ofstream &fout);
#endif
};

#endif