#include "CodeGeneration.h"

#ifdef DEBUG_OPTIMIZE

CTempReg::CTempReg(){
    SetCounter(0);
    SetTempVar(NULL);
}
CTempReg::~CTempReg() {}
int CTempReg::GetCounter() { return m_nCounter; }
CSymbolTableItem* CTempReg::GetTempVar() { return m_pTempVar; }
void CTempReg::SetCounter(int nNum) { m_nCounter = nNum; }
void CTempReg::AddCounter() { m_nCounter++; }
void CTempReg::SetTempVar(CSymbolTableItem* pTempVar) { m_pTempVar = pTempVar; }

CGlobalRegAlloc::CGlobalRegAlloc(CSymbolTableItem* pVariable, string sRegister) { m_pVariable = pVariable; m_sRegister = sRegister; }
CGlobalRegAlloc::~CGlobalRegAlloc() {}
CSymbolTableItem* CGlobalRegAlloc::GetVarItemPtr() { return m_pVariable; }
string CGlobalRegAlloc::GetRegister() { return m_sRegister; }

CRIG::CRIG()
{
    for (int i=0;i<RIG_MAX_SIZE;i++){
        for (int j=0;j<RIG_MAX_SIZE;j++)
            m_aRIG[i][j] = 0;
    }
}
CRIG::~CRIG() {}

int CRIG::__GetIndex(string sVarName)
{
    for(unsigned int i=0;i<m_vVars.size();i++){
        if (m_vVars[i]==sVarName)
            return i;
    }
    return -1;
}

bool CRIG::__isInVector(string sTest, vector<string> vVector)
{
    for (unsigned int i=0;i<vVector.size();i++){
        if (vVector[i]==sTest)
            return true;
    }
    return false;
}

void CRIG::__SetEdge(string sNode1, string sNode2)
{
    int nIndex1 = __GetIndex(sNode1);
    int nIndex2 = __GetIndex(sNode2);
    if (nIndex1 < nIndex2)
        m_aRIG[nIndex2][nIndex1] = 1;
    else
        m_aRIG[nIndex1][nIndex2] = 1;
}

int CRIG::__CalcDegree(string sVarName)
{
    int nIndex = __GetIndex(sVarName);
    int nDegree = 0;
    for (unsigned int i=0;i<m_vVars.size();i++){
        if (m_aRIG[i][nIndex]==1)
            nDegree++;
    }
    for (unsigned int i=0;i<m_vVars.size();i++){
        if (m_aRIG[nIndex][i]==1)
            nDegree++;
    }
    return nDegree;
}

void CRIG::__RemoveNode(string sVarName)
{
    int nIndex = __GetIndex(sVarName);
    for (unsigned int i=0;i<m_vVars.size();i++){
        m_aRIG[i][nIndex] = 0;
    }
    for (unsigned int i=0;i<m_vVars.size();i++){
        m_aRIG[nIndex][i] = 0;
    }
}

void CRIG::__PushNode(string sVarName)
{
    int nIndex = __GetIndex(sVarName);
    for (unsigned int i=0;i<m_vVars.size();i++){
        if (m_aRIG[i][nIndex]==1)
            m_aRIG[i][nIndex] = -1;
    }
    for (unsigned int i=0;i<m_vVars.size();i++){
        if (m_aRIG[nIndex][i]==1)
            m_aRIG[nIndex][i] = -1;
    }
}

void CRIG::__PopNode(string sVarName)
{
    int nIndex = __GetIndex(sVarName);
    for (unsigned int i=0;i<m_vVars.size();i++){
        if (m_aRIG[i][nIndex]==-1)
            m_aRIG[i][nIndex] = 1;
    }
    for (unsigned int i=0;i<m_vVars.size();i++){
        if (m_aRIG[nIndex][i]==-1)
            m_aRIG[nIndex][i] = 1;
    }
}

vector<string> CRIG::__GetNeighbor(string sVarName)
{
    vector<string> vNeighbors;
    int nIndex = __GetIndex(sVarName);
    for (unsigned int i=0;i<m_vVars.size();i++){
        if (m_aRIG[i][nIndex]!=0)
            vNeighbors.push_back(m_vVars[i]);
    }
    for (unsigned int i=0;i<m_vVars.size();i++){
        if (m_aRIG[nIndex][i]!=0)
            vNeighbors.push_back(m_vVars[i]);
    }
    return vNeighbors;
}

bool CRIG::__isRemoved(string sVarName)
{
    int nIndex = __GetIndex(sVarName);
    for (unsigned int i=0;i<m_vVars.size();i++){
        if (m_aRIG[i][nIndex]!=0)
            return false;
    }
    for (unsigned int i=0;i<m_vVars.size();i++){
        if (m_aRIG[nIndex][i]!=0)
            return false;
    }
    return true;
}

void CRIG::__GenVars(vector<CBasicBlock*> vBasicBlocks)
{
    for (unsigned int i=0;i<vBasicBlocks.size();i++){
        for (unsigned int j=0;j<vBasicBlocks[i]->GetInSet().size();j++){
            if (__GetIndex(vBasicBlocks[i]->GetInSet()[j])==-1)
                m_vVars.push_back(vBasicBlocks[i]->GetInSet()[j]);
        }
    }
}

void CRIG::__GenRIG(vector<CBasicBlock*> vBasicBlocks)
{
    for (unsigned int i=0;i<vBasicBlocks.size();i++){
        if (vBasicBlocks[i]->GetInSet().size() >= 2){
            for (unsigned int j=0;j<vBasicBlocks[i]->GetInSet().size()-1;j++){
                for (unsigned int k=j+1;k<vBasicBlocks[i]->GetInSet().size();k++)
                    __SetEdge(vBasicBlocks[i]->GetInSet()[j], vBasicBlocks[i]->GetInSet()[k]);
            }
        }
    }
}

vector<CGlobalRegAlloc*> CRIG::__RIGColoring(vector<CBasicBlock*> vBasicBlocks, vector<CSymbolTableItem*> vSymbolTable)
{
    vector<string> vTemp = m_vVars;
    vector<string> vTeam;
    while (vTemp.size()>1){
        for (unsigned int i=0;i<m_vVars.size();i++){
            if ((!__isInVector(m_vVars[i], vTeam))&&(!__isRemoved(m_vVars[i]))&&(__CalcDegree(m_vVars[i]) < GLOBAL_VAR_POOL_SIZE)){
                vTeam.push_back(m_vVars[i]);
                __PushNode(m_vVars[i]);
                for(vector<string>::iterator iter=vTemp.begin();iter!=vTemp.end();iter++){
                    if (*iter==m_vVars[i]){
                        vTemp.erase(iter);
                        break;
                    }
                }
                continue;
            }
            else if (i==m_vVars.size()-1){
                for (unsigned int j=0;j<m_vVars.size();j++){
                    if (__CalcDegree(m_vVars[j])>0){
                        __RemoveNode(m_vVars[j]);
                        for(vector<string>::iterator iter=vTemp.begin();iter!=vTemp.end();iter++){
                            if (*iter==m_vVars[j]){
                                vTemp.erase(iter);
                                break;
                            }
                        }
                        break;
                    }
                }
            }
        }
    }
    string sFuncName = vBasicBlocks[0]->GetLabel();
    if (sFuncName!="main"){
        string sPrefix = FUNC_LABEL_PREFIX;
        sFuncName = sFuncName.substr(sPrefix.length());
    }
    vector<CGlobalRegAlloc*> vGlobalRegAlloc;
    if (vTemp.size()!=0){
        int nIndex = CParser::FindNameInLocalAndGlobal(vTemp[0], sFuncName);
        CSymbolTableItem* pItem = vSymbolTable[nIndex];
        vGlobalRegAlloc.push_back(new CGlobalRegAlloc(pItem, "$s0"));
    }
    for (int i=vTeam.size()-1;i>=0;i--){
        vector<string> vNeighbors = __GetNeighbor(vTeam[i]);
        set<int> sInvalidIndex;
        for (unsigned int k=0;k<vNeighbors.size();k++){
            for (unsigned int m=0;m<vGlobalRegAlloc.size();m++){
                if (vNeighbors[k]==vGlobalRegAlloc[m]->GetVarItemPtr()->GetName()){
                    sInvalidIndex.insert(vGlobalRegAlloc[m]->GetRegister().substr(2).c_str()[0] - '0');
                    break;
                }
            }
        }
        int nRegNum = 0;
        while (sInvalidIndex.find(nRegNum)!=sInvalidIndex.end())
            nRegNum++;
        int nIndex = CParser::FindNameInLocalAndGlobal(vTeam[i], sFuncName);
        CSymbolTableItem* pItem = vSymbolTable[nIndex];
        stringstream ss;
        string sRegNum;
        ss << nRegNum;
        ss >> sRegNum;
        vGlobalRegAlloc.push_back(new CGlobalRegAlloc(pItem, "$s" + sRegNum));
    }
    return vGlobalRegAlloc;
}

