#include "BasicBlock.h"

int CBasicBlock::sm_nBlockNo = 1;

CBasicBlock::CBasicBlock(string sLabel = "")
{
    stringstream ss;
    string sTemp;
    ss << sm_nBlockNo;
    ss >> sTemp;
    m_sBlockName = "B" + sTemp;
    sm_nBlockNo++;
    m_sLabel = sLabel;
}
CBasicBlock::CBasicBlock(string sBlockName, string sLabel)
{
    m_sBlockName = sBlockName;
     m_sLabel = sLabel;
}
CBasicBlock::~CBasicBlock() {}

string CBasicBlock::GetBlockName() { return m_sBlockName; }
string CBasicBlock::GetLabel() { return m_sLabel; }
vector<CQuadruple*> CBasicBlock::GetQuads() { return m_vQuads; }
#ifdef DEBUG_OPTIMIZE
vector<string> CBasicBlock::GetInSet() { return m_vInSet; }
#endif

void CBasicBlock::AddSuccessor(CBasicBlock* pBasicBlock) { m_vSuccessor.push_back(pBasicBlock); }
void CBasicBlock::SetQuads(vector<CQuadruple*> vQuads) { m_vQuads = vQuads; }

CBasicBlock* CBasicBlock::__GenStartBlock(string sLabel) { return (new CBasicBlock("Start", sLabel)); }
CBasicBlock* CBasicBlock::__GenEndBlock() { return (new CBasicBlock("End", "")); }

void CBasicBlock::__ResetBlockNo() { sm_nBlockNo = 1; }

