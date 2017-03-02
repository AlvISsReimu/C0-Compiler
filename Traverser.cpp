#include "Traverser.h"

vector<CQuadruple*> CTraverser::sm_vQuads;
CLabelStack CTraverser::sm_iLabelStack;
string CTraverser::sm_sLocation = "";

CTraverser::CTraverser() {}
CTraverser::~CTraverser() {}

vector<CQuadruple*> CTraverser::GetQuads() { return sm_vQuads; }

void CTraverser::ASTTraversal(CASTNode* pTreeNode)
{
    if (pTreeNode->GetNodeKind()==CONDSTMT){
        __GenCondStmtQuads(pTreeNode);
        if (pTreeNode->GetSibling()!=NULL)
            ASTTraversal(pTreeNode->GetSibling());
        return ;
    }
    else if (pTreeNode->GetNodeKind()==WHILESTMT){
        __GenWhileStmtQuads(pTreeNode);
        if (pTreeNode->GetSibling()!=NULL)
            ASTTraversal(pTreeNode->GetSibling());
        return ;
    }
    else if (pTreeNode->GetNodeKind()==FORSTMT){
        __GenForStmtQuads(pTreeNode);
        if (pTreeNode->GetSibling()!=NULL)
            ASTTraversal(pTreeNode->GetSibling());
        return ;
    }
    else if (pTreeNode->GetNodeKind()==FUNCDEF){
        __GenFuncDefQuads(pTreeNode);
        if (pTreeNode->GetSibling()!=NULL)
            ASTTraversal(pTreeNode->GetSibling());
        return ;
    }
    else if (pTreeNode->GetNodeKind()==CALLSTMT){
        __GenCallStmtQuads(pTreeNode);
        if (pTreeNode->GetSibling()!=NULL)
            ASTTraversal(pTreeNode->GetSibling());
        return ;
    }
    else if ((pTreeNode->GetNodeKind()==CONSTDEC)||(pTreeNode->GetNodeKind()==VARDEC))
        return ;

    if (pTreeNode!=NULL){
        int i = 0;
        while(i<MAX_CHILD&&pTreeNode->GetChild(i)!=NULL){
            ASTTraversal(pTreeNode->GetChild(i));
            i++;
        }
    }

    if ((pTreeNode->GetNodeKind()==PLUS)||(pTreeNode->GetNodeKind()==MINU)||(pTreeNode->GetNodeKind()==MULT)||(pTreeNode->GetNodeKind()==DIV))
        __GenCalcQuads(pTreeNode);
    else if ((pTreeNode->GetNodeKind()==EQL)||(pTreeNode->GetNodeKind()==NEQ)||(pTreeNode->GetNodeKind()==GRE)||(pTreeNode->GetNodeKind()==GEQ)||(pTreeNode->GetNodeKind()==LSS)||(pTreeNode->GetNodeKind()==LEQ))
        __GenCondQuads(pTreeNode, sm_iLabelStack.GetElement(sm_iLabelStack.GetSize() - 1));
    else if (pTreeNode->GetNodeKind()==ASSIGNSTMT)
        __GenAssignQuads(pTreeNode);
    else if (pTreeNode->GetNodeKind()==RETURNSTMT){
        __GenReturnQuads(pTreeNode);
        return ;
    }
    else if (pTreeNode->GetNodeKind()==PRINTSTMT)
        __GenPrintStmtQuads(pTreeNode);
    else if (pTreeNode->GetNodeKind()==SCANSTMT)
        __GenScanStmtQuads(pTreeNode);
    else if (pTreeNode->GetNodeKind()==IDENTIFIER&&pTreeNode->ChildNum()==1){
        string sTemp = CQuadruple::GenTemp();
        sm_vQuads.push_back(new CQuadruple("=[]", pTreeNode->GetQuadTemp(), pTreeNode->GetChild(0)->GetQuadTemp(), sTemp));
        pTreeNode->SetQuadTemp(sTemp);
    }
    /*
    else if (pTreeNode->GetNodeKind()==STRING){
        string sTemp = CQuadruple::GenTemp();
        sm_vQuads.push_back(new CQuadruple("=", sTemp, "\"" + (pTreeNode->GetQuadTemp()) + "\"", QUAD_EMPTY));
        pTreeNode->SetQuadTemp(sTemp);
    }
    */
    else if (pTreeNode->GetNodeKind()==CHARACTER){
        string sTemp = CQuadruple::GenTemp();
        sm_vQuads.push_back(new CQuadruple("=", sTemp, "'" + (pTreeNode->GetQuadTemp()) + "'", QUAD_EMPTY));
        pTreeNode->SetQuadTemp(sTemp);
    }
    /*
    else if (pTreeNode->GetNodeKind()==INTEGER){
        string sTemp = CQuadruple::GenTemp();
        sm_vQuads.push_back(new CQuadruple("=", sTemp, pTreeNode->GetQuadTemp(), QUAD_EMPTY));
        pTreeNode->SetQuadTemp(sTemp);
    }

    else if (pTreeNode->GetNodeKind()==IDENTIFIER&&pTreeNode->ChildNum()==0){
    string sTemp = CQuadruple::GenTemp();
    sm_vQuads.push_back(new CQuadruple("=", sTemp, pTreeNode->GetQuadTemp(), QUAD_EMPTY));
    pTreeNode->SetQuadTemp(sTemp);
    }
    */

    if (pTreeNode->GetSibling()!=NULL)
        ASTTraversal(pTreeNode->GetSibling());
}