vector<CGlobalRegAlloc*> CRIG::GenGlobalVarAlloc(vector<CBasicBlock*> vBasicBlocks, vector<CSymbolTableItem*> vSymbolTable)
{
    __GenVars(vBasicBlocks);
    __GenRIG(vBasicBlocks);
    return __RIGColoring(vBasicBlocks, vSymbolTable);
}

string CCodeGeneration::sm_sLocation = "";
bool CCodeGeneration::sm_bHasTextInGlobal = false;
CTempReg CCodeGeneration::sm_aTempVarPool[TEMP_VAR_POOL_SIZE];

CCodeGeneration::CCodeGeneration() {}
CCodeGeneration::~CCodeGeneration() {}

void CCodeGeneration::__GenSegmentAnnotation(ofstream &fout, string sSegment)
{
    for (int i=0;i<75;i++)
        fout << "#";
    fout << endl;
    fout << left << setw(74) << "#" << "#" << endl;
    fout << left << setw(31) << "#" << sSegment + " segment" << right << setw(32) << "#" << endl;
    fout << left << setw(74) << "#" << "#" << endl;
    for (int i=0;i<75;i++)
        fout << "#";
    fout << endl;
}

void CCodeGeneration::GenCode(ofstream &fout, vector<vector<CBasicBlock*>> vBasicBlocks, vector<CSymbolTableItem*> vSymbolTable, vector<CFuncIndex*> vFuncIndex)
{
    __GenGlobalCode(fout, vSymbolTable, vFuncIndex);

    if (!sm_bHasTextInGlobal){
        fout << endl;
        __GenSegmentAnnotation(fout, "text");
        fout << ".text" << endl;
    }
    fout << left << setw(30) << "jal main" << "# jump to main function" << endl;
    fout << left << setw(30) << "j _EOF" << "# jump to the end of the program" << endl;
    vector <CQuadruple*> vQuads;
    for (unsigned int k = 0;k<vBasicBlocks.size();k++){
        CRIG iRIG;
        vector<CGlobalRegAlloc*> vGlobalRegAlloc = iRIG.GenGlobalVarAlloc(vBasicBlocks[k], vSymbolTable);
        //for (int i=0;i<GLOBAL_VAR_POOL_SIZE;i++)
            //fout << left << setw(30) << "li $s" + __itos(i+5) + ", 0" << endl;
        for (unsigned int m = 0;m<vBasicBlocks[k].size();m++){
            for (int i=0;i<TEMP_VAR_POOL_SIZE;i++){
                sm_aTempVarPool[i].SetCounter(0);
                sm_aTempVarPool[i].SetTempVar(NULL);
            }
            vQuads = vBasicBlocks[k][m]->GetQuads();
            int nQuadNum = vQuads.size();
            for (int i=0;i<nQuadNum;i++){
                for (int j=0;j<TEMP_VAR_POOL_SIZE;j++){
                    if (sm_aTempVarPool[j].GetTempVar()!=NULL)
                        sm_aTempVarPool[j].AddCounter();
                }
                if (vQuads[i]->GetQuadType()==QUAD_TYPE_FUNCLABEL||vQuads[i]->GetQuadType()==QUAD_TYPE_LABEL){
                    if (vQuads[i]->GetOp()=="main")
                        sm_sLocation = "main";
                    else if (vQuads[i]->GetQuadType()==QUAD_TYPE_FUNCLABEL){
                        string sPrefix = FUNC_LABEL_PREFIX;
                        sm_sLocation = vQuads[i]->GetOp().substr(sPrefix.size());
                    }
                    fout << vQuads[i]->GetOp() << ":" << endl;
                }
                else if (vQuads[i]->GetQuadType()==QUAD_TYPE_BEGINFUNC){
                    __OutputQuad(i, fout, vQuads);
                    fout << left << setw(30) << "subu $sp, $sp, 8" << "# decrement sp to make space to save ra, fp" << endl;
                    fout << left << setw(30) << "sw $fp, 8($sp)" << "# save fp" << endl;
                    fout << left << setw(30) << "sw $ra, 4($sp)" << "# save ra" << endl;
                    fout << left << setw(30) << "addiu $fp, $sp, 8" << "# set up new fp" << endl;
                    fout << left << setw(30) << "subu $sp, $sp, " + vQuads[i]->GetArg1() << "# decrement sp to make space for locals/temps" << endl;
                    for (unsigned int j=0;j<vFuncIndex.size();j++){
                        if (vFuncIndex[j]->GetFuncName()==sm_sLocation){
                            int nIndex = vFuncIndex[j]->GetIndex() + 1;
                            while (nIndex<(int)(vSymbolTable.size())&&vSymbolTable[nIndex]->GetCatagory()!=CAT_FUNC){
                                if (vSymbolTable[nIndex]->GetCatagory()==CAT_CONST){
                                    string sReg = __lwInto(i, "", vSymbolTable[nIndex]->GetConstValue(), fout, vQuads, vSymbolTable, vFuncIndex, vGlobalRegAlloc);
                                    __swInto(i, sReg, vSymbolTable[nIndex]->GetName(), fout, vQuads, vSymbolTable, vFuncIndex, vGlobalRegAlloc);
                                }
                                nIndex++;
                            }
                            break;
                        }
                    }
                }
                else if (vQuads[i]->GetQuadType()==QUAD_TYPE_PUSH){
                    __OutputQuad(i, fout, vQuads);
                    fout << left << setw(30) << "subu $sp, $sp, 4" << "# decrement sp to make space for param" << endl;
                    string sReg = __lwInto(i, "", vQuads[i]->GetArg1(), fout, vQuads, vSymbolTable, vFuncIndex, vGlobalRegAlloc);
                    fout << left << setw(30) << "sw " + sReg + ", 4($sp)" << "# copy param value to stack" << endl;
                }
                else if (vQuads[i]->GetQuadType()==QUAD_TYPE_PRINTPUSH){
                    __OutputQuad(i, fout, vQuads);
                    if (CSymbolTableItem::isString(vQuads[i]->GetArg1()))
                        fout << left << setw(30) << "la $a0, " + vQuads[i]->GetArg1() << "# $a0 = &(" << vQuads[i]->GetArg1() << ")" << endl;
                    else
                        __lwInto(i, "$a0", vQuads[i]->GetArg1(), fout, vQuads, vSymbolTable, vFuncIndex, vGlobalRegAlloc);
                }
                /*
                else if (vQuads[i]->GetQuadType()==QUAD_TYPE_SCANPUSH)
                __OutputQuad(i, fout, vQuads);
                */
                else if (vQuads[i]->GetQuadType()==QUAD_TYPE_POP){
                    __OutputQuad(i, fout, vQuads);
                    if (vQuads[i]->GetArg1()!="")
                        fout << left << setw(30) << "add $sp, $sp, " + vQuads[i]->GetArg1() << "# pop params off stack" << endl;
                    for (int j=0;j<GLOBAL_VAR_POOL_SIZE;j++){
                        fout << "# clear global register $s" + __itos(j+5) + " if it holds other function's value" << endl;
                        string sLabel = CQuadruple::GenLabel();
                        fout << left << setw(30) << "bgt $s" + __itos(j+5) + ", $sp, " + sLabel << endl;
                        fout << left << setw(30) << "li $s" + __itos(j+5) + ", 0" << endl;
                        fout << left << setw(30) << sLabel + ":" << endl;
                    }
                }
                else if (vQuads[i]->GetQuadType()==QUAD_TYPE_SCANPOP){
                    __OutputQuad(i, fout, vQuads);
                    for (int j=0;j<GLOBAL_VAR_POOL_SIZE;j++){
                        fout << "# clear global register $s" + __itos(j+5) + " if it holds other function's value" << endl;
                        string sLabel = CQuadruple::GenLabel();
                        fout << left << setw(30) << "bgt $s" + __itos(j+5) + ", $sp, " + sLabel << endl;
                        fout << left << setw(30) << "li $s" + __itos(j+5) + ", 0" << endl;
                        fout << left << setw(30) << sLabel + ":" << endl;
                    }
                    __swInto(i, "$v0", vQuads[i]->GetArg1(), fout, vQuads, vSymbolTable, vFuncIndex, vGlobalRegAlloc);
                }
                else if (vQuads[i]->GetQuadType()==QUAD_TYPE_ASSIGN){
                    __OutputQuad(i, fout, vQuads);
                    if (vQuads[i]->GetOp()=="="){
                        if ((vQuads[i]->GetArg2().substr(0, 6)=="LCall ")){
                            fout << left << setw(30) << "jal " + vQuads[i]->GetArg2().substr(6) << "# call function " << vQuads[i]->GetArg2().substr(6) << endl;
                            for (int j=0;j<GLOBAL_VAR_POOL_SIZE;j++){
                                string sLabel = CQuadruple::GenLabel();
                                fout << "# clear global register $s" + __itos(j+5) + " if it holds other function's value" << endl;
                                fout << left << setw(30) << "bgt $s" + __itos(j+5) + ", $sp, " + sLabel << endl;
                                fout << left << setw(30) << "li $s" + __itos(j+5) + ", 0" << endl;
                                fout << left << setw(30) << sLabel + ":" << endl;
                            }
                            __swInto(i, "$v0", vQuads[i]->GetArg1(), fout, vQuads, vSymbolTable, vFuncIndex, vGlobalRegAlloc);
                        }
                        else{
                            string sReg = __lwInto(i, "", vQuads[i]->GetArg2(), fout, vQuads, vSymbolTable, vFuncIndex, vGlobalRegAlloc);
                            __swInto(i, sReg, vQuads[i]->GetArg1(), fout, vQuads, vSymbolTable, vFuncIndex, vGlobalRegAlloc);
                        }
                    }
                    else if (vQuads[i]->GetOp()=="[]="){
                        string sReg = __lwInto(i, "$t8", vQuads[i]->GetArg1(), fout, vQuads, vSymbolTable, vFuncIndex, vGlobalRegAlloc);
                        __swArrayInto(i, "$t8", vQuads[i]->GetResult(), atoi(vQuads[i]->GetArg2().c_str()), fout, vQuads, vSymbolTable, vFuncIndex, vGlobalRegAlloc);
                    }
                    else if (vQuads[i]->GetOp()=="=[]"){
                        __lwArrayInto(i, "$t" + __itos(TEMP_VAR_POOL_SIZE), vQuads[i]->GetArg1(), atoi(vQuads[i]->GetArg2().c_str()), fout, vQuads, vSymbolTable, vFuncIndex, vGlobalRegAlloc);
                        __swInto(i, "$t" + __itos(TEMP_VAR_POOL_SIZE), vQuads[i]->GetResult(), fout, vQuads, vSymbolTable, vFuncIndex, vGlobalRegAlloc);
                    }
                }
                else if (vQuads[i]->GetQuadType()==QUAD_TYPE_CALC){
                    __OutputQuad(i, fout, vQuads);
                    string sReg1 = __lwInto(i, "$t8", vQuads[i]->GetArg1(), fout, vQuads, vSymbolTable, vFuncIndex, vGlobalRegAlloc);
                    string sReg2 = "";
                    if (vQuads[i]->GetArg2()!=QUAD_EMPTY)
                        sReg2 = __lwInto(i, "$t9", vQuads[i]->GetArg2(), fout, vQuads, vSymbolTable, vFuncIndex, vGlobalRegAlloc);
                    if (vQuads[i]->GetOp()=="+")
                        fout << left << setw(30) << "add $t" + __itos(TEMP_VAR_POOL_SIZE) + ", $t8, $t9" << "# $t" << __itos(TEMP_VAR_POOL_SIZE) << " = " << "$t8" << " + " << "$t9" << endl;
                    else if (vQuads[i]->GetOp()=="*")
                        fout << left << setw(30) << "mul $t" + __itos(TEMP_VAR_POOL_SIZE) + ", $t8, $t9" << "# $t" << __itos(TEMP_VAR_POOL_SIZE) << " = " << "$t8" << " * " << "$t9" << endl;
                    else if (vQuads[i]->GetOp()=="/")
                        fout << left << setw(30) << "div $t" + __itos(TEMP_VAR_POOL_SIZE) + ", $t8, $t9" << "# $t" << __itos(TEMP_VAR_POOL_SIZE) << " = " << "$t8" << " / " << "$t9" << endl;
                    else if (vQuads[i]->GetOp()=="-"&&vQuads[i]->GetArg2()!=QUAD_EMPTY)
                        fout << left << setw(30) << "sub $t" + __itos(TEMP_VAR_POOL_SIZE) + ", $t8, $t9" << "# $t" << __itos(TEMP_VAR_POOL_SIZE) << " = " << "$t8" << " - " << "$t9" << endl;
                    else if (vQuads[i]->GetOp()=="-"&&vQuads[i]->GetArg2()==QUAD_EMPTY)
                        fout << left << setw(30) << "sub $t" + __itos(TEMP_VAR_POOL_SIZE) + ", $zero, $t8" << "# $t" << __itos(TEMP_VAR_POOL_SIZE) << " = 0 - " << "$t8" << endl;
                    __swInto(i, "$t" + __itos(TEMP_VAR_POOL_SIZE), vQuads[i]->GetResult(), fout, vQuads, vSymbolTable, vFuncIndex, vGlobalRegAlloc);
                }
                else if (vQuads[i]->GetQuadType()==QUAD_TYPE_CONDJUMP){
                    __OutputQuad(i, fout, vQuads);
                    string sReg1 = __lwInto(i, "$t8", vQuads[i]->GetArg1(), fout, vQuads, vSymbolTable, vFuncIndex, vGlobalRegAlloc);
                    string sReg2 = __lwInto(i, "$t9", vQuads[i]->GetArg2(), fout, vQuads, vSymbolTable, vFuncIndex, vGlobalRegAlloc);
                    if (vQuads[i]->GetOp()=="j=")
                        fout << left << setw(30) << "beq $t8, $t9, " + vQuads[i]->GetResult() << "# if " << "$t8" << " == " << "$t9" << " branch" << endl;
                    else if (vQuads[i]->GetOp()=="j!=")
                        fout << left << setw(30) << "bne $t8, $t9, " + vQuads[i]->GetResult() << "# if " << "$t8" << " != " << "$t9" << " branch" << endl;
                    else if (vQuads[i]->GetOp()=="j>")
                        fout << left << setw(30) << "bgt $t8, $t9, " + vQuads[i]->GetResult() << "# if " << "$t8" << " > " << "$t9" << " branch" << endl;
                    else if (vQuads[i]->GetOp()=="j>=")
                        fout << left << setw(30) << "bge $t8, $t9, " + vQuads[i]->GetResult() << "# if " << "$t8" << " >= " << "$t9" << " branch" << endl;
                    else if (vQuads[i]->GetOp()=="j<")
                        fout << left << setw(30) << "blt $t8, $t9, " + vQuads[i]->GetResult() << "# if " << "$t8" << " < " << "$t9" << " branch" << endl;
                    else if (vQuads[i]->GetOp()=="j<=")
                        fout << left << setw(30) << "ble $t8, $t9, " + vQuads[i]->GetResult() << "# if " << "$t8" << " <= " << "$t9" << " branch" << endl;
                }
                else if (vQuads[i]->GetQuadType()==QUAD_TYPE_JUMP){
                    __OutputQuad(i, fout, vQuads);
                    fout << "j " << vQuads[i]->GetResult() << endl;
                }
                else if (vQuads[i]->GetQuadType()==QUAD_TYPE_RETURN){
                    __OutputQuad(i, fout, vQuads);
                    if (vQuads[i]->GetArg1()!=QUAD_EMPTY){
                        string sReg = __lwInto(i, "", vQuads[i]->GetArg1(), fout, vQuads, vSymbolTable, vFuncIndex, vGlobalRegAlloc);
                        fout << left << setw(30) << "move $v0, " + sReg << "# assign return value into $v0" << endl;
                    }
                    fout << left << setw(30) << "move $sp, $fp" << "# pop callee frame off stack" << endl;
                    fout << left << setw(30) << "lw $ra, -4($fp)" << "# restore saved ra" << endl;
                    fout << left << setw(30) << "lw $fp, 0($fp)" << "# restore saved fp" << endl;
                    fout << left << setw(30) << "jr $ra" << "# return from function" << endl;
                }
                else if (vQuads[i]->GetQuadType()==QUAD_TYPE_ENDFUNC){
                    __OutputQuad(i, fout, vQuads);
                    fout << "# (below handles reaching end of function body with no explicit return)" << endl;
                    fout << left << setw(30) << "move $sp, $fp" << "# pop callee frame off stack" << endl;
                    fout << left << setw(30) << "lw $ra, -4($fp)" << "# restore saved ra" << endl;
                    fout << left << setw(30) << "lw $fp, 0($fp)" << "# restore saved fp" << endl;
                    fout << left << setw(30) << "jr $ra" << "# return from function" << endl;
                }
                else if (vQuads[i]->GetQuadType()==QUAD_TYPE_LCALL){
                    __OutputQuad(i, fout, vQuads);
                    fout << "jal " << vQuads[i]->GetArg1() << endl;
                }
                else if (vQuads[i]->GetQuadType()==QUAD_TYPE_SYSCALL){
                    __OutputQuad(i, fout, vQuads);
                    if (vQuads[i]->GetArg1()=="_PrintInt")
                        fout << left << setw(30) << "li $v0, 1" << "# $v0 = 1 for syscall \"PrintInt\"" << endl;
                    else if (vQuads[i]->GetArg1()=="_PrintString")
                        fout << left << setw(30) << "li $v0, 4" << "# $v0 = 4 for syscall \"PrintString\"" << endl;
                    else if (vQuads[i]->GetArg1()=="_PrintChar")
                        fout << left << setw(30) << "li $v0, 11" << "# $v0 = 11 for syscall \"PrintChar\"" << endl;
                    else if (vQuads[i]->GetArg1()=="_ScanInt")
                        fout << left << setw(30) << "li $v0, 5" << "# $v0 = 5 for syscall \"ScanInt\"" << endl;
                    else if (vQuads[i]->GetArg1()=="_ScanChar")
                        fout << left << setw(30) << "li $v0, 12" << "# $v0 = 12 for syscall \"ScanChar\"" << endl;
                    fout << left << setw(30) << "syscall" << endl;
                }
            }
        }
    }
    fout << "_EOF: " << endl;
    fout << left << setw(30) << "li $v0, 10" << "# $v0 = 10 for syscall \"Exit\"" << endl;
    fout << "syscall" << endl;
    fout << "## end of file" << endl;
}

