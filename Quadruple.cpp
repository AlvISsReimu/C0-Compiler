#include "Quadruple.h"

int CQuadruple::sm_iLabelIndex = 0;
int CQuadruple::sm_gTempIndex[MAX_TEMP_INDEX];
int CQuadruple::sm_gFuncTempDirty[MAX_TEMP_INDEX];

CQuadruple::CQuadruple(string sOp, string sArg1, string sArg2, string sResult)
{
    m_sOp = sOp;
    m_sArg1 = sArg1;
    m_sArg2 = sArg2;
    m_sResult = sResult;
    m_nFuncIndex = -1;
    if (sOp=="+"||sOp=="-"||sOp=="*"||sOp=="/")
        m_eQuadType = QUAD_TYPE_CALC;
    else if (sOp=="="||sOp=="[]="||sOp=="=[]")
        m_eQuadType = QUAD_TYPE_ASSIGN;
    else if (sOp=="j")
        m_eQuadType = QUAD_TYPE_JUMP;
    else if (sOp=="j="||sOp=="j!="||sOp=="j>"||sOp=="j>="||sOp=="j<"||sOp=="j<=")
        m_eQuadType = QUAD_TYPE_CONDJUMP;
    else if (isLabel(sOp))
        m_eQuadType = QUAD_TYPE_LABEL;
    else if (isFunc(sOp))
        m_eQuadType = QUAD_TYPE_FUNCLABEL;
    else if (sOp=="LCall")
        m_eQuadType = QUAD_TYPE_LCALL;
    else if (sOp=="SysCall")
        m_eQuadType = QUAD_TYPE_SYSCALL;
    else if (sOp=="PopParams")
        m_eQuadType = QUAD_TYPE_POP;
    else if (sOp=="PushParam")
        m_eQuadType = QUAD_TYPE_PUSH;
    else if (sOp=="PrintPushParam")
        m_eQuadType = QUAD_TYPE_PRINTPUSH;
    else if (sOp=="ScanPushParam")
        m_eQuadType = QUAD_TYPE_SCANPUSH;
    else if (sOp=="ScanPopParam")
        m_eQuadType = QUAD_TYPE_SCANPOP;
    else if (sOp=="BeginFunc")
        m_eQuadType = QUAD_TYPE_BEGINFUNC;
    else if (sOp=="EndFunc")
        m_eQuadType = QUAD_TYPE_ENDFUNC;
    else if (sOp=="Return")
        m_eQuadType = QUAD_TYPE_RETURN;
}

CQuadruple::~CQuadruple() {}

string CQuadruple::GetOp() { return m_sOp; }
string CQuadruple::GetArg1() { return m_sArg1; }
string CQuadruple::GetArg2() { return m_sArg2; }
string CQuadruple::GetResult() { return m_sResult; }
QuadType CQuadruple::GetQuadType() { return m_eQuadType; }
int CQuadruple::GetFuncIndex() { return m_nFuncIndex; }
void CQuadruple::SetArg1(string sArg1) { m_sArg1 = sArg1; }
void CQuadruple::SetArg2(string sArg2) { m_sArg2 = sArg2; }
void CQuadruple::SetResult(string sResult) { m_sResult = sResult; }
void CQuadruple::SetFuncIndex(int nFuncIndex) { m_nFuncIndex = nFuncIndex; }

string CQuadruple::GenTemp()
{
    stringstream ssTemp;
    string sTemp;
    int i = 0;
    while (sm_gTempIndex[i]==TEMP_USED)
        i++;
    sm_gTempIndex[i] = TEMP_USED;
    sm_gFuncTempDirty[i] = TEMP_USED;
    ssTemp << i;
    ssTemp >> sTemp;
    sTemp = TEMP_PREFIX + sTemp;
#ifdef DEBUG_GEN_DROP_TEMP
    cout << "Gen " << sTemp << endl;
#endif
    return sTemp;
}

void CQuadruple::DropTemp(string sTemp)
{
    string sPrefix = TEMP_PREFIX;
    string sTemp1 = sTemp.substr(0, sPrefix.size());
    if (sTemp1==sPrefix){
        string sTemp2 = sTemp.substr(sPrefix.size());
        const char* pTemp = sTemp2.c_str();
        int iTemp = atoi(pTemp);
        sm_gTempIndex[iTemp] = TEMP_UNUSED;
#ifdef DEBUG_GEN_DROP_TEMP
        cout << "Drop " << sTemp << endl;
#endif
    }
}

string CQuadruple::GenLabel()
{
    stringstream ssTemp;
    string sTemp;
    ssTemp << sm_iLabelIndex;
    ssTemp >> sTemp;
    sTemp = LABEL_PREFIX + sTemp;
    sm_iLabelIndex++;
    return sTemp;
}

void CQuadruple::ResetFuncTempDirty()
{
    for (int i=0;i<MAX_TEMP_INDEX;i++)
        sm_gFuncTempDirty[i] = TEMP_UNUSED;
}

int CQuadruple::SumFuncTempDirty()
{
    int iSum = 0;
    for (int i=0;i<MAX_TEMP_INDEX;i++){
        if (sm_gFuncTempDirty[i]==TEMP_USED)
            iSum++;
    }
    return iSum;
}

bool CQuadruple::isLabel(string sTest)
{
    string sPrefix = LABEL_PREFIX;
    string sTemp = sTest.substr(0, sPrefix.size());
    return (sTemp==LABEL_PREFIX);
}