#ifdef DEBUG_TEST_QUAD
void CTraverser::PrintQuads(ofstream &fout)
{
    QuadsIterator end = sm_vQuads.end();
    for (QuadsIterator i = sm_vQuads.begin(); i != end; i++)
        (*i)->PrintQuad(fout);
}
#endif

void CTraverser::__GenWhileStmtQuads(CASTNode* pTreeNode)
{
    sm_iLabelStack.Push(CQuadruple::GenLabel());
    sm_iLabelStack.Push(CQuadruple::GenLabel());
    sm_vQuads.push_back(new CQuadruple(sm_iLabelStack.GetElement(sm_iLabelStack.GetSize() - 2), QUAD_EMPTY, QUAD_EMPTY, QUAD_EMPTY));
    CASTNode* pTemp = pTreeNode->GetChild(0);
    ASTTraversal(pTreeNode->GetChild(0));
    if (!((pTemp->GetNodeKind()==EQL)||(pTemp->GetNodeKind()==NEQ)||(pTemp->GetNodeKind()==GRE)||(pTemp->GetNodeKind()==GEQ)||(pTemp->GetNodeKind()==LSS)||(pTemp->GetNodeKind()==LEQ)))
        sm_vQuads.push_back(new CQuadruple("j=", pTemp->GetQuadTemp(), "0", sm_iLabelStack.GetElement(sm_iLabelStack.GetSize() - 1)));
    ASTTraversal(pTreeNode->GetChild(1));
    sm_vQuads.push_back(new CQuadruple("j", QUAD_EMPTY, QUAD_EMPTY, sm_iLabelStack.GetElement(sm_iLabelStack.GetSize() - 2)));
    sm_vQuads.push_back(new CQuadruple(sm_iLabelStack.Pop(), QUAD_EMPTY, QUAD_EMPTY, QUAD_EMPTY));
    sm_iLabelStack.Pop();
}