void CCodeGeneration::__GenGlobalCode(ofstream &fout, vector<CSymbolTableItem*> vSymbolTable, vector<CFuncIndex*> vFuncIndex)
{
    int nGlobalConstNum = 0, nGlobalVarNum = 0, nStringNum = 0;
    for (int i=0;i<vFuncIndex[0]->GetIndex();i++){
        if (vSymbolTable[i]->GetCatagory()==CAT_CONST)
            nGlobalConstNum++;
        else if (vSymbolTable[i]->GetCatagory()==CAT_VAR||vSymbolTable[i]->GetCatagory()==CAT_ARRAY)
            nGlobalVarNum++;
        else
            nStringNum++;
    }
    if (nGlobalConstNum + nStringNum!=0){
        __GenSegmentAnnotation(fout, "data");
        fout << ".data" << endl;
    }
    for (int i=0;i<nGlobalConstNum;i++){
        fout << left << setw(15) << vSymbolTable[i]->GetName() + ":" << left << setw(15) << ".word";
        if (CQuadruple::isNum(vSymbolTable[i]->GetConstValue()))
            fout << vSymbolTable[i]->GetConstValue();
        else
            fout << (int)*(vSymbolTable[i]->GetConstValue().c_str() + 1);
        fout << endl;
    }
    for (int i=nGlobalConstNum + nGlobalVarNum;i<nGlobalConstNum + nGlobalVarNum + nStringNum;i++){
        fout << left << setw(15) << vSymbolTable[i]->GetName() + ":" << left << setw(15) << ".asciiz";
        fout << "\"" << vSymbolTable[i]->GetConstValue() << "\"" << endl;
    }
    if (nGlobalVarNum!=0){
        fout << endl;
        __GenSegmentAnnotation(fout, "text");
        fout << ".text" << endl;
        sm_bHasTextInGlobal = true;
    }
    int nOffset = 0;
    for (int i=0;i<nGlobalVarNum;i++){
        vSymbolTable[i + nGlobalConstNum]->SetOffset(nOffset);
        if (vSymbolTable[i + nGlobalConstNum]->GetCatagory()==CAT_VAR)
            nOffset += 4;
        else if (vSymbolTable[i + nGlobalConstNum]->GetCatagory()==CAT_ARRAY)
            nOffset += (vSymbolTable[i + nGlobalConstNum]->GetArraySize() * 4);
    }
    if (nGlobalVarNum!=0)
        fout << "subu $gp, $gp, " << nOffset << endl;
}

