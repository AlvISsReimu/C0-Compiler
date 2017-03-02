#ifndef DAG_H
#define DAG_H

#include <iostream>
#include <string>
#include <vector>
#include "Quadruple.h"
#include "Debug.h"

using namespace std;

#ifdef DEBUG_OPTIMIZE
class CDAGNode
{
    int m_nNumber;
    string m_sOp;
    CDAGNode* m_pLChild;
    CDAGNode* m_pRChild;
    vector<CDAGNode*> m_vFather;

public:
    CDAGNode(int nNumber, string sOp);
    ~CDAGNode();
    void SetLChild(CDAGNode* pDAGNode);
    void SetRChild(CDAGNode* pDAGNode);
    void AddFather(CDAGNode* pDAGNode);
    int GetNumber();
    string GetOp();
    CDAGNode* GetLChild();
    CDAGNode* GetRChild();
    vector<CDAGNode*> GetFather();
};

class CDAGNodeTableItem
{
    string m_sID;
    int m_nNumber;

public:
    CDAGNodeTableItem(string sID, int nNumber);
    ~CDAGNodeTableItem();
    string GetID();
    int GetNumber();
    void SetNumber(int nNumber);
};

class CDAG
{
    int m_nNodeSum;
    vector<CDAGNode*> m_vDAG;
    vector<CDAGNodeTableItem*> m_vNodeTable;

public:
    CDAG();
    ~CDAG();
    int GetNodeSum();
    vector<CDAGNode*> GetDAG();
    vector<CDAGNodeTableItem*> GetNodeTable();
    void AddNode(CDAGNode* pDAGNode);
    void AddNodeTableItem(CDAGNodeTableItem* pItem);
    int FindInTable(string sName);
    CDAGNode* FindDAGNode(int nNumber);
    void SetNodeNumberInTable(string sName, int nNumber);
};
#endif

#endif