void CTraverser::__GenCondStmtQuads(CASTNode* pTreeNode)
{
    sm_iLabelStack.Push(CQuadruple::GenLabel());
    if (pTreeNode->ChildNum()==3)
        sm_iLabelStack.Push(CQuadruple::GenLabel());
    CASTNode* pTemp = pTreeNode->GetChild(0);
    ASTTraversal(pTreeNode->GetChild(0));
    if (!((pTemp->GetNodeKind()==EQL)||(pTemp->GetNodeKind()==NEQ)||(pTemp->GetNodeKind()==GRE)||(pTemp->GetNodeKind()==GEQ)||(pTemp->GetNodeKind()==LSS)||(pTemp->GetNodeKind()==LEQ)))
        sm_vQuads.push_back(new CQuadruple("j=", pTemp->GetQuadTemp(), "0", sm_iLabelStack.GetElement(sm_iLabelStack.GetSize() - 1)));
    ASTTraversal(pTreeNode->GetChild(1));
    if (pTreeNode->ChildNum()==3)
        sm_vQuads.push_back(new CQuadruple("j", QUAD_EMPTY, QUAD_EMPTY, sm_iLabelStack.GetElement(sm_iLabelStack.GetSize() - 2)));
    sm_vQuads.push_back(new CQuadruple(sm_iLabelStack.Pop(), QUAD_EMPTY, QUAD_EMPTY, QUAD_EMPTY));
    if (pTreeNode->ChildNum()==3){
        ASTTraversal(pTreeNode->GetChild(2));
        sm_vQuads.push_back(new CQuadruple(sm_iLabelStack.Pop(), QUAD_EMPTY, QUAD_EMPTY, QUAD_EMPTY));
    }
}

void CTraverser::__GenForStmtQuads(CASTNode* pTreeNode)
{
    ASTTraversal(pTreeNode->GetChild(1));
    sm_vQuads.push_back(new CQuadruple("=", pTreeNode->GetChild(0)->GetQuadTemp(), pTreeNode->GetChild(1)->GetQuadTemp(), QUAD_EMPTY));
    sm_iLabelStack.Push(CQuadruple::GenLabel());
    sm_iLabelStack.Push(CQuadruple::GenLabel());
    sm_vQuads.push_back(new CQuadruple(sm_iLabelStack.GetElement(sm_iLabelStack.GetSize() - 2), QUAD_EMPTY, QUAD_EMPTY, QUAD_EMPTY));
    CASTNode* pTemp = pTreeNode->GetChild(2);
    ASTTraversal(pTreeNode->GetChild(2));
    if (!((pTemp->GetNodeKind()==EQL)||(pTemp->GetNodeKind()==NEQ)||(pTemp->GetNodeKind()==GRE)||(pTemp->GetNodeKind()==GEQ)||(pTemp->GetNodeKind()==LSS)||(pTemp->GetNodeKind()==LEQ)))
        sm_vQuads.push_back(new CQuadruple("j=", pTemp->GetQuadTemp(), "0", sm_iLabelStack.GetElement(sm_iLabelStack.GetSize() - 1)));
    ASTTraversal(pTreeNode->GetChild(5));
    string sOp = "";
    string sSibling1 = pTreeNode->GetChild(0)->GetSibling()->GetValue();
    string sSibling2 = pTreeNode->GetChild(0)->GetSibling()->GetSibling()->GetValue();
    if (pTreeNode->GetChild(3)->GetValue()=="+")
        sOp = "+";
    else
        sOp = "-";
    string sTemp = CQuadruple::GenTemp();
    sm_vQuads.push_back(new CQuadruple(sOp, sSibling2, pTreeNode->GetChild(4)->GetValue(), sTemp));
    sm_vQuads.push_back(new CQuadruple("=", sSibling1, sTemp, QUAD_EMPTY));
    CQuadruple::DropTemp(sTemp);
    sm_vQuads.push_back(new CQuadruple("j", QUAD_EMPTY, QUAD_EMPTY, sm_iLabelStack.GetElement(sm_iLabelStack.GetSize() - 2)));
    sm_vQuads.push_back(new CQuadruple(sm_iLabelStack.Pop(), QUAD_EMPTY, QUAD_EMPTY, QUAD_EMPTY));
    sm_iLabelStack.Pop();
}