string CCodeGeneration::__lwInto(int QuadIndex, string sRegister, string sVarName, ofstream &fout, vector<CQuadruple*> vQuads, vector<CSymbolTableItem*> vSymbolTable, vector<CFuncIndex*> vFuncIndex, vector<CGlobalRegAlloc*> vGlobalVarAlloc)
{
    int nOffset, nRegister = -1;
    if (CQuadruple::isTemp(sVarName)){
        for (int i=0;i<TEMP_VAR_POOL_SIZE;i++){
            if (sm_aTempVarPool[i].GetTempVar()!=NULL&&sm_aTempVarPool[i].GetTempVar()->GetName()==sVarName){
                sm_aTempVarPool[i].SetCounter(0);
                nRegister = i;
                break;
            }
        }
        if (nRegister == -1){
            for (int i=0;i<TEMP_VAR_POOL_SIZE;i++){
                if (sm_aTempVarPool[i].GetTempVar()==NULL){
                    nOffset = vFuncIndex[vQuads[QuadIndex]->GetFuncIndex()]->GetOffsetInTempVarTable(sVarName);
                    fout << left << setw(30) << "lw $t" + __itos(i) + ", " + __itos(nOffset) + "($fp)" << "# $t" + __itos(i) << " = " << sVarName << endl;
                    nRegister = i;
                    sm_aTempVarPool[i].SetTempVar(vFuncIndex[vQuads[QuadIndex]->GetFuncIndex()]->GetItemPtr(sVarName));
                    sm_aTempVarPool[i].SetCounter(0);
                    break;
                }
            }
        }
        if (nRegister == -1){
            int nMax = 0;
            for (int i=0;i<TEMP_VAR_POOL_SIZE;i++){
                if (sm_aTempVarPool[i].GetCounter() > nMax){
                    nMax = sm_aTempVarPool[i].GetCounter();
                    nRegister = i;
                }
            }
            fout << left << setw(30) << "sw $t" + __itos(nRegister) + ", " + __itos(sm_aTempVarPool[nRegister].GetTempVar()->GetOffset()) + "($fp)"
                << "# " + sm_aTempVarPool[nRegister].GetTempVar()->GetName() + " = $t" + __itos(nRegister) << endl;
            nOffset = vFuncIndex[vQuads[QuadIndex]->GetFuncIndex()]->GetOffsetInTempVarTable(sVarName);
            fout << left << setw(30) << "lw $t" + __itos(nRegister) + ", " + __itos(nOffset) + "($fp)" << "# $t" + __itos(nRegister) << " = " << sVarName << endl;
            sm_aTempVarPool[nRegister].SetTempVar(vFuncIndex[vQuads[QuadIndex]->GetFuncIndex()]->GetItemPtr(sVarName));
            sm_aTempVarPool[nRegister].SetCounter(0);
        }
        if (sRegister!="")
            fout << "\n" << left << setw(30) << "move " + sRegister + ", $t" + __itos(nRegister) << "# " + sRegister + " = $t" + __itos(nRegister) << endl;
        return ("$t" + __itos(nRegister));
    }
    else{
        int nSymbolTableIndex = CParser::FindNameInLocal(sVarName, vFuncIndex[vQuads[QuadIndex]->GetFuncIndex()]->GetFuncName());
        if (nSymbolTableIndex!=-1){
            int nOffset = vSymbolTable[nSymbolTableIndex]->GetOffset();
            string sGlobalRegister = "";
            for (unsigned int i=0;i<vGlobalVarAlloc.size();i++){
                if (vGlobalVarAlloc[i]->GetVarItemPtr()==vSymbolTable[nSymbolTableIndex]){
                    sGlobalRegister = vGlobalVarAlloc[i]->GetRegister();
                    break;
                }
            }
            if (sGlobalRegister==""){
                nRegister = GLOBAL_VAR_POOL_SIZE;
                fout << left << setw(30) << "lw $s" + __itos(GLOBAL_VAR_POOL_SIZE) + ", " + __itos(nOffset) + "($fp)";
            }
            else{
                nRegister = sGlobalRegister.substr(2).c_str()[0] - '0';
                string sCalcRegister = "$t" + __itos(TEMP_VAR_POOL_SIZE + 2);
                string sAssistRegister = "$s" + __itos(nRegister + 5);
                string sLabel = CQuadruple::GenLabel();
                string sLabel2 = CQuadruple::GenLabel();
                fout << left << setw(30) << "addi " + sCalcRegister + ", $fp, " + __itos(nOffset) << endl;
                fout << left << setw(30) << "beq " + sAssistRegister + ", " + sCalcRegister + ", " + sLabel << endl;
                fout << left << setw(30) << "beq " + sAssistRegister + ", $zero, " + sLabel2 << endl;
                fout << left << setw(30) << "sw " + sGlobalRegister + ", 0(" + sAssistRegister + ")" << endl;
                fout << sLabel2 + ":" << endl;
                fout << left << setw(30) << "lw " + sGlobalRegister + ", " + __itos(nOffset) + "($fp)" << endl;
                fout << left << setw(30) << "move " + sAssistRegister + ", " + sCalcRegister << endl;
                fout << sLabel + ":";
            }
        }
        else{
            nSymbolTableIndex = CParser::FindNameInGlobal(sVarName);
            if (nSymbolTableIndex!=-1){
                if (vSymbolTable[nSymbolTableIndex]->GetCatagory()==CAT_CONST){
                    nRegister = GLOBAL_VAR_POOL_SIZE;
                    fout << left << setw(30) << "lw $s" + __itos(GLOBAL_VAR_POOL_SIZE) + ", " + sVarName;
                }
                else if (vSymbolTable[nSymbolTableIndex]->GetCatagory()==CAT_VAR){
                    nOffset = vSymbolTable[nSymbolTableIndex]->GetOffset();
                    nRegister = GLOBAL_VAR_POOL_SIZE;
                    fout << left << setw(30) << "lw $s" + __itos(GLOBAL_VAR_POOL_SIZE) + ", " + __itos(nOffset) + "($gp)";
                }
            }
            else{
                nRegister = GLOBAL_VAR_POOL_SIZE;
                fout << left << setw(30) << "li $s" + __itos(GLOBAL_VAR_POOL_SIZE) + ", " + sVarName;
            }
        }
        if (sRegister!="")
            fout << endl << left << setw(30) << "move " + sRegister + ", $s" + __itos(nRegister);
        fout  << "# $s" + __itos(nRegister) << " = " << sVarName << endl;
        return ("$s" + __itos(nRegister));
    }
}

