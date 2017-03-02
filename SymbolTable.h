#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include "Debug.h"

using namespace std;

typedef enum Catagory{
    CAT_CONST, CAT_VAR, CAT_ARRAY, CAT_FUNC, CAT_PARAM, CAT_STRING
};

#define STRING_PREFIX "_str"

#ifdef DEBUG_SYMBOL_TABLE
const char * const CatagoryString[] = {
    "Const", "Var", "Array", "Func", "Param", "String"
};
#endif

class CSymbolTableItem
{
    string m_sName;
    Catagory m_eCatagory;
    string m_sType;
    int m_nOffset;
    string m_sConstValue;
    int m_nArraySize;
    int m_nFuncParamNum;
    bool m_bLeafFunc;

    static int sm_nStringIndex;
public:
    CSymbolTableItem(string sName, Catagory eCatagory, string sType, int nOffset, string sConstValue, int nArraySize, int nFuncParamNum, bool bLeafFunc);
    ~CSymbolTableItem();

    string GetName();
    Catagory GetCatagory();
    string GetType();
    int GetOffset();
    string GetConstValue();
    int GetArraySize();
    int GetFuncParamNum();
    bool GetLeafFunc();
    void SetFuncParamNum(int nFuncParamNum);
    void SetLeafFunc(bool bLeafFunc);
    void SetOffset(int m_nOffset);
    static string GenStringIndex();
    static bool CSymbolTableItem::isString(string sTest);

#ifdef DEBUG_SYMBOL_TABLE
    void PrintSymbolTableItem(ofstream &fout);
#endif
};

class CFuncIndex
{
    string m_sFuncName;
    int m_nIndex;
    vector<CSymbolTableItem*> m_vTempVarTable;

public:
    CFuncIndex(string sFuncName, int nIndex);
    ~CFuncIndex();
    string GetFuncName();
    int GetIndex();
    void InsertTempVarTable(CSymbolTableItem* pTempVarTableItem);
    bool isInTempVarTable(string sTempName);
    CSymbolTableItem* GetItemPtr(string sTempName);
    int GetOffsetInTempVarTable(string sName);
    void PlusIndex();
};

#endif