vector<vector<CBasicBlock*>> CBasicBlock::GenBasicBlocks(vector<CQuadruple*> vQuads)
{
    int nSize = vQuads.size();
    vector<int> vTags(nSize, BLOCK_TAG_UNDEFINED);
    for (unsigned int i=0;i<vQuads.size();i++){
        if (i==0)
            vTags[i] = BLOCK_TAG_IN;
        else if (CQuadruple::isLabel(vQuads[i]->GetOp())||CQuadruple::isFunc(vQuads[i]->GetOp())||vQuads[i]->GetOp()=="main"||vQuads[i]->GetQuadType()==QUAD_TYPE_PUSH||vQuads[i]->GetQuadType()==QUAD_TYPE_PRINTPUSH||vQuads[i]->GetQuadType()==QUAD_TYPE_POP||vQuads[i]->GetQuadType()==QUAD_TYPE_SCANPOP||vQuads[i]->GetQuadType()==QUAD_TYPE_BEGINFUNC||vQuads[i]->GetOp()=="[]="||vQuads[i]->GetOp()=="=[]")
            vTags[i] = BLOCK_TAG_IN;
        else if ((vQuads[i]->GetOp()=="j")||(vQuads[i]->GetOp()=="j=")||(vQuads[i]->GetOp()=="j!=")||(vQuads[i]->GetOp()=="j>")||(vQuads[i]->GetOp()=="j>=")||(vQuads[i]->GetOp()=="j<")||(vQuads[i]->GetOp()=="j<=")||(vQuads[i]->GetOp()=="LCall")||(vQuads[i]->GetQuadType()==QUAD_TYPE_SYSCALL)||(vQuads[i]->GetQuadType()==QUAD_TYPE_RETURN)||(vQuads[i]->GetQuadType()==QUAD_TYPE_ENDFUNC)){
            if (i<vQuads.size() - 1)
            vTags[i+1] = BLOCK_TAG_IN;
        }
    }

    //for (int i=0;i<vTags.size();i++)
        //cout << vTags[i] << ": " << vQuads[i]->TransQuad() << endl;

    vector<vector<CBasicBlock*>> vAllBasicBlocks;
    unsigned int i = 0;
    while (i<vQuads.size()){
        if (vQuads[i]->GetQuadType()==QUAD_TYPE_FUNCLABEL){
            vector<CBasicBlock*> vNewBasicBlocks;
            vNewBasicBlocks.push_back(__GenStartBlock(vQuads[i]->GetOp()));
            __ResetBlockNo();
            while(true){
                vector<CQuadruple*> vQuadsTemp;
                string sLabel = "";
                if (vQuads[i]->GetQuadType()==QUAD_TYPE_FUNCLABEL||vQuads[i]->GetQuadType()==QUAD_TYPE_LABEL)
                    sLabel = vQuads[i]->GetOp();
                vQuadsTemp.push_back(vQuads[i]);
                i++;
                while (i<vTags.size()&&vTags[i]!=BLOCK_TAG_IN){
                    vQuadsTemp.push_back(vQuads[i]);
                    i++;
                }
                CBasicBlock* pNewBasicBlock = new CBasicBlock(sLabel);
                pNewBasicBlock->SetQuads(vQuadsTemp);
#ifdef DEBUG_OPTIMIZE
                pNewBasicBlock->__GenUseDefSet();
#endif
                vNewBasicBlocks.push_back(pNewBasicBlock);
                if (vQuadsTemp[vQuadsTemp.size() - 1]->GetQuadType()==QUAD_TYPE_ENDFUNC){
                    vNewBasicBlocks.push_back(__GenEndBlock());
                    break;
                }
            }

            for (unsigned int i=1;i<vNewBasicBlocks.size() - 1;i++){
                vector<CQuadruple*> vQuads = vNewBasicBlocks[i]->GetQuads();
                string sLastQuadOp = vQuads[vQuads.size() - 1]->GetOp();
                if ((sLastQuadOp=="j")||(sLastQuadOp=="j=")||(sLastQuadOp=="j!=")||(sLastQuadOp=="j>")||(sLastQuadOp=="j>=")||(sLastQuadOp=="j<")||(sLastQuadOp=="j<=")){
                    string sLabel = vQuads[vQuads.size() - 1]->GetResult();
                    for (unsigned int j=1;j<vNewBasicBlocks.size() - 1;j++){
                        if (sLabel==vNewBasicBlocks[j]->GetLabel()){
                            vNewBasicBlocks[i]->AddSuccessor(vNewBasicBlocks[j]);
                            break;
                        }
                    }
                }
                if (sLastQuadOp!="j")
                    vNewBasicBlocks[i]->AddSuccessor(vNewBasicBlocks[i+1]);
            }
            vNewBasicBlocks[0]->AddSuccessor(vNewBasicBlocks[1]);
#ifdef DEBUG_OPTIMIZE
            bool bFlag = false;
            do{
                bFlag = __GenInOutSet(vNewBasicBlocks);
            }while(bFlag);
#endif
            vAllBasicBlocks.push_back(vNewBasicBlocks);
        }
    }

    return vAllBasicBlocks;
}