void CTraverser::__GenAssignQuads(CASTNode* pTreeNode)
{
    if (pTreeNode->ChildNum() == 2){
        CQuadruple* iQuadruple = new CQuadruple("=", pTreeNode->GetChild(0)->GetQuadTemp(), pTreeNode->GetChild(1)->GetQuadTemp(), QUAD_EMPTY);
        sm_vQuads.push_back(iQuadruple);
    }
    else if (pTreeNode->ChildNum() == 3){
        CQuadruple* iQuadruple = new CQuadruple("[]=", pTreeNode->GetChild(2)->GetQuadTemp(), pTreeNode->GetChild(1)->GetQuadTemp(), pTreeNode->GetChild(0)->GetValue());
        sm_vQuads.push_back(iQuadruple);
    }
}

void CTraverser::__GenReturnQuads(CASTNode* pTreeNode)
{
    if (pTreeNode->ChildNum()!=0)
        sm_vQuads.push_back(new CQuadruple("Return", pTreeNode->GetChild(0)->GetQuadTemp(), QUAD_EMPTY, QUAD_EMPTY));
    else
        sm_vQuads.push_back(new CQuadruple("Return", QUAD_EMPTY, QUAD_EMPTY, QUAD_EMPTY));
}

void CTraverser::__GenCalcQuads(CASTNode* pTreeNode)
{
    string sTemp = CQuadruple::GenTemp();
    pTreeNode->SetQuadTemp(sTemp);
    string sOp = "";
    switch (pTreeNode->GetNodeKind()){
    case PLUS: sOp = "+"; break;
    case MINU: sOp = "-"; break;
    case MULT: sOp = "*"; break;
    case DIV: sOp = "/"; break;
    }
    string sArg1 = pTreeNode->GetChild(0)->GetQuadTemp();
    string sArg2 = QUAD_EMPTY;
    if (!((pTreeNode->GetNodeKind()==MINU)&&(pTreeNode->ChildNum()==1)))
        sArg2 = pTreeNode->GetChild(1)->GetQuadTemp();
    CQuadruple* iQuadruple = new CQuadruple(sOp, sArg1, sArg2, sTemp);
    sm_vQuads.push_back(iQuadruple);
    CQuadruple::DropTemp(sArg1);
    if (sArg2!=QUAD_EMPTY)
        CQuadruple::DropTemp(sArg2);
}

void CTraverser::__GenCondQuads(CASTNode* pTreeNode, string sLable)
{
    string sOp = "";
    CQuadruple* iQuadruple = NULL;
    NodeKind eNodeKind = pTreeNode->GetNodeKind();
    switch (eNodeKind){
    case EQL: sOp = "j!="; break;
    case NEQ: sOp = "j="; break;
    case GRE: sOp = "j<="; break;
    case GEQ: sOp = "j<"; break;
    case LSS: sOp = "j>="; break;
    case LEQ: sOp = "j>"; break;
    default:  sOp = "j="; break;
    }
    string sArg1, sArg2;
    sArg1 = pTreeNode->GetChild(0)->GetQuadTemp();
    sArg2 = pTreeNode->GetChild(1)->GetQuadTemp();
    iQuadruple = new CQuadruple(sOp, sArg1, sArg2, sLable);
    sm_vQuads.push_back(iQuadruple);
}

void CTraverser::__GenFuncDefQuads(CASTNode* pTreeNode)
{
    string sFuncName = (pTreeNode->GetChild(1)->GetValue()=="main")?(pTreeNode->GetChild(1)->GetValue()):(FUNC_LABEL_PREFIX + pTreeNode->GetChild(1)->GetValue());
    __SetLocation(pTreeNode->GetChild(1)->GetValue());
    sm_vQuads.push_back(new CQuadruple(sFuncName, QUAD_EMPTY, QUAD_EMPTY, QUAD_EMPTY));
    sm_vQuads.push_back(new CQuadruple("BeginFunc", QUAD_EMPTY, QUAD_EMPTY, QUAD_EMPTY));
    int nIndex = sm_vQuads.size() - 1;
    CQuadruple::ResetFuncTempDirty();
    ASTTraversal(pTreeNode->GetChild(3));
    int nSumFuncTempDirty = CQuadruple::SumFuncTempDirty();
    CASTNode* pConstDefNode = pTreeNode->GetChild(3)->GetChild(0)->GetChild(0);
    CASTNode* pVarDefNode = pTreeNode->GetChild(3)->GetChild(1)->GetChild(0);
    int nConstNum = 0;
    int nVarNum = 0;
    while (pConstDefNode!=NULL){
        nConstNum += __CountSiblings(pConstDefNode);
        pConstDefNode = pConstDefNode->GetSibling();
    }
    while (pVarDefNode!=NULL){
        CASTNode* pTemp = pVarDefNode->GetChild(2);
        while (pTemp!=NULL){
            if (pTemp->GetValue()=="0")
                nVarNum++;
            else
                nVarNum += atoi(pTemp->GetValue().c_str());
            pTemp = pTemp->GetSibling();
        }
        pVarDefNode = pVarDefNode->GetSibling();
    }
    stringstream ss;
    string sTemp;
    ss << ((nConstNum + nVarNum + nSumFuncTempDirty) * TYPE_SIZE);
    ss >> sTemp;
    sm_vQuads[nIndex]->SetArg1(sTemp);
    sm_vQuads.push_back(new CQuadruple("EndFunc", QUAD_EMPTY, QUAD_EMPTY, QUAD_EMPTY));
}