bool CQuadruple::isFunc(string sTest)
{
    string sPrefix = FUNC_LABEL_PREFIX;
    string sTemp = sTest.substr(0, sPrefix.size());
    return (sTemp==FUNC_LABEL_PREFIX||sTest=="main");
}

bool CQuadruple::isTemp(string sTest)
{
    string sPrefix = TEMP_PREFIX;
    string sTemp = sTest.substr(0, sPrefix.size());
    return (sTemp==TEMP_PREFIX);
}

bool CQuadruple::isString(string sTest)
{
    string sPrefix = STRING_PREFIX;
    string sTemp = sTest.substr(0, sPrefix.size());
    return (sTemp==STRING_PREFIX);
}

bool CQuadruple::isNum(string sTest)
{
    const char* pTemp = sTest.c_str();
    if (pTemp[0]=='-'||(pTemp[0]>='0'&&pTemp[0]<='9'))
        return true;
    return false;
}

#ifdef DEBUG_TEST_QUAD
void CQuadruple::PrintQuad(ofstream &fout)
{
    if ((m_sArg1==QUAD_EMPTY)&&(m_sArg2==QUAD_EMPTY)&&(m_sResult==QUAD_EMPTY)){
        string sLablePrefix = LABEL_PREFIX;
        string sFuncPrefix = FUNC_LABEL_PREFIX;
        string sTemp1 = m_sOp.substr(0, sLablePrefix.size());
        string sTemp2 = m_sOp.substr(0, sFuncPrefix.size());
        if ((sTemp1==LABEL_PREFIX)||(sTemp2==FUNC_LABEL_PREFIX)||(m_sOp=="main"))
            fout << m_sOp << ":" << endl;
        else
            fout << "\t" << m_sOp << ";" << endl;
    }
    else if ((m_sArg2==QUAD_EMPTY)&&(m_sResult==QUAD_EMPTY))
        fout << "\t" << m_sOp << " " << m_sArg1 << ";" << endl;
    else if ((m_sOp=="+")||(m_sOp=="-")||(m_sOp=="*")||(m_sOp=="/")){
            if (m_sArg2!=QUAD_EMPTY)
                fout << "\t" << m_sResult << " = " << m_sArg1 << " " << m_sOp << " " << m_sArg2 << ";" << endl;
            else
                fout << "\t" << m_sResult << " = " << m_sOp << m_sArg1 << ";" << endl;
        }
    else if (m_sOp=="=")
        fout << "\t" << m_sArg1 << " = " << m_sArg2 << ";" << endl;
    else if (m_sOp=="=[]")
        fout << "\t" << m_sResult << " = " << m_sArg1 << "[" << m_sArg2 << "];" << endl;
    else if (m_sOp=="[]=")
        fout << "\t" << m_sResult << "[" << m_sArg2 << "] = " << m_sArg1 << ";" << endl;
    else if (m_sOp=="j")
        fout << "\tGoto: " << m_sResult << ";" << endl;
    else if ((m_sOp=="j=")||(m_sOp=="j!=")||(m_sOp=="j>")||(m_sOp=="j>=")||(m_sOp=="j<")||(m_sOp=="j<=")){
        string sTemp = m_sOp.substr(1);
        fout << "\tIf " << m_sArg1 << " " << sTemp << " " << m_sArg2 << " Goto " << m_sResult << ";" << endl;
    }
}
#endif

string CQuadruple::TransQuad()
{
    string s;
    if ((m_sArg1==QUAD_EMPTY)&&(m_sArg2==QUAD_EMPTY)&&(m_sResult==QUAD_EMPTY)){
        string sLablePrefix = LABEL_PREFIX;
        string sFuncPrefix = FUNC_LABEL_PREFIX;
        string sTemp1 = m_sOp.substr(0, sLablePrefix.size());
        string sTemp2 = m_sOp.substr(0, sFuncPrefix.size());
        if ((sTemp1==LABEL_PREFIX)||(sTemp2==FUNC_LABEL_PREFIX)||(m_sOp=="main"))
            s += m_sOp + ":";
        else
            s += m_sOp;
    }
    else if ((m_sArg2==QUAD_EMPTY)&&(m_sResult==QUAD_EMPTY))
        s += m_sOp + " " + m_sArg1;
    else if ((m_sOp=="+")||(m_sOp=="-")||(m_sOp=="*")||(m_sOp=="/")){
        if (m_sArg2!=QUAD_EMPTY)
            s += m_sResult + " = " + m_sArg1 + " " + m_sOp + " " + m_sArg2;
        else
            s += m_sResult + " = " + m_sOp + m_sArg1;
    }
    else if (m_sOp=="=")
        s += m_sArg1 + " = " + m_sArg2;
    else if (m_sOp=="=[]")
        s += m_sResult + " = " + m_sArg1 + "[" + m_sArg2 + "]";
    else if (m_sOp=="[]=")
        s += m_sResult + "[" + m_sArg2 + "] = " + m_sArg1;
    else if (m_sOp=="j")
        s += "Goto: " + m_sResult;
    else if ((m_sOp=="j=")||(m_sOp=="j!=")||(m_sOp=="j>")||(m_sOp=="j>=")||(m_sOp=="j<")||(m_sOp=="j<=")){
        string sTemp = m_sOp.substr(1);
        s += "If " + m_sArg1 + " " + sTemp + " " + m_sArg2 + " Goto " + m_sResult;
    }
    return s;
}