#ifdef DEBUG_OPTIMIZE
CDAG CBasicBlock::GenDAG()
{
    CDAG iDAG;
    vector<CQuadruple*> vQuads = this->GetQuads();
    for (unsigned int i=0;i<vQuads.size();i++){
        if (vQuads[i]->GetQuadType()==QUAD_TYPE_CALC){
            if (CQuadruple::isNum(vQuads[i]->GetArg1())){
                string sCalcResult;
                if (vQuads[i]->GetArg2()!=QUAD_EMPTY&&CQuadruple::isNum(vQuads[i]->GetArg2()))
                    sCalcResult = __Calc(vQuads[i]->GetArg1(), vQuads[i]->GetOp(), vQuads[i]->GetArg2());
                else if (vQuads[i]->GetArg2()==QUAD_EMPTY)
                    sCalcResult = "-" + vQuads[i]->GetArg1();
                if ((vQuads[i]->GetArg2()==QUAD_EMPTY)||(vQuads[i]->GetArg2()!=QUAD_EMPTY&&CQuadruple::isNum(vQuads[i]->GetArg2()))){
                    int nCalcResult = iDAG.FindInTable(sCalcResult);
                    if (nCalcResult==-1){
                        int nNodeNum = iDAG.GetNodeSum();
                        nCalcResult = nNodeNum;
                        CDAGNode* pNewDAGNode = new CDAGNode(nNodeNum, sCalcResult);
                        iDAG.AddNode(pNewDAGNode);
                        iDAG.AddNodeTableItem(new CDAGNodeTableItem(sCalcResult, nNodeNum));
                    }
                    int nResult = iDAG.FindInTable(vQuads[i]->GetResult());
                    if (nResult!=-1)
                        iDAG.SetNodeNumberInTable(vQuads[i]->GetResult(), nCalcResult);
                    else
                        iDAG.AddNodeTableItem(new CDAGNodeTableItem(vQuads[i]->GetResult(), nCalcResult));
                }
            }
            else{
                int nLArg = iDAG.FindInTable(vQuads[i]->GetArg1());
                if (nLArg==-1){
                    int nNodeNum = iDAG.GetNodeSum();
                    nLArg = nNodeNum;
                    CDAGNode* pNewDAGNode = new CDAGNode(nNodeNum, vQuads[i]->GetArg1());
                    iDAG.AddNode(pNewDAGNode);
                    iDAG.AddNodeTableItem(new CDAGNodeTableItem(vQuads[i]->GetArg1(), nNodeNum));
                }
                int nRArg = -1;
                if (vQuads[i]->GetArg2()!=QUAD_EMPTY){
                    nRArg = iDAG.FindInTable(vQuads[i]->GetArg2());
                    if (nRArg==-1){
                        int nNodeNum = iDAG.GetNodeSum();
                        nRArg = nNodeNum;
                        CDAGNode* pNewDAGNode = new CDAGNode(nNodeNum, vQuads[i]->GetArg2());
                        iDAG.AddNode(pNewDAGNode);
                        iDAG.AddNodeTableItem(new CDAGNodeTableItem(vQuads[i]->GetArg2(), nNodeNum));
                    }
                }
                vector<CDAGNode*> vDAG = iDAG.GetDAG();
                int nOp = -1;
                if (vQuads[i]->GetArg2()!=QUAD_EMPTY){
                    for (unsigned int j=0;j<vDAG.size();j++){
                        if (vDAG[j]->GetOp()==vQuads[i]->GetOp()&&vDAG[j]->GetLChild()!=NULL&&vDAG[j]->GetRChild()!=NULL){
                            if (vQuads[i]->GetOp()=="-"||vQuads[i]->GetOp()=="/"){
                                if (vDAG[j]->GetLChild()->GetNumber()==nLArg&&vDAG[j]->GetRChild()->GetNumber()==nRArg)
                                    nOp = vDAG[j]->GetNumber();
                                break;
                            }
                            else if (vQuads[i]->GetOp()=="+"||vQuads[i]->GetOp()=="*"){
                                if ((vDAG[j]->GetLChild()->GetNumber()==nLArg&&vDAG[j]->GetRChild()->GetNumber()==nRArg)||(vDAG[j]->GetLChild()->GetNumber()==nRArg&&vDAG[j]->GetRChild()->GetNumber()==nLArg))
                                    nOp = vDAG[j]->GetNumber();
                                break;
                            }
                        }
                    }
                    if (nOp==-1){
                        int nNodeNum = iDAG.GetNodeSum();
                        nOp = nNodeNum;
                        CDAGNode* pNewDAGNode = new CDAGNode(nNodeNum, vQuads[i]->GetOp());
                        CDAGNode* pLNode = iDAG.FindDAGNode(nLArg);
                        CDAGNode* pRNode = iDAG.FindDAGNode(nRArg);
                        pNewDAGNode->SetLChild(pLNode);
                        pNewDAGNode->SetRChild(pRNode);
                        pLNode->AddFather(pNewDAGNode);
                        pRNode->AddFather(pNewDAGNode);
                        iDAG.AddNode(pNewDAGNode);
                    }
                }
                else{
                    for (unsigned int j=0;j<vDAG.size();j++){
                        if (vDAG[j]->GetOp()==vQuads[i]->GetOp()&&vDAG[j]->GetLChild()!=NULL&&vDAG[j]->GetLChild()->GetNumber()==nLArg){
                            nOp = vDAG[j]->GetNumber();
                            break;
                        }
                    }
                    if (nOp==-1){
                        int nNodeNum = iDAG.GetNodeSum();
                        nOp = nNodeNum;
                        CDAGNode* pNewDAGNode = new CDAGNode(nNodeNum, vQuads[i]->GetOp());
                        CDAGNode* pLNode = iDAG.FindDAGNode(nLArg);
                        pNewDAGNode->SetLChild(pLNode);
                        pLNode->AddFather(pNewDAGNode);
                        iDAG.AddNode(pNewDAGNode);
                    }
                }
                int nResult = iDAG.FindInTable(vQuads[i]->GetResult());
                if (nResult!=-1)
                    iDAG.SetNodeNumberInTable(vQuads[i]->GetResult(), nOp);
                else
                    iDAG.AddNodeTableItem(new CDAGNodeTableItem(vQuads[i]->GetResult(), nOp));
            }
        }
        else if (vQuads[i]->GetQuadType()==QUAD_TYPE_ASSIGN){
            int nLArg = iDAG.FindInTable(vQuads[i]->GetArg2());
            if (nLArg==-1){
                int nNodeNum = iDAG.GetNodeSum();
                nLArg = nNodeNum;
                CDAGNode* pNewDAGNode = new CDAGNode(nNodeNum, vQuads[i]->GetArg2());
                iDAG.AddNode(pNewDAGNode);
                iDAG.AddNodeTableItem(new CDAGNodeTableItem(vQuads[i]->GetArg2(), nNodeNum));
            }
            int nResult = iDAG.FindInTable(vQuads[i]->GetArg1());
            if (nResult!=-1)
                iDAG.SetNodeNumberInTable(vQuads[i]->GetArg1(), nLArg);
            else
                iDAG.AddNodeTableItem(new CDAGNodeTableItem(vQuads[i]->GetArg1(), nLArg));
        }
    }
    return iDAG;
}

