#include "SymbolTable.h"

int CSymbolTableItem::sm_nStringIndex = 0;

CSymbolTableItem::CSymbolTableItem(string sName = "", Catagory eCatagory = CAT_CONST, string sType = "", int nOffset = 0, string sConstValue = "", int nArraySize = 0, int nFuncParamNum = 0, bool bLeafFunc = true)
{
    m_sName = sName;
    m_eCatagory = eCatagory;
    m_sType = sType;
    m_nOffset = nOffset;
    m_sConstValue = sConstValue;
    m_nArraySize = nArraySize;
    m_nFuncParamNum = nFuncParamNum;
    m_bLeafFunc = bLeafFunc;
}
CSymbolTableItem::~CSymbolTableItem() {}
string CSymbolTableItem::GetName() { return m_sName; }
Catagory CSymbolTableItem::GetCatagory() { return m_eCatagory; }
string CSymbolTableItem::GetType() { return m_sType; }
int CSymbolTableItem::GetOffset() { return m_nOffset; }
string CSymbolTableItem::GetConstValue() { return m_sConstValue; }
int CSymbolTableItem::GetArraySize() { return m_nArraySize; }
int CSymbolTableItem::GetFuncParamNum() { return m_nFuncParamNum; }
bool CSymbolTableItem::GetLeafFunc() { return m_bLeafFunc; }
void CSymbolTableItem::SetFuncParamNum(int nFuncParamNum) { m_nFuncParamNum = nFuncParamNum; }
void CSymbolTableItem::SetLeafFunc(bool bLeafFunc) { m_bLeafFunc = bLeafFunc; }
void CSymbolTableItem::SetOffset(int nOffset) { m_nOffset = nOffset; }

string CSymbolTableItem::GenStringIndex()
{
    string sPrefix = STRING_PREFIX;
    stringstream ss;
    string sTemp;
    ss << sm_nStringIndex;
    ss >> sTemp;
    string sStringIndex = sPrefix + sTemp;
    sm_nStringIndex++;
    return sStringIndex;
}

bool CSymbolTableItem::isString(string sTest)
{
    string sPrefix = STRING_PREFIX;
    string sTemp = sTest.substr(0, sPrefix.size());
    return (sTemp==STRING_PREFIX);
}

CFuncIndex::CFuncIndex(string sFuncName = "", int nIndex = 0) { m_sFuncName = sFuncName; m_nIndex = nIndex; }
CFuncIndex::~CFuncIndex() {}
string CFuncIndex::GetFuncName() { return m_sFuncName; }
int CFuncIndex::GetIndex() { return m_nIndex; }
void CFuncIndex::InsertTempVarTable(CSymbolTableItem* pTempVarTableItem)
{
    m_vTempVarTable.push_back(pTempVarTableItem);
}

bool CFuncIndex::isInTempVarTable(string sTempName)
{
    for (unsigned int i=0;i<m_vTempVarTable.size();i++){
        if (m_vTempVarTable[i]->GetName()==sTempName)
            return true;
    }
    return false;
}

CSymbolTableItem* CFuncIndex::GetItemPtr(string sTempName)
{
    for (unsigned int i=0;i<m_vTempVarTable.size();i++){
        if (m_vTempVarTable[i]->GetName()==sTempName)
            return m_vTempVarTable[i];
    }
    return NULL;
}

int CFuncIndex::GetOffsetInTempVarTable(string sName)
{
    for (unsigned int i=0;i<m_vTempVarTable.size();i++){
        if (m_vTempVarTable[i]->GetName()==sName){
            return m_vTempVarTable[i]->GetOffset();
        }
    }
    return -1;
}

void CFuncIndex::PlusIndex()
{
    m_nIndex++;
}

#ifdef DEBUG_SYMBOL_TABLE
void CSymbolTableItem::PrintSymbolTableItem(ofstream &fout)
{
    string sTemp = CatagoryString[m_eCatagory];
    fout << m_sName << "\t" << (m_sName.size()<8?(m_sName.size()<4?"\t\t":"\t"):"")
        << sTemp << "\t\t" << (sTemp.size()<4?"\t":"")
        << m_sType << "\t" << (m_sType.size()<4?"\t":"")
        << m_sConstValue << "\t\t\t"
        << m_nOffset << "\t\t"
        << m_nArraySize << "\t\t\t"
        << m_nFuncParamNum << "\t\t\t\t"
        << (m_bLeafFunc==true?"true":"false")
        << endl;
}
#endif