void CCodeGeneration::__lwArrayInto(int QuadIndex, string sRegister, string sVarName, int nIndex, ofstream &fout, vector<CQuadruple*> vQuads, vector<CSymbolTableItem*> vSymbolTable, vector<CFuncIndex*> vFuncIndex, vector<CGlobalRegAlloc*> vGlobalVarAlloc)
{
    int nOffset;
    int nSymbolTableIndex = CParser::FindNameInLocal(sVarName, vFuncIndex[vQuads[QuadIndex]->GetFuncIndex()]->GetFuncName());
    if (nSymbolTableIndex!=-1){
        nOffset = vSymbolTable[nSymbolTableIndex]->GetOffset();
        string sReg = __lwInto(QuadIndex, "", vQuads[QuadIndex]->GetArg2(), fout, vQuads, vSymbolTable, vFuncIndex, vGlobalVarAlloc);
        fout << "mul $t" << TEMP_VAR_POOL_SIZE << ", " << sReg << ", 4" << endl;
        fout << "subu $t" << (TEMP_VAR_POOL_SIZE + 1) << ", $fp, $t" << TEMP_VAR_POOL_SIZE << endl;
        fout << left << setw(30) << "lw " + sRegister + ", " + __itos(nOffset) + "($t" + __itos((TEMP_VAR_POOL_SIZE + 1)) + ")";
    }
    else{
        nSymbolTableIndex = CParser::FindNameInGlobal(sVarName);
        if (nSymbolTableIndex!=-1){
            nOffset = vSymbolTable[nSymbolTableIndex]->GetOffset();
            string sReg = __lwInto(QuadIndex, "", vQuads[QuadIndex]->GetArg2(), fout, vQuads, vSymbolTable, vFuncIndex, vGlobalVarAlloc);
            fout << "mul $t" << TEMP_VAR_POOL_SIZE << ", " << sReg << ", 4" << endl;
            fout << "addu $t" << (TEMP_VAR_POOL_SIZE + 1) << ", $gp, $t" << TEMP_VAR_POOL_SIZE << endl;
            fout << left << setw(30) << "lw " + sRegister + ", " + __itos(nOffset) + "($t" + __itos((TEMP_VAR_POOL_SIZE + 1)) + ")";
        }
    }
    fout << "# " << sRegister << " = " << sVarName << "[" << vQuads[QuadIndex]->GetArg2() << "]" << endl;
}

void CCodeGeneration::__swInto(int QuadIndex, string sRegister, string sVarName, ofstream &fout, vector<CQuadruple*> vQuads, vector<CSymbolTableItem*> vSymbolTable, vector<CFuncIndex*> vFuncIndex, vector<CGlobalRegAlloc*> vGlobalVarAlloc)
{
    int nOffset, nRegister = -1;
    if (CQuadruple::isTemp(sVarName)){
        for (int i=0;i<TEMP_VAR_POOL_SIZE;i++){
            if (sm_aTempVarPool[i].GetTempVar()!=NULL&&sm_aTempVarPool[i].GetTempVar()->GetName()==sVarName){
                sm_aTempVarPool[i].SetCounter(0);
                nRegister = i;
                fout << left << setw(30) << "move $t" + __itos(i) + ", " + sRegister << endl;
                nOffset = vFuncIndex[vQuads[QuadIndex]->GetFuncIndex()]->GetOffsetInTempVarTable(sVarName);
                fout << left << setw(30) << "sw " + sRegister + ", " + __itos(nOffset) + "($fp)";
                break;
            }
        }
        if (nRegister==-1){
            nOffset = vFuncIndex[vQuads[QuadIndex]->GetFuncIndex()]->GetOffsetInTempVarTable(sVarName);
            fout << left << setw(30) << "sw " + sRegister + ", " + __itos(nOffset) + "($fp)";
        }
    }
    else{
        int nSymbolTableIndex = CParser::FindNameInLocal(sVarName, vFuncIndex[vQuads[QuadIndex]->GetFuncIndex()]->GetFuncName());
        if (nSymbolTableIndex!=-1){
            int nOffset = vSymbolTable[nSymbolTableIndex]->GetOffset();
            string sGlobalRegister = "";
            for (unsigned int i=0;i<vGlobalVarAlloc.size();i++){
                if (vGlobalVarAlloc[i]->GetVarItemPtr()==vSymbolTable[nSymbolTableIndex]){
                    sGlobalRegister = vGlobalVarAlloc[i]->GetRegister();
                    break;
                }
            }
            if (sGlobalRegister=="")
                fout << left << setw(30) << "sw " + sRegister + ", " + __itos(nOffset) + "($fp)";
            else{
                string sCalcRegister = "$t" + __itos(TEMP_VAR_POOL_SIZE + 2);
                string sAssistRegister = "$s" + __itos(sGlobalRegister.substr(2).c_str()[0] - '0' + 5);
                string sLabel = CQuadruple::GenLabel();
                string sLabel2 = CQuadruple::GenLabel();
                fout << left << setw(30) << "move $t9, " + sRegister << endl;
                fout << left << setw(30) << "addi " + sCalcRegister + ", $fp, " + __itos(nOffset) << endl;
                fout << left << setw(30) << "beq " + sAssistRegister + ", " + sCalcRegister + ", " + sLabel << endl;
                fout << left << setw(30) << "beq " + sAssistRegister + ", $zero, " + sLabel2 << endl;
                fout << left << setw(30) << "sw " + sGlobalRegister + ", 0(" + sAssistRegister + ")" << endl;
                fout << sLabel2 + ":" << endl;
                fout << left << setw(30) << "lw " + sGlobalRegister + ", " + __itos(nOffset) + "($fp)" << endl;
                fout << left << setw(30) << "move " + sAssistRegister + ", " + sCalcRegister << endl;
                fout << sLabel + ":" << endl;
                fout << left << setw(30) << "move " + sGlobalRegister + ", $t9";
            }
        }
        else{
            nSymbolTableIndex = CParser::FindNameInGlobal(sVarName);
            if (nSymbolTableIndex!=-1){
                nOffset = vSymbolTable[nSymbolTableIndex]->GetOffset();
                fout << left << setw(30) << "sw " + sRegister + ", " + __itos(nOffset) + "($gp)";
            }
        }
    }
    fout << "# " << sVarName << " = " << sRegister << endl;
}