void CTraverser::__GenPrintStmtQuads(CASTNode* pTreeNode)
{
    if (pTreeNode->ChildNum()!=1){
        sm_vQuads.push_back(new CQuadruple("PrintPushParam", pTreeNode->GetChild(0)->GetQuadTemp(), QUAD_EMPTY, QUAD_EMPTY));
        sm_vQuads.push_back(new CQuadruple("SysCall", "_PrintString", QUAD_EMPTY, QUAD_EMPTY));
        sm_vQuads.push_back(new CQuadruple("PrintPushParam", pTreeNode->GetChild(1)->GetQuadTemp(), QUAD_EMPTY, QUAD_EMPTY));
        if (pTreeNode->GetChild(1)->GetNodeKind()==CHARACTER)
            sm_vQuads.push_back(new CQuadruple("SysCall", "_PrintChar", QUAD_EMPTY, QUAD_EMPTY));
        else if (pTreeNode->GetChild(1)->GetNodeKind()==INTEGER)
            sm_vQuads.push_back(new CQuadruple("SysCall", "_PrintInt", QUAD_EMPTY, QUAD_EMPTY));
        else if (pTreeNode->GetChild(1)->GetNodeKind()!=IDENTIFIER)
            sm_vQuads.push_back(new CQuadruple("SysCall", "_PrintInt", QUAD_EMPTY, QUAD_EMPTY));
        else{
            if (CParser::FindNameInLocal(pTreeNode->GetChild(1)->GetQuadTemp(), sm_sLocation)==-1){
                if (CParser::FindNameInGlobal(pTreeNode->GetChild(1)->GetQuadTemp())==-1)
                    sm_vQuads.push_back(new CQuadruple("SysCall", "_PrintInt", QUAD_EMPTY, QUAD_EMPTY));
                else if (CParser::GetSymbolTable()[CParser::FindNameInGlobal(pTreeNode->GetChild(1)->GetQuadTemp())]->GetType()=="char")
                    sm_vQuads.push_back(new CQuadruple("SysCall", "_PrintChar", QUAD_EMPTY, QUAD_EMPTY));
                else
                    sm_vQuads.push_back(new CQuadruple("SysCall", "_PrintInt", QUAD_EMPTY, QUAD_EMPTY));
            }
            else{
                if (CParser::GetSymbolTable()[CParser::FindNameInLocal(pTreeNode->GetChild(1)->GetQuadTemp(), sm_sLocation)]->GetType()=="char")
                    sm_vQuads.push_back(new CQuadruple("SysCall", "_PrintChar", QUAD_EMPTY, QUAD_EMPTY));
                else
                    sm_vQuads.push_back(new CQuadruple("SysCall", "_PrintInt", QUAD_EMPTY, QUAD_EMPTY));
            }
        }
    }
    else{
        sm_vQuads.push_back(new CQuadruple("PrintPushParam", pTreeNode->GetChild(0)->GetQuadTemp(), QUAD_EMPTY, QUAD_EMPTY));
        if (pTreeNode->GetChild(0)->GetNodeKind()==STRING)
            sm_vQuads.push_back(new CQuadruple("SysCall", "_PrintString", QUAD_EMPTY, QUAD_EMPTY));
        else{
            if (pTreeNode->GetChild(0)->GetNodeKind()==CHARACTER)
                sm_vQuads.push_back(new CQuadruple("SysCall", "_PrintChar", QUAD_EMPTY, QUAD_EMPTY));
            else if (pTreeNode->GetChild(0)->GetNodeKind()==INTEGER)
                sm_vQuads.push_back(new CQuadruple("SysCall", "_PrintInt", QUAD_EMPTY, QUAD_EMPTY));
            else if (pTreeNode->GetChild(0)->GetNodeKind()!=IDENTIFIER)
                sm_vQuads.push_back(new CQuadruple("SysCall", "_PrintInt", QUAD_EMPTY, QUAD_EMPTY));
            else{
                if (CParser::FindNameInLocal(pTreeNode->GetChild(0)->GetQuadTemp(), sm_sLocation)==-1){
                    if (CParser::FindNameInGlobal(pTreeNode->GetChild(0)->GetQuadTemp())==-1)
                        sm_vQuads.push_back(new CQuadruple("SysCall", "_PrintInt", QUAD_EMPTY, QUAD_EMPTY));
                    else if (CParser::GetSymbolTable()[CParser::FindNameInGlobal(pTreeNode->GetChild(0)->GetQuadTemp())]->GetType()=="char")
                        sm_vQuads.push_back(new CQuadruple("SysCall", "_PrintChar", QUAD_EMPTY, QUAD_EMPTY));
                    else
                        sm_vQuads.push_back(new CQuadruple("SysCall", "_PrintInt", QUAD_EMPTY, QUAD_EMPTY));
                }
                else{
                    if (CParser::GetSymbolTable()[CParser::FindNameInLocal(pTreeNode->GetChild(0)->GetQuadTemp(), sm_sLocation)]->GetType()=="char")
                        sm_vQuads.push_back(new CQuadruple("SysCall", "_PrintChar", QUAD_EMPTY, QUAD_EMPTY));
                    else
                        sm_vQuads.push_back(new CQuadruple("SysCall", "_PrintInt", QUAD_EMPTY, QUAD_EMPTY));
                }
            }
        }
    }
}