string CBasicBlock::__Calc(string sArg1, string sOp, string sArg2)
{
    int nArg1 = atoi(sArg1.c_str());
    int nArg2 = atoi(sArg2.c_str());
    int nResult = 0;
    if (sOp=="+")
        nResult = nArg1 + nArg2;
    else if (sOp=="-")
        nResult = nArg1 - nArg2;
    else if (sOp=="*")
        nResult = nArg1 * nArg2;
    else if (sOp=="/")
        nResult = nArg1 / nArg2;
    stringstream ss;
    string s;
    ss << nResult;
    ss >> s;
    return s;
}

void CBasicBlock::__GenUseDefSet()
{
    vector<CQuadruple*> vQuads = this->GetQuads();
    for (unsigned int i=0;i<vQuads.size();i++){
        if (vQuads[i]->GetQuadType()==QUAD_TYPE_CALC||vQuads[i]->GetQuadType()==QUAD_TYPE_CONDJUMP){
            if ((!CQuadruple::isNum(vQuads[i]->GetArg1()))&&(!CQuadruple::isTemp(vQuads[i]->GetArg1()))){
                if (this->m_sDefSet.find(vQuads[i]->GetArg1())==this->m_sDefSet.end())
                    this->m_sUseSet.insert(vQuads[i]->GetArg1());
            }
            if ((!CQuadruple::isNum(vQuads[i]->GetArg2()))&&(!CQuadruple::isTemp(vQuads[i]->GetArg2()))&&(vQuads[i]->GetArg2()!=QUAD_EMPTY)){
                if (this->m_sDefSet.find(vQuads[i]->GetArg2())==this->m_sDefSet.end())
                    this->m_sUseSet.insert(vQuads[i]->GetArg2());
            }
        }
        else if (vQuads[i]->GetOp()=="="){
            if ((!CQuadruple::isNum(vQuads[i]->GetArg1()))&&(!CQuadruple::isTemp(vQuads[i]->GetArg1()))){
                if (this->m_sUseSet.find(vQuads[i]->GetArg1())==this->m_sUseSet.end())
                    this->m_sDefSet.insert(vQuads[i]->GetArg1());
            }
            if ((!CQuadruple::isNum(vQuads[i]->GetArg2()))&&(!CQuadruple::isTemp(vQuads[i]->GetArg2()))&&(vQuads[i]->GetArg2().substr(0, 6)!="LCall ")&&(vQuads[i]->GetArg2().substr(0, 1)!="'")){
                if (this->m_sDefSet.find(vQuads[i]->GetArg2())==this->m_sDefSet.end())
                    this->m_sUseSet.insert(vQuads[i]->GetArg2());
            }
        }
        else if (vQuads[i]->GetOp()=="[]="||vQuads[i]->GetQuadType()==QUAD_TYPE_PUSH||vQuads[i]->GetQuadType()==QUAD_TYPE_SCANPUSH||vQuads[i]->GetQuadType()==QUAD_TYPE_RETURN){
            if ((!CQuadruple::isNum(vQuads[i]->GetArg1()))&&(!CQuadruple::isTemp(vQuads[i]->GetArg1()))&&(vQuads[i]->GetArg1()!=QUAD_EMPTY)){
                if (this->m_sDefSet.find(vQuads[i]->GetArg1())==this->m_sDefSet.end())
                    this->m_sUseSet.insert(vQuads[i]->GetArg1());
            }
        }
        else if (vQuads[i]->GetQuadType()==QUAD_TYPE_PRINTPUSH){
            if ((!CQuadruple::isNum(vQuads[i]->GetArg1()))&&(!CQuadruple::isTemp(vQuads[i]->GetArg1()))&&(!CQuadruple::isString(vQuads[i]->GetArg1()))){
                if (this->m_sDefSet.find(vQuads[i]->GetArg1())==this->m_sDefSet.end())
                    this->m_sUseSet.insert(vQuads[i]->GetArg1());
            }
        }
    }
}