void CCodeGeneration::__swArrayInto(int QuadIndex, string sRegister, string sVarName, int nIndex, ofstream &fout, vector<CQuadruple*> vQuads, vector<CSymbolTableItem*> vSymbolTable, vector<CFuncIndex*> vFuncIndex, vector<CGlobalRegAlloc*> vGlobalVarAlloc)
{
    int nOffset;
    int nSymbolTableIndex = CParser::FindNameInLocal(sVarName, vFuncIndex[vQuads[QuadIndex]->GetFuncIndex()]->GetFuncName());
    if (nSymbolTableIndex!=-1){
        nOffset = vSymbolTable[nSymbolTableIndex]->GetOffset();
        string sReg = __lwInto(QuadIndex, "", vQuads[QuadIndex]->GetArg2(), fout, vQuads, vSymbolTable, vFuncIndex, vGlobalVarAlloc);
        fout << "mul $t" << TEMP_VAR_POOL_SIZE << ", " << sReg << ", 4" << endl;
        fout << "subu $t" << (TEMP_VAR_POOL_SIZE + 1) << ", $fp, $t" << TEMP_VAR_POOL_SIZE << endl;
        fout << left << setw(30) << "sw " + sRegister + ", " + __itos(nOffset) + "($t" + __itos((TEMP_VAR_POOL_SIZE + 1)) + ")";
    }
    else{
        nSymbolTableIndex = CParser::FindNameInGlobal(sVarName);
        if (nSymbolTableIndex!=-1){
            nOffset = vSymbolTable[nSymbolTableIndex]->GetOffset();
            string sReg = __lwInto(QuadIndex, "", vQuads[QuadIndex]->GetArg2(), fout, vQuads, vSymbolTable, vFuncIndex, vGlobalVarAlloc);
            fout << "mul $t" << TEMP_VAR_POOL_SIZE << ", " << sReg << ", 4" << endl;
            fout << "addu $t" << (TEMP_VAR_POOL_SIZE + 1) << ", $gp, $t" << TEMP_VAR_POOL_SIZE << endl;
            fout << left << setw(30) << "sw " + sRegister + ", " + __itos(nOffset) + "($t" + __itos((TEMP_VAR_POOL_SIZE + 1)) + ")";
        }
    }
    fout << "# " << sVarName << "[" << vQuads[QuadIndex]->GetArg2() << "] = " << sRegister << endl;
}

#else

string CCodeGeneration::sm_sLocation = "";
bool CCodeGeneration::sm_bHasTextInGlobal = false;

CCodeGeneration::CCodeGeneration() {}
CCodeGeneration::~CCodeGeneration() {}

void CCodeGeneration::__GenSegmentAnnotation(ofstream &fout, string sSegment)
{
    for (int i=0;i<75;i++)
        fout << "#";
    fout << endl;
    fout << left << setw(74) << "#" << "#" << endl;
    fout << left << setw(31) << "#" << sSegment + " segment" << right << setw(32) << "#" << endl;
    fout << left << setw(74) << "#" << "#" << endl;
    for (int i=0;i<75;i++)
        fout << "#";
    fout << endl;
}

