#include "AST.h"

CASTNode::CASTNode(NodeKind eNodeKind = OTHER)
{
    m_eNodeKind = eNodeKind;
    for(int i=0;i<MAX_CHILD;i++)
        m_pChild[i] = NULL;
    this->SetSibling(NULL);
    this->SetValue("");
    this->SetQuadTemp("");
    this->SetLine(CLexicalAnalysis::LineNumber());
}
CASTNode::CASTNode(string sValue)
{
    m_eNodeKind = OTHER;
    for(int i=0;i<MAX_CHILD;i++)
        m_pChild[i] = NULL;
    this->SetSibling(NULL);
    this->SetValue(sValue);
    this->SetQuadTemp("");
    this->SetLine(CLexicalAnalysis::LineNumber());
#ifdef DEBUG_AST
    cout << "Create Node [27]" << endl;
#endif
}
CASTNode::~CASTNode() {}

NodeKind CASTNode::GetNodeKind() { return m_eNodeKind; }
CASTNode* CASTNode::GetChild(int iIndex) { return m_pChild[iIndex]; }
CASTNode* CASTNode::GetSibling() { return m_pSibling; }
string CASTNode::GetValue() { return m_sValue; }
string CASTNode::GetQuadTemp()
{
    if (this->m_sQuadTemp!="")
        return m_sQuadTemp;
    else
        return m_sValue;
}
int CASTNode::GetLine() { return m_iLine; }

void CASTNode::SetNodeKind(NodeKind eNodeKind) { m_eNodeKind = eNodeKind; }
void CASTNode::AddChild(CASTNode* pChild)
{
    int i=0;
    while (this->GetChild(i)!=NULL)
        i++;
    if (i<MAX_CHILD)
        m_pChild[i] = pChild;
}
void CASTNode::SetSibling(CASTNode* pSibling) { m_pSibling = pSibling; }
void CASTNode::SetValue(string sValue) { m_sValue = sValue; }
void CASTNode::SetQuadTemp(string sQuadTemp) { m_sQuadTemp = sQuadTemp; }
void CASTNode::SetLine(int iLine) { m_iLine = iLine;}

void CASTNode::InsertSiblings(CASTNode* pSibling, int iChildIndex)
    /*作用：在this的以iChildIndex为下标的Child上插入Sibling。
    效果：Child本身为NULL则直接作为Child，否则添入Sibling。*/
{
    CASTNode* q = this;
    CASTNode* p = q->GetChild(iChildIndex);
    if (p==NULL)
        q->AddChild(pSibling);
    else{
        while (p!=NULL){
            q = p;
            p = p->GetSibling();
        }
        q->SetSibling(pSibling);
    }
}

#ifdef DEBUG_TEST_TREE
void CASTNode::TestTree(CASTNode* pAST, int iLayer, ofstream &fout)
{
    if (pAST!=NULL){
        PrintTab(iLayer, fout);
        if (pAST->GetNodeKind()<OTHER)
            fout << "<" << NodeKindString[pAST->GetNodeKind()] << ">";
        else if (pAST->GetValue()!="")
            fout << "[" << pAST->GetValue() << "]";
        fout << endl;
        if (pAST->GetChild(0)!=NULL){
            PrintTab(iLayer, fout);
            fout << "Child:" << endl;
            int i = 0;
            while(i<MAX_CHILD&&pAST->GetChild(i)!=NULL){
                TestTree(pAST->GetChild(i), iLayer+1, fout);
                i++;
            }
            PrintTab(iLayer, fout);
            fout << "Child End -- " << NodeKindString[pAST->GetNodeKind()] << " " << pAST->GetValue() << endl;
            i = 0;
        }
        if (pAST->GetSibling()!=NULL){
            PrintTab(iLayer, fout);
            fout << "Sibling:" << endl;
            TestTree(pAST->GetSibling(), iLayer+1, fout);
            PrintTab(iLayer, fout);
            fout << "Sibling end -- " << NodeKindString[pAST->GetNodeKind()] << " " << pAST->GetValue() << endl;
        }
    }
}

void CASTNode::PrintTab(int iNum, ofstream &fout)
{
    for(int i=0;i<iNum;i++)
        fout << "\t";
}
#endif

int CASTNode::ChildNum()
{
    int iNum = 0;
    for (int i=0;i<MAX_CHILD;i++){
        if (this->GetChild(i)!=NULL)
            iNum++;
    }
    return iNum;
}

int CASTNode::SiblingNum(int iIndex)
{
    int iNum = 0;
    CASTNode* pTemp = this->GetChild(iIndex);
    if (pTemp!=NULL){
        iNum++;
        while (pTemp->GetSibling()!=NULL){
            pTemp = pTemp->GetSibling();
            iNum++;
        }
    }
    else
        return 0;
    return iNum;
}