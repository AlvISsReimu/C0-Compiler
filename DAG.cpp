#include "DAG.h"

#ifdef DEBUG_OPTIMIZE
CDAGNode::CDAGNode(int nNumber, string sOp = "")
{
    m_nNumber = nNumber;
    m_sOp = sOp;
    m_pLChild = NULL;
    m_pRChild = NULL;
}
CDAGNode::~CDAGNode() {}
void CDAGNode::SetLChild(CDAGNode* pDAGNode) { m_pLChild = pDAGNode; }
void CDAGNode::SetRChild(CDAGNode* pDAGNode) { m_pRChild = pDAGNode; }
void CDAGNode::AddFather(CDAGNode* pDAGNode) { m_vFather.push_back(pDAGNode); }
int CDAGNode::GetNumber() { return m_nNumber; }
string CDAGNode::GetOp() { return m_sOp; }
CDAGNode* CDAGNode::GetLChild() { return m_pLChild; }
CDAGNode* CDAGNode::GetRChild() { return m_pRChild; }
vector<CDAGNode*> CDAGNode::GetFather() { return m_vFather; }

CDAGNodeTableItem::CDAGNodeTableItem(string sID, int nNumber) { m_sID = sID; m_nNumber = nNumber; }
CDAGNodeTableItem::~CDAGNodeTableItem() {}
string CDAGNodeTableItem::GetID() { return m_sID; }
int CDAGNodeTableItem::GetNumber() { return m_nNumber; }
void CDAGNodeTableItem::SetNumber(int nNumber) { m_nNumber = nNumber; }

CDAG::CDAG() { m_nNodeSum = 0; }
CDAG::~CDAG() {}
int CDAG::GetNodeSum() { return (m_nNodeSum++); }
vector<CDAGNode*> CDAG::GetDAG() { return m_vDAG; }
vector<CDAGNodeTableItem*> CDAG::GetNodeTable() { return m_vNodeTable; }
void CDAG::AddNode(CDAGNode* pDAGNode) { m_vDAG.push_back(pDAGNode); }
void CDAG::AddNodeTableItem(CDAGNodeTableItem* pItem) { m_vNodeTable.push_back(pItem); }

int CDAG::FindInTable(string sName)
{
    for (unsigned int i=0;i<m_vNodeTable.size();i++){
        if (m_vNodeTable[i]->GetID()==sName)
            return m_vNodeTable[i]->GetNumber();
    }
    return -1;
}

CDAGNode* CDAG::FindDAGNode(int nNumber)
{
    for (unsigned int i=0;i<m_vDAG.size();i++){
        if (m_vDAG[i]->GetNumber()==nNumber)
            return m_vDAG[i];
    }
    return NULL;
}

void CDAG::SetNodeNumberInTable(string sName, int nNumber)
{
    for (unsigned int i=0;i<m_vNodeTable.size();i++){
        if (m_vNodeTable[i]->GetID()==sName)
            m_vNodeTable[i]->SetNumber(nNumber);
    }
}
#endif