void CCodeGeneration::GenCode(ofstream &fout, vector<CQuadruple*> vQuads, vector<CSymbolTableItem*> vSymbolTable, vector<CFuncIndex*> vFuncIndex)
{
    __GenGlobalCode(fout, vQuads, vSymbolTable, vFuncIndex);

    if (!sm_bHasTextInGlobal){
        fout << endl;
        __GenSegmentAnnotation(fout, "text");
        fout << ".text" << endl;
    }
    fout << "jal main" << endl;
    fout << "j _EOF" << endl;
    int nQuadNum = vQuads.size();
    for (int i=0;i<nQuadNum;i++){
        if (vQuads[i]->GetQuadType()==QUAD_TYPE_FUNCLABEL||vQuads[i]->GetQuadType()==QUAD_TYPE_LABEL){
            if (vQuads[i]->GetOp()=="main")
                sm_sLocation = "main";
            else if (vQuads[i]->GetQuadType()==QUAD_TYPE_FUNCLABEL){
                string sPrefix = FUNC_LABEL_PREFIX;
                sm_sLocation = vQuads[i]->GetOp().substr(sPrefix.size());
            }
            fout << vQuads[i]->GetOp() << ":" << endl;
        }
        else if (vQuads[i]->GetQuadType()==QUAD_TYPE_BEGINFUNC){
            __OutputQuad(i, fout, vQuads);
            fout << left << setw(30) << "subu $sp, $sp, 8" << "# decrement sp to make space to save ra, fp" << endl;
            fout << left << setw(30) << "sw $fp, 8($sp)" << "# save fp" << endl;
            fout << left << setw(30) << "sw $ra, 4($sp)" << "# save ra" << endl;
            fout << left << setw(30) << "addiu $fp, $sp, 8" << "# set up new fp" << endl;
            fout << left << setw(30) << "subu $sp, $sp, " + vQuads[i]->GetArg1() << "# decrement sp to make space for locals/temps" << endl;
            for (int j=0;j<vFuncIndex.size();j++){
                if (vFuncIndex[j]->GetFuncName()==sm_sLocation){
                    int nIndex = vFuncIndex[j]->GetIndex() + 1;
                    while (nIndex<vSymbolTable.size()&&vSymbolTable[nIndex]->GetCatagory()!=CAT_FUNC){
                        if (vSymbolTable[nIndex]->GetCatagory()==CAT_CONST){
                            __lwInto(i, "$t0", vSymbolTable[nIndex]->GetConstValue(), fout, vQuads, vSymbolTable, vFuncIndex);
                            __swInto(i, "$t0", vSymbolTable[nIndex]->GetName(), fout, vQuads, vSymbolTable, vFuncIndex);
                        }
                        nIndex++;
                    }
                    break;
                }
            }
        }
        else if (vQuads[i]->GetQuadType()==QUAD_TYPE_PUSH){
            __OutputQuad(i, fout, vQuads);
            fout << left << setw(30) << "subu $sp, $sp, 4" << "# decrement sp to make space for param" << endl;
            __lwInto(i, "$t0", vQuads[i]->GetArg1(), fout, vQuads, vSymbolTable, vFuncIndex);
            fout << left << setw(30) << "sw $t0, 4($sp)" << "# copy param value to stack" << endl;
        }
        else if (vQuads[i]->GetQuadType()==QUAD_TYPE_PRINTPUSH){
            __OutputQuad(i, fout, vQuads);
            if (CSymbolTableItem::isString(vQuads[i]->GetArg1()))
                fout << "la $a0, " << vQuads[i]->GetArg1() << endl;
            else
                __lwInto(i, "$a0", vQuads[i]->GetArg1(), fout, vQuads, vSymbolTable, vFuncIndex);
        }
        /*
        else if (vQuads[i]->GetQuadType()==QUAD_TYPE_SCANPUSH)
            __OutputQuad(i, fout, vQuads);
        */
        else if (vQuads[i]->GetQuadType()==QUAD_TYPE_POP){
            __OutputQuad(i, fout, vQuads);
            fout << left << setw(30) << "add $sp, $sp, " + vQuads[i]->GetArg1() << "# pop params off stack" << endl;
        }
        else if (vQuads[i]->GetQuadType()==QUAD_TYPE_SCANPOP){
            __OutputQuad(i, fout, vQuads);
            __swInto(i, "$v0", vQuads[i]->GetArg1(), fout, vQuads, vSymbolTable, vFuncIndex);
        }
        else if (vQuads[i]->GetQuadType()==QUAD_TYPE_ASSIGN){
            __OutputQuad(i, fout, vQuads);
            if (vQuads[i]->GetOp()=="="){
                if ((vQuads[i]->GetArg2().substr(0, 6)=="LCall ")){
                    fout << "jal " << vQuads[i]->GetArg2().substr(6) <<endl;
                    __swInto(i, "$v0", vQuads[i]->GetArg1(), fout, vQuads, vSymbolTable, vFuncIndex);
                }
                else{
                    __lwInto(i, "$t0", vQuads[i]->GetArg2(), fout, vQuads, vSymbolTable, vFuncIndex);
                    __swInto(i, "$t0", vQuads[i]->GetArg1(), fout, vQuads, vSymbolTable, vFuncIndex);
                }
            }
            else if (vQuads[i]->GetOp()=="[]="){
                __lwInto(i, "$t0", vQuads[i]->GetArg1(), fout, vQuads, vSymbolTable, vFuncIndex);
                __swArrayInto(i, "$t0", vQuads[i]->GetResult(), atoi(vQuads[i]->GetArg2().c_str()), fout, vQuads, vSymbolTable, vFuncIndex);
            }
            else if (vQuads[i]->GetOp()=="=[]"){
                __lwArrayInto(i, "$t0", vQuads[i]->GetArg1(), atoi(vQuads[i]->GetArg2().c_str()), fout, vQuads, vSymbolTable, vFuncIndex);
                __swInto(i, "$t0", vQuads[i]->GetResult(), fout, vQuads, vSymbolTable, vFuncIndex);
            }
        }
        else if (vQuads[i]->GetQuadType()==QUAD_TYPE_CALC){
            __OutputQuad(i, fout, vQuads);
            __lwInto(i, "$t0", vQuads[i]->GetArg1(), fout, vQuads, vSymbolTable, vFuncIndex);
            if (vQuads[i]->GetArg2()!=QUAD_EMPTY)
                __lwInto(i, "$t1", vQuads[i]->GetArg2(), fout, vQuads, vSymbolTable, vFuncIndex);
            if (vQuads[i]->GetOp()=="+")
                fout << left << setw(30) << "add $t2, $t0, $t1" << "# $t2 = $t0 + $t1" << endl;
            else if (vQuads[i]->GetOp()=="*")
                fout << left << setw(30) << "mul $t2, $t0, $t1" << "# $t2 = $t0 * $t1" << endl;
            else if (vQuads[i]->GetOp()=="/")
                fout << left << setw(30) << "div $t2, $t0, $t1" << "# $t2 = $t0 / $t1" << endl;
            else if (vQuads[i]->GetOp()=="-"&&vQuads[i]->GetArg2()!=QUAD_EMPTY)
                fout << left << setw(30) << "sub $t2, $t0, $t1" << "# $t2 = $t0 - $t1" << endl;
            else if (vQuads[i]->GetOp()=="-"&&vQuads[i]->GetArg2()==QUAD_EMPTY)
                fout << left << setw(30) << "sub $t2, $zero, $t0" << "# $t2 = 0 - $t0" << endl;
            __swInto(i, "$t2", vQuads[i]->GetResult(), fout, vQuads, vSymbolTable, vFuncIndex);
        }
        else if (vQuads[i]->GetQuadType()==QUAD_TYPE_CONDJUMP){
            __OutputQuad(i, fout, vQuads);
            __lwInto(i, "$t0", vQuads[i]->GetArg1(), fout, vQuads, vSymbolTable, vFuncIndex);
            __lwInto(i, "$t1", vQuads[i]->GetArg2(), fout, vQuads, vSymbolTable, vFuncIndex);
            if (vQuads[i]->GetOp()=="j=")
                fout << left << setw(30) << "beq $t0, $t1, " + vQuads[i]->GetResult() << "# if $t0 == $t1 branch" << endl;
            else if (vQuads[i]->GetOp()=="j!=")
                fout << left << setw(30) << "bne $t0, $t1, " + vQuads[i]->GetResult() << "# if $t0 != $t1 branch" << endl;
            else if (vQuads[i]->GetOp()=="j>")
                fout << left << setw(30) << "bgt $t0, $t1, " + vQuads[i]->GetResult() << "# if $t0 > $t1 branch" << endl;
            else if (vQuads[i]->GetOp()=="j>=")
                fout << left << setw(30) << "bge $t0, $t1, " + vQuads[i]->GetResult() << "# if $t0 >= $t1 branch" << endl;
            else if (vQuads[i]->GetOp()=="j<")
                fout << left << setw(30) << "blt $t0, $t1, " + vQuads[i]->GetResult() << "# if $t0 < $t1 branch" << endl;
            else if (vQuads[i]->GetOp()=="j<=")
                fout << left << setw(30) << "ble $t0, $t1, " + vQuads[i]->GetResult() << "# if $t0 <= $t1 branch" << endl;
        }
        else if (vQuads[i]->GetQuadType()==QUAD_TYPE_JUMP){
            __OutputQuad(i, fout, vQuads);
            fout << "j " << vQuads[i]->GetResult() << endl;
        }
        else if (vQuads[i]->GetQuadType()==QUAD_TYPE_RETURN){
            __OutputQuad(i, fout, vQuads);
            if (vQuads[i]->GetArg1()!=QUAD_EMPTY){
                __lwInto(i, "$t0", vQuads[i]->GetArg1(), fout, vQuads, vSymbolTable, vFuncIndex);
                fout << left << setw(30) << "move $v0, $t0" << "# assign return value into $v0" << endl;
            }
            fout << left << setw(30) << "move $sp, $fp" << "# pop callee frame off stack" << endl;
            fout << left << setw(30) << "lw $ra, -4($fp)" << "# restore saved ra" << endl;
            fout << left << setw(30) << "lw $fp, 0($fp)" << "# restore saved fp" << endl;
            fout << left << setw(30) << "jr $ra" << "# return from function" << endl;
        }
        else if (vQuads[i]->GetQuadType()==QUAD_TYPE_ENDFUNC){
            __OutputQuad(i, fout, vQuads);
            fout << "# (below handles reaching end of function body with no explicit return)" << endl;
            fout << left << setw(30) << "move $sp, $fp" << "# pop callee frame off stack" << endl;
            fout << left << setw(30) << "lw $ra, -4($fp)" << "# restore saved ra" << endl;
            fout << left << setw(30) << "lw $fp, 0($fp)" << "# restore saved fp" << endl;
            fout << left << setw(30) << "jr $ra" << "# return from function" << endl;
        }
        else if (vQuads[i]->GetQuadType()==QUAD_TYPE_LCALL){
            __OutputQuad(i, fout, vQuads);
            fout << "jal " << vQuads[i]->GetArg1() << endl;
        }
        else if (vQuads[i]->GetQuadType()==QUAD_TYPE_SYSCALL){
            __OutputQuad(i, fout, vQuads);
            if (vQuads[i]->GetArg1()=="_PrintInt")
                fout << "li $v0, 1" << endl;
            else if (vQuads[i]->GetArg1()=="_PrintString")
                fout << "li $v0, 4" << endl;
            else if (vQuads[i]->GetArg1()=="_PrintChar")
                fout << "li $v0, 11" << endl;
            else if (vQuads[i]->GetArg1()=="_ScanInt")
                fout << "li $v0, 5" << endl;
            else if (vQuads[i]->GetArg1()=="_ScanChar")
                fout << "li $v0, 12" << endl;
            fout << "syscall" << endl;
        }
    }
    fout << "_EOF: " << endl;
    fout << "li $v0, 10" << endl;
    fout << "syscall" << endl;
    fout << "## end of file" << endl;
}