bool CBasicBlock::__GenInOutSet(vector<CBasicBlock*> vBasicBlocks)
{
    bool bChanged = false;
    for (int i = vBasicBlocks.size() - 1;i>=0;i--){
        vector<string> vTemp = vBasicBlocks[i]->m_vOutSet;
        vBasicBlocks[i]->m_vOutSet.clear();
        for (unsigned int j=0;j<vBasicBlocks[i]->m_vSuccessor.size();j++){
            vector <string> vResult1;
            CBasicBlock* pSuccessor =  vBasicBlocks[i]->m_vSuccessor[j];
            set_union(pSuccessor->m_vInSet.begin(), pSuccessor->m_vInSet.end(), vBasicBlocks[i]->m_vOutSet.begin(), vBasicBlocks[i]->m_vOutSet.end(), back_inserter(vResult1));
            vBasicBlocks[i]->m_vOutSet = vResult1;
        }
        if (vTemp!=vBasicBlocks[i]->m_vOutSet)
            bChanged = true;
        vector<string> vResult1, vResult2;
        vBasicBlocks[i]->m_vInSet.clear();
        set_difference(vBasicBlocks[i]->m_vOutSet.begin(), vBasicBlocks[i]->m_vOutSet.end(), vBasicBlocks[i]->m_sDefSet.begin(), vBasicBlocks[i]->m_sDefSet.end(), back_inserter(vResult1));
        set_union(vBasicBlocks[i]->m_sUseSet.begin(), vBasicBlocks[i]->m_sUseSet.end(), vResult1.begin(), vResult1.end(), back_inserter(vResult2));
        vBasicBlocks[i]->m_vInSet = vResult2;
    }
    return bChanged;
}
#endif