void CTraverser::__GenScanStmtQuads(CASTNode* pTreeNode)
{
    vector<string> vParams;
    CASTNode* pNode = pTreeNode->GetChild(0);
    while (pNode!=NULL){
        vParams.push_back(pNode->GetQuadTemp());
        pNode = pNode->GetSibling();
    }
    int nTemp = vParams.size();
    for (int i=0;i<nTemp;i++){
        //sm_vQuads.push_back(new CQuadruple("ScanPushParam", vParams[i], QUAD_EMPTY, QUAD_EMPTY));
        if (CParser::FindNameInLocal(vParams[i], sm_sLocation)==-1){
            if (CParser::GetSymbolTable()[CParser::FindNameInGlobal(vParams[i])]->GetType()=="char")
                sm_vQuads.push_back(new CQuadruple("SysCall", "_ScanChar", QUAD_EMPTY, QUAD_EMPTY));
            else
                sm_vQuads.push_back(new CQuadruple("SysCall", "_ScanInt", QUAD_EMPTY, QUAD_EMPTY));
        }
        else{
            if (CParser::GetSymbolTable()[CParser::FindNameInLocal(vParams[i], sm_sLocation)]->GetType()=="char")
                sm_vQuads.push_back(new CQuadruple("SysCall", "_ScanChar", QUAD_EMPTY, QUAD_EMPTY));
            else
                sm_vQuads.push_back(new CQuadruple("SysCall", "_ScanInt", QUAD_EMPTY, QUAD_EMPTY));
        }
        sm_vQuads.push_back(new CQuadruple("ScanPopParam", vParams[i], QUAD_EMPTY, QUAD_EMPTY));
    }
}