void CCodeGeneration::__GenGlobalCode(ofstream &fout, vector<CQuadruple*> vQuads, vector<CSymbolTableItem*> vSymbolTable, vector<CFuncIndex*> vFuncIndex)
{
    int nGlobalConstNum = 0, nGlobalVarNum = 0, nStringNum = 0;
    for (int i=0;i<vFuncIndex[0]->GetIndex();i++){
        if (vSymbolTable[i]->GetCatagory()==CAT_CONST)
            nGlobalConstNum++;
        else if (vSymbolTable[i]->GetCatagory()==CAT_VAR||vSymbolTable[i]->GetCatagory()==CAT_ARRAY)
            nGlobalVarNum++;
        else
            nStringNum++;
    }
    if (nGlobalConstNum + nStringNum!=0){
        __GenSegmentAnnotation(fout, "data");
        fout << ".data" << endl;
    }
    for (int i=0;i<nGlobalConstNum;i++){
        fout << left << setw(15) << vSymbolTable[i]->GetName() + ":" << left << setw(15) << ".word";
        if (CQuadruple::isNum(vSymbolTable[i]->GetConstValue()))
            fout << vSymbolTable[i]->GetConstValue();
        else
            fout << (int)*(vSymbolTable[i]->GetConstValue().c_str() + 1);
        fout << endl;
    }
    for (int i=nGlobalConstNum + nGlobalVarNum;i<nGlobalConstNum + nGlobalVarNum + nStringNum;i++){
        fout << left << setw(15) << vSymbolTable[i]->GetName() + ":" << left << setw(15) << ".asciiz";
        fout << "\"" << vSymbolTable[i]->GetConstValue() << "\"" << endl;
    }
    if (nGlobalVarNum!=0){
        fout << endl;
        __GenSegmentAnnotation(fout, "text");
        fout << ".text" << endl;
        sm_bHasTextInGlobal = true;
    }
    int nOffset = 0;
    for (int i=0;i<nGlobalVarNum;i++){
        vSymbolTable[i + nGlobalConstNum]->SetOffset(nOffset);
        if (vSymbolTable[i + nGlobalConstNum]->GetCatagory()==CAT_VAR)
            nOffset += 4;
        else if (vSymbolTable[i + nGlobalConstNum]->GetCatagory()==CAT_ARRAY)
            nOffset += (vSymbolTable[i + nGlobalConstNum]->GetArraySize() * 4);
    }
    if (nGlobalVarNum!=0)
        fout << "subu $gp, $gp, " << nOffset << endl;
}

void CCodeGeneration::__lwInto(int QuadIndex, string sRegister, string sVarName, ofstream &fout, vector<CQuadruple*> vQuads, vector<CSymbolTableItem*> vSymbolTable, vector<CFuncIndex*> vFuncIndex)
{
    int nOffset;
    if (CQuadruple::isTemp(sVarName)){
        nOffset = vFuncIndex[vQuads[QuadIndex]->GetFuncIndex()]->GetOffsetInTempVarTable(sVarName);
        fout << left << setw(30) << "lw " + sRegister + ", " + __itos(nOffset) + "($fp)";
    }
    else{
        int nSymbolTableIndex = CParser::FindNameInLocal(sVarName, vFuncIndex[vQuads[QuadIndex]->GetFuncIndex()]->GetFuncName());
        if (nSymbolTableIndex!=-1){
            nOffset = vSymbolTable[nSymbolTableIndex]->GetOffset();
            fout << left << setw(30) << "lw " + sRegister + ", " + __itos(nOffset) + "($fp)";
        }
        else{
            nSymbolTableIndex = CParser::FindNameInGlobal(sVarName);
            if (nSymbolTableIndex!=-1){
                if (vSymbolTable[nSymbolTableIndex]->GetCatagory()==CAT_CONST)
                    fout << left << setw(30) << "lw " + sRegister + ", " + sVarName;
                else if (vSymbolTable[nSymbolTableIndex]->GetCatagory()==CAT_VAR){
                    nOffset = vSymbolTable[nSymbolTableIndex]->GetOffset();
                    fout << left << setw(30) << "lw " + sRegister + ", " + __itos(nOffset) + "($gp)";
                }
            }
            else
                    fout << left << setw(30) << "li " + sRegister + ", " + sVarName;
        }
    }
    fout  << "# " << sRegister << " = " << sVarName << endl;
}

void CCodeGeneration::__lwArrayInto(int QuadIndex, string sRegister, string sVarName, int nIndex, ofstream &fout, vector<CQuadruple*> vQuads, vector<CSymbolTableItem*> vSymbolTable, vector<CFuncIndex*> vFuncIndex)
{
    int nOffset;
    int nSymbolTableIndex = CParser::FindNameInLocal(sVarName, vFuncIndex[vQuads[QuadIndex]->GetFuncIndex()]->GetFuncName());
    if (nSymbolTableIndex!=-1){
        nOffset = vSymbolTable[nSymbolTableIndex]->GetOffset();
        __lwInto(QuadIndex, "$t1", vQuads[QuadIndex]->GetArg2(), fout, vQuads, vSymbolTable, vFuncIndex);
        fout << "mul $t2, $t1, 4" << endl;
        fout << "subu $t3, $fp, $t2" << endl;
        fout << left << setw(30) << "lw " + sRegister + ", " + __itos(nOffset) + "($t3)";
    }
    else{
        nSymbolTableIndex = CParser::FindNameInGlobal(sVarName);
        if (nSymbolTableIndex!=-1){
            nOffset = vSymbolTable[nSymbolTableIndex]->GetOffset();
            __lwInto(QuadIndex, "$t1", vQuads[QuadIndex]->GetArg2(), fout, vQuads, vSymbolTable, vFuncIndex);
            fout << "mul $t2, $t1, 4" << endl;
            fout << "addu $t3, $gp, $t2" << endl;
            fout << left << setw(30) << "lw " + sRegister + ", " + __itos(nOffset) + "($t3)";
        }
    }
    fout << "# " << sRegister << " = " << sVarName << "[" << vQuads[QuadIndex]->GetArg2() << "]" << endl;
}

void CCodeGeneration::__swInto(int QuadIndex, string sRegister, string sVarName, ofstream &fout, vector<CQuadruple*> vQuads, vector<CSymbolTableItem*> vSymbolTable, vector<CFuncIndex*> vFuncIndex)
{
    int nOffset;
    if (CQuadruple::isTemp(sVarName)){
        nOffset = vFuncIndex[vQuads[QuadIndex]->GetFuncIndex()]->GetOffsetInTempVarTable(sVarName);
        fout << left << setw(30) << "sw " + sRegister + ", " + __itos(nOffset) + "($fp)";
    }
    else{
        int nSymbolTableIndex = CParser::FindNameInLocal(sVarName, vFuncIndex[vQuads[QuadIndex]->GetFuncIndex()]->GetFuncName());
        if (nSymbolTableIndex!=-1){
            nOffset = vSymbolTable[nSymbolTableIndex]->GetOffset();
            fout << left << setw(30) << "sw " + sRegister + ", " + __itos(nOffset) + "($fp)";
        }
        else{
            nSymbolTableIndex = CParser::FindNameInGlobal(sVarName);
            if (nSymbolTableIndex!=-1){
                nOffset = vSymbolTable[nSymbolTableIndex]->GetOffset();
                fout << left << setw(30) << "sw " + sRegister + ", " + __itos(nOffset) + "($gp)";
            }
        }
    }
    fout << "# " << sVarName << " = " << sRegister << endl;
}

void CCodeGeneration::__swArrayInto(int QuadIndex, string sRegister, string sVarName, int nIndex, ofstream &fout, vector<CQuadruple*> vQuads, vector<CSymbolTableItem*> vSymbolTable, vector<CFuncIndex*> vFuncIndex)
{
    int nOffset;
    int nSymbolTableIndex = CParser::FindNameInLocal(sVarName, vFuncIndex[vQuads[QuadIndex]->GetFuncIndex()]->GetFuncName());
    if (nSymbolTableIndex!=-1){
        nOffset = vSymbolTable[nSymbolTableIndex]->GetOffset();
        __lwInto(QuadIndex, "$t1", vQuads[QuadIndex]->GetArg2(), fout, vQuads, vSymbolTable, vFuncIndex);
        fout << "mul $t2, $t1, 4" << endl;
        fout << "subu $t3, $fp, $t2" << endl;
        fout << left << setw(30) << "sw " + sRegister + ", " + __itos(nOffset) + "($t3)";
    }
    else{
        nSymbolTableIndex = CParser::FindNameInGlobal(sVarName);
        if (nSymbolTableIndex!=-1){
            nOffset = vSymbolTable[nSymbolTableIndex]->GetOffset();
            __lwInto(QuadIndex, "$t1", vQuads[QuadIndex]->GetArg2(), fout, vQuads, vSymbolTable, vFuncIndex);
            fout << "mul $t2, $t1, 4" << endl;
            fout << "addu $t3, $gp, $t2" << endl;
            fout << left << setw(30) << "sw " + sRegister + ", " + __itos(nOffset) + "($t3)";
        }
    }
    fout << "# " << sVarName << "[" << vQuads[QuadIndex]->GetArg2() << "] = " << sRegister << endl;
}

#endif

string CCodeGeneration::__itos(int nNum)
{
    stringstream ss;
    string s;
    ss << nNum;
    ss >> s;
    return s;
}

void CCodeGeneration::__OutputQuad(int QuadIndex, ofstream &fout, vector<CQuadruple*> vQuads)
{
#ifdef DEBUG_GENCODE_PRINTQUAD
    fout << "# " << vQuads[QuadIndex]->TransQuad() << endl;
#endif
    return ;
}