void CTraverser::__GenCallStmtQuads(CASTNode* pTreeNode)
{
    int nTemp = 0;
    stringstream ss;
    string s;
    if (pTreeNode->ChildNum()==2){
        ASTTraversal(pTreeNode->GetChild(1));
        vector<string> vParams;
        CASTNode* pNode = pTreeNode->GetChild(1);
        while (pNode!=NULL){
            vParams.push_back(pNode->GetQuadTemp());
            pNode = pNode->GetSibling();
        }
        nTemp = vParams.size();
        ss << (TYPE_SIZE * nTemp);
        ss >> s;
        for (int i=nTemp-1;i>=0;i--)
            sm_vQuads.push_back(new CQuadruple("PushParam", vParams[i], QUAD_EMPTY, QUAD_EMPTY));
    }
    int nIndex = CParser::FindNameInAll(pTreeNode->GetChild(0)->GetValue());
    vector<CSymbolTableItem*> vSTI = CParser::GetSymbolTable();
    if ((vSTI[nIndex])->GetType()!="void"){
        string sTemp = CQuadruple::GenTemp();
        pTreeNode->SetQuadTemp(sTemp);
        string sPrefix = FUNC_LABEL_PREFIX;
        sm_vQuads.push_back(new CQuadruple("=", sTemp, "LCall " + sPrefix + (pTreeNode->GetChild(0)->GetValue()), QUAD_EMPTY));
        pTreeNode->SetQuadTemp(sTemp);
    }
    else
        sm_vQuads.push_back(new CQuadruple("LCall", FUNC_LABEL_PREFIX + (pTreeNode->GetChild(0)->GetValue()), QUAD_EMPTY, QUAD_EMPTY));
    //if (nTemp!=0)
    sm_vQuads.push_back(new CQuadruple("PopParams", s, QUAD_EMPTY, QUAD_EMPTY));
}

int CTraverser::__CountSiblings(CASTNode* pNode)
{
    return pNode->SiblingNum(1);
}

void CTraverser::OptimizeLabels()
{
    vector<CQuadruple*>::iterator itr = sm_vQuads.begin();
    for (++itr;itr!=sm_vQuads.end();itr++){
        string sOpLatter = (*itr)->GetOp();
        string sOpFormer = (*(itr-1))->GetOp();
        if (CQuadruple::isLabel(sOpFormer)&&CQuadruple::isLabel(sOpLatter)){
            vector<CQuadruple*>::iterator itr2 = sm_vQuads.begin();
            for (;itr2!=sm_vQuads.end();itr2++){
                if ((*itr2)->GetArg1()==sOpLatter)
                    (*itr2)->SetArg1(sOpFormer);
                if ((*itr2)->GetArg2()==sOpLatter)
                    (*itr2)->SetArg2(sOpFormer);
                if ((*itr2)->GetResult()==sOpLatter)
                    (*itr2)->SetResult(sOpFormer);
            }
            itr = sm_vQuads.erase(itr);
            itr--;
        }
    }
}

string CTraverser::__GetLocation() { return sm_sLocation; }
void CTraverser::__SetLocation(string sLocation) { sm_sLocation = sLocation; }


CLabelStack::CLabelStack() { m_iStackTop = -1; }
int CLabelStack::GetSize() { return (m_iStackTop+1); }
bool CLabelStack::isStackEmpty() { return (m_iStackTop==-1); }
bool CLabelStack::isStackFull() { return (m_iStackTop==MAX_LABEL_STACK - 1); }
string CLabelStack::GetElement(int iIndex) { return m_gLabelStack[iIndex]; }
bool CLabelStack::Push(string sLabel)
{
    if (!isStackFull())
        m_gLabelStack[++m_iStackTop] = sLabel;
    else
        return false;
    return true;
}
string CLabelStack::Pop()
{
    if (!isStackEmpty()){
        string sTemp = m_gLabelStack[m_iStackTop--];
        return sTemp;
    }
    else
        return "";
}