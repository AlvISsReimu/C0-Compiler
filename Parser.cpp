#include "Parser.h"

CASTNode* CParser::sm_pAST = new CASTNode(PROGRAM);
string CParser::sm_sCode = "";
string CParser::sm_sSymbol = "";
string CParser::sm_sValue = "";
string CParser::sm_sLocation = "";
vector<CSymbolTableItem*> CParser::sm_vSymbolTable;
vector<CFuncIndex*> CParser::sm_vFuncIndex;
bool CParser::sm_bisLeafFunc = true;
bool CParser::sm_bhasReturn = false;
set<string> CParser::sm_sStatementFIRST;
set<string> CParser::sm_sCompStmtFIRST;
set<string> CParser::sm_sExpressionFIRST;

CParser::CParser() {}

CParser::~CParser() {}

void CParser::__SetCode(const string* sCode) { sm_sCode = *sCode; }
void CParser::__SetLocation(string sLocation) { sm_sLocation = sLocation; }
string CParser::__Symbol() { return sm_sSymbol; }
string CParser::__Value() { return sm_sValue; }
string CParser::__Location() { return sm_sLocation; }

void CParser::__SetLeafFunc(bool bLeafFunc) { sm_bisLeafFunc = bLeafFunc; }
void CParser::__SetHasReturn(bool bHasReturn) { sm_bhasReturn = bHasReturn; }

void CParser::__InitialSet()
{

    sm_sStatementFIRST.insert("IFTK");
    sm_sStatementFIRST.insert("WHILETK");
    sm_sStatementFIRST.insert("FORTK");
    sm_sStatementFIRST.insert("LBRACE");
    sm_sStatementFIRST.insert("IDEN");
    sm_sStatementFIRST.insert("PRINTFTK");
    sm_sStatementFIRST.insert("SCANFTK");
    sm_sStatementFIRST.insert("SEMICN");
    sm_sStatementFIRST.insert("RETURNTK");
    sm_sCompStmtFIRST = sm_sStatementFIRST;
    sm_sCompStmtFIRST.insert("CONSTTK");
    sm_sCompStmtFIRST.insert("CHARTK");
    sm_sCompStmtFIRST.insert("INTTK");
    sm_sExpressionFIRST.insert("PLUS");
    sm_sExpressionFIRST.insert("MINU");
    sm_sExpressionFIRST.insert("IDEN");
    sm_sExpressionFIRST.insert("INTCON");
    sm_sExpressionFIRST.insert("CHARCON");
    sm_sExpressionFIRST.insert("LPARENT");
}

bool CParser::__isMatch(string sSymbol) { return (sSymbol==__Symbol()?true:false); }

void CParser::Parser(const string* sCode)
{
    __InitialSet();
    __SetCode(sCode);
    CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    __Program();

#ifdef DEBUG_TEST_TREE
    ofstream fout_TestTree("TestTree.txt");
    CASTNode::TestTree(sm_pAST, 0, fout_TestTree);
    fout_TestTree.close();
#endif
    if (CError::GetErrorSum()!=0)
        return ;
    CTraverser::ASTTraversal(sm_pAST);
    CTraverser::OptimizeLabels();
    __FillOffset();

#ifdef DEBUG_TEST_QUAD
    ofstream fout_TestQuad("TestQuad.txt");
    CTraverser::PrintQuads(fout_TestQuad);
    fout_TestQuad.close();
#endif

#ifdef DEBUG_SYMBOL_TABLE
    ofstream fout_SymbolTable("SymbolTable.txt");
    CParser::PrintSymbolTable(fout_SymbolTable);
    fout_SymbolTable.close();
#endif

}

void CParser::__Program()
{
    sm_pAST->AddChild(__ConstDec());
    sm_pAST->AddChild(__VarDec());
    while (__isMatch("INTTK")||__isMatch("CHARTK")||__isMatch("VOIDTK")){
        string sReturnType = sm_sValue;
        string sTemp = "";
        CLexicalAnalysis::GetNextSymbol(&sm_sCode, sm_sSymbol, sm_sValue, sTemp, 1);
        if (sTemp=="MAINTK")
            break;
        sm_pAST->InsertSiblings(__FuncDef(), 2);
        if (sm_bhasReturn==false&&(sReturnType=="INTTK"||sReturnType=="CHARTK"))
            CError::PrintErrorMsg(40);
    }
    if (!__isMatch("VOIDTK")){
        if (__isMatch("INTTK")||__isMatch("CHARTK"))
            CError::PrintErrorMsg(37);
        else
            CError::PrintErrorMsg(22);
        set<string> sStopSet = sm_sCompStmtFIRST;
        sStopSet.insert("MAINTK");
        sStopSet.insert("LPARENT");
        CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
    }
    else
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    if (!__isMatch("MAINTK")){
        CError::PrintErrorMsg(21);
        return ;
    }
    else
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    CFuncIndex* iFuncIndex = new CFuncIndex("main", GetSymbolTable().size());
    sm_vFuncIndex.push_back(iFuncIndex);
    CSymbolTableItem* iSymbolTableItem = new CSymbolTableItem("main", CAT_FUNC, "void", NULL, "0", 0, 0, true);
    sm_vSymbolTable.push_back(iSymbolTableItem);
    __SetLocation("main");
    if (!__isMatch("LPARENT")){
        CError::PrintErrorMsg(13);
        set<string> sStopSet = sm_sCompStmtFIRST;
        sStopSet.insert("RPARENT");
        CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
    }
    else
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    if (!__isMatch("RPARENT")){
        string sTemp = "";
        CLexicalAnalysis::GetNextSymbol(&sm_sCode, sm_sSymbol, sm_sValue, sTemp, 1);
        if (sTemp=="LBRACE")
            CError::PrintErrorMsg(5);
        else
            CError::PrintErrorMsg(38);
        set<string> sStopSet = sm_sCompStmtFIRST;
        sStopSet.insert("RBRACE");
        CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
    }
    else
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    if (__isMatch("LBRACE"))
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    else
        CError::PrintErrorMsg(14);
    CASTNode* MainNode = new CASTNode(FUNCDEF);
    MainNode->AddChild(new CASTNode("void"));
    MainNode->AddChild(new CASTNode("main"));
    MainNode->AddChild(new CASTNode(PARATABLE));
    MainNode->AddChild(__CompStmt());
    sm_pAST->InsertSiblings(MainNode, 2);
    sm_vSymbolTable[sm_vFuncIndex[sm_vFuncIndex.size()-1]->GetIndex()]->SetLeafFunc(sm_bisLeafFunc);
    if (__isMatch("RBRACE"))
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    else
        CError::PrintErrorMsg(15);
    if (!(__isMatch("EOF")||__isMatch("")))
        CError::PrintErrorMsg(16);
#ifdef DEBUG_PARSER
    cout << "<Program>" << endl;
#endif
}

CASTNode* CParser::__ConstDec()
{
    CASTNode* CONSTDECNode = new CASTNode(CONSTDEC);
    if (__isMatch("CONSTTK")){
        __ConstDecPart(CONSTDECNode);
        while (__isMatch("CONSTTK"))
            __ConstDecPart(CONSTDECNode);
    }
#ifdef DEBUG_PARSER
    cout << "<ConstDec>" << endl;
#endif
    return CONSTDECNode;
}

void CParser::__ConstDecPart(CASTNode* CONSTDECNode)
{
    CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    CONSTDECNode->InsertSiblings(__ConstDef(), 0);
    if (!__isMatch("SEMICN"))
        CError::PrintErrorMsg(17);
    else
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
}

CASTNode* CParser::__ConstDef()
{
    CASTNode* CONSTDEFNode = new CASTNode(CONSTDEF);
    if (__isMatch("INTTK")){
        CASTNode* TypeNode = new CASTNode("int");
        CONSTDEFNode->AddChild(TypeNode);
        __ConstDefPart(CONSTDEFNode, "int");
        while(__isMatch("COMMA"))
            __ConstDefPart(CONSTDEFNode, "int");
    }
    else if (__isMatch("CHARTK")){
        CASTNode* TypeNode = new CASTNode("char");
        CONSTDEFNode->AddChild(TypeNode);
        __ConstDefPart(CONSTDEFNode, "char");
        while(__isMatch("COMMA"))
            __ConstDefPart(CONSTDEFNode, "char");
    }
    else{
        CError::PrintErrorMsg(41);
        set<string> sStopSet;
        sStopSet.insert("SEMICN");
        CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
    }
#ifdef DEBUG_PARSER
    cout << "<ConstDef>" << endl;
#endif
    if (CONSTDEFNode->ChildNum()!=3)
        return NULL;
    else
        return CONSTDEFNode;
}

void CParser::__ConstDefPart(CASTNode* CONSTDEFNode, string sMode)
{
    CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    CASTNode* iIdentifier = NULL;
    CASTNode* ValueNode = NULL;
    if (__isMatch("IDEN"))
        iIdentifier = __Identifier();
    else{
        CError::PrintErrorMsg(30);
        set<string> sStopSet;
        sStopSet.insert("COMMA");
        sStopSet.insert("SEMICN");
        CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
        return ;
    }
    if (!__isMatch("ASSIGN")){
        CError::PrintErrorMsg(31);
        set<string> sStopSet;
        sStopSet.insert("COMMA");
        sStopSet.insert("SEMICN");
        CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
        return ;
    }
    else{
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
        if (sMode=="char"){
            if (!__isMatch("CHARCON")){
                CError::PrintErrorMsg(32, sm_sValue);
                set<string> sStopSet;
                sStopSet.insert("COMMA");
                sStopSet.insert("SEMICN");
                CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
                return ;
            }
            ValueNode = new CASTNode("'" + sm_sValue + "'");
            CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
        }
        else if (sMode=="int"){
            if ((!__isMatch("INTCON"))&&(!__isMatch("MINU"))){
                CError::PrintErrorMsg(32, sm_sValue);
                set<string> sStopSet;
                sStopSet.insert("COMMA");
                sStopSet.insert("SEMICN");
                CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
                return ;
            }
            ValueNode = __Integer();
        }
    }
    if ((__Location()=="")&&(FindNameInGlobal(iIdentifier->GetValue())!=-1))
        CError::PrintErrorMsg(18, iIdentifier->GetValue() + " in global");
    else if ((__Location()!="")&&(FindNameInLocal(iIdentifier->GetValue(), __Location())!=-1))
        CError::PrintErrorMsg(18, iIdentifier->GetValue() + " in function \"" + __Location() + "\"");
    else{
        CONSTDEFNode->InsertSiblings(iIdentifier, 1);
        CONSTDEFNode->InsertSiblings(ValueNode, 2);
        CSymbolTableItem* iSymbolTableItem = new CSymbolTableItem(iIdentifier->GetValue(), CAT_CONST, sMode, NULL, ValueNode->GetValue(), 0, 0, true);
        sm_vSymbolTable.push_back(iSymbolTableItem);
    }
}

CASTNode* CParser::__VarDec()
{
    CASTNode* VARDECNode = new CASTNode(VARDEC);
    if (__isMatch("INTTK")||__isMatch("CHARTK")){
        string sTemp = "";
        CLexicalAnalysis::GetNextSymbol(&sm_sCode, sm_sSymbol, sm_sValue, sTemp, 1);
        if (sTemp=="MAINTK")
            return VARDECNode;
        CLexicalAnalysis::GetNextSymbol(&sm_sCode, sm_sSymbol, sm_sValue, sTemp, 2);
        if (sTemp!="LPARENT"){
            VARDECNode->InsertSiblings(__VarDef(), 0);
            if (!__isMatch("SEMICN"))
                CError::PrintErrorMsg(17);
            else
                CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
            while (__isMatch("CONSTTK")){
                CError::PrintErrorMsg(33);
                set<string> sStopSet;
                sStopSet.insert("SEMICN");
                CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
                CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
            }
            while (__isMatch("INTTK")||__isMatch("CHARTK")){
                CLexicalAnalysis::GetNextSymbol(&sm_sCode, sm_sSymbol, sm_sValue, sTemp, 2);
                if (sTemp=="LPARENT")
                    break;
                else{
                    VARDECNode->InsertSiblings(__VarDef(), 0);
                    if (!__isMatch("SEMICN"))
                        CError::PrintErrorMsg(17);
                    else
                        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
                    while (__isMatch("CONSTTK")){
                        CError::PrintErrorMsg(33);
                        set<string> sStopSet;
                        sStopSet.insert("SEMICN");
                        CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
                        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
                    }
                }
            }
        }
    }
#ifdef DEBUG_PARSER
    cout << "<VarDec>" << endl;
#endif
    return VARDECNode;
}

CASTNode* CParser::__VarDef()
{
    CASTNode* VARDEFNode = new CASTNode(VARDEF);
    CASTNode* TypeNode = new CASTNode(TYPE);
    if (__isMatch("INTTK"))
        TypeNode->SetValue("int");
    else if (__isMatch("CHARTK"))
        TypeNode->SetValue("char");
    VARDEFNode->AddChild(TypeNode);
    __VarDefPart(VARDEFNode);
    while(__isMatch("COMMA"))
        __VarDefPart(VARDEFNode);
#ifdef DEBUG_PARSER
    cout << "<VarDef>" << endl;
#endif
    if (VARDEFNode->ChildNum()==1)
        return NULL;
    else
        return VARDEFNode;
}

void CParser::__VarDefPart(CASTNode* VARDEFNode)
{
    CASTNode* iIdentifier = NULL;
    CASTNode* NumNode = NULL;
    CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    if (__isMatch("IDEN"))
        iIdentifier = __Identifier();
    else{
        CError::PrintErrorMsg(34);
        set<string> sStopSet;
        sStopSet.insert("COMMA");
        sStopSet.insert("SEMICN");
        CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
        return ;
    }
    if (__isMatch("LBRACK")){
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
        if (__isMatch("MINU")||(sm_sValue=="0")){
            CError::PrintErrorMsg(35);
            set<string> sStopSet;
            sStopSet.insert("COMMA");
            sStopSet.insert("SEMICN");
            CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
            return ;
        }
        else if (__isMatch("INTCON")){
            NumNode = new CASTNode(sm_sValue);
            CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
            if (!__isMatch("RBRACK"))
                CError::PrintErrorMsg(6);
            else
                CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
        }
        else{
            CError::PrintErrorMsg(36, sm_sValue);
            set<string> sStopSet;
            sStopSet.insert("COMMA");
            sStopSet.insert("SEMICN");
            CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
            return ;
        }
    }
    else
        NumNode = new CASTNode("0");
    if ((__Location()=="")&&(FindNameInGlobal(iIdentifier->GetValue())!=-1))
        CError::PrintErrorMsg(18, iIdentifier->GetValue() + " in global");
    else if ((__Location()!="")&&(FindNameInLocal(iIdentifier->GetValue(), __Location())!=-1))
        CError::PrintErrorMsg(18, iIdentifier->GetValue() + " in function \"" + __Location() + "\"");
    else{
        VARDEFNode->InsertSiblings(iIdentifier, 1);
        VARDEFNode->InsertSiblings(NumNode, 2);
        Catagory eCatagory = CAT_VAR;
        if (NumNode->GetValue()!="0")
            eCatagory = CAT_ARRAY;
        CSymbolTableItem* iSymbolTableItem = new CSymbolTableItem(iIdentifier->GetValue(), eCatagory, VARDEFNode->GetChild(0)->GetValue(), NULL, "0", atoi(NumNode->GetValue().c_str()), 0, true);
        sm_vSymbolTable.push_back(iSymbolTableItem);
    }
}

CASTNode* CParser::__Identifier()
{
    CASTNode* IDNode = new CASTNode(IDENTIFIER);
    IDNode->SetValue(sm_sValue);
    CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    return IDNode;
}

CASTNode* CParser::__Integer()
{
    bool isPlus = true;
    CASTNode* IntNode = new CASTNode(INTEGER);
    if (__isMatch("PLUS")||__isMatch("MINU")){
        if (__isMatch("MINU"))
            isPlus = false;
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
        if (sm_sValue=="0")
            CError::PrintErrorMsg(24);
    }
    if (!isPlus)
        IntNode->SetValue("-" + sm_sValue);
    else
        IntNode->SetValue(sm_sValue);
    CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    return IntNode;
}

CASTNode* CParser::__FuncDef()
{
    CASTNode* FUNCDEFNode = new CASTNode(FUNCDEF);
    CASTNode* RetTypeNode = new CASTNode(TYPE);
    CASTNode* iIdentifier = NULL;
    __SetLeafFunc(true);
    __SetHasReturn(false);
    if (__isMatch("INTTK"))
        RetTypeNode->SetValue("int");
    else if (__isMatch("CHARTK"))
        RetTypeNode->SetValue("char");
    else if (__isMatch("VOIDTK"))
        RetTypeNode->SetValue("void");
    FUNCDEFNode->AddChild(RetTypeNode);
    CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    if (__isMatch("IDEN")){
        iIdentifier = __Identifier();
        if (FindNameInAll(iIdentifier->GetValue())!=-1){
            CError::PrintErrorMsg(18, iIdentifier->GetValue());
            FUNCDEFNode->AddChild(new CASTNode(""));
        }
        else{
            CFuncIndex* iFuncIndex = new CFuncIndex(iIdentifier->GetValue(), GetSymbolTable().size());
            sm_vFuncIndex.push_back(iFuncIndex);
            CSymbolTableItem* iSymbolTableItem = new CSymbolTableItem(iIdentifier->GetValue(), CAT_FUNC, RetTypeNode->GetValue(), NULL, "0", 0, 0, true);
            sm_vSymbolTable.push_back(iSymbolTableItem);
            FUNCDEFNode->AddChild(iIdentifier);
        }
    }
    else{
        CError::PrintErrorMsg(39);
        set<string> sStopSet;
        sStopSet.insert("LPARENT");
        CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
    }
    __SetLocation(FUNCDEFNode->GetChild(1)->GetValue());
    if (__isMatch("LPARENT"))
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    else{
        CError::PrintErrorMsg(13);
        set<string> sStopSet;
        sStopSet.insert("INTTK");
        sStopSet.insert("CHARTK");
        sStopSet.insert("RPARENT");
        CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
    }
    FUNCDEFNode->AddChild(__ParaTable());
    sm_vSymbolTable[sm_vFuncIndex[sm_vFuncIndex.size()-1]->GetIndex()]->SetFuncParamNum(FUNCDEFNode->GetChild(2)->SiblingNum(0));
    if (__isMatch("RPARENT"))
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    else{
        CError::PrintErrorMsg(5);
        set<string> sStopSet = sm_sCompStmtFIRST;
        sStopSet.insert("RBRACE");
        CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
    }
    if (__isMatch("LBRACE"))
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    else
        CError::PrintErrorMsg(14);
    FUNCDEFNode->AddChild(__CompStmt());
    sm_vSymbolTable[sm_vFuncIndex[sm_vFuncIndex.size()-1]->GetIndex()]->SetLeafFunc(sm_bisLeafFunc);
    if (__isMatch("RBRACE"))
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    else
        CError::PrintErrorMsg(15);
#ifdef DEBUG_PARSER
    cout << "<FunDef>" << endl;
#endif
    return FUNCDEFNode;
}

CASTNode* CParser::__ParaTable()
{
    CASTNode* PARATABLENode = new CASTNode(PARATABLE);
    int nParaNum = 1;
    if ((__isMatch("INTTK")||__isMatch("CHARTK"))){
        __ParaTablePart(PARATABLENode, nParaNum);
        while (__isMatch("COMMA")){
            nParaNum++;
            CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
            __ParaTablePart(PARATABLENode, nParaNum);
        }
    }
    else{
        if (__isMatch("IDEN")){
            CError::PrintErrorMsg(41);
            set<string> sStopSet;
            sStopSet.insert("RPARENT");
            CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
        }
    }
#ifdef DEBUG_PARSER
    cout << "<ParaTable>" << endl;
#endif
    return PARATABLENode;
}

void CParser::__ParaTablePart(CASTNode* PARATABLENode, int nParaNum)
{
    CASTNode* iIdentifier = NULL;
    string sType = "";
    if (__isMatch("INTTK"))
        sType = "int";
    else if (__isMatch("CHARTK"))
        sType = "char";
    PARATABLENode->InsertSiblings(new CASTNode(sType), 0);
    CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    if (__isMatch("IDEN"))
        iIdentifier = __Identifier();
    else{
        CError::PrintErrorMsg(42);
        set<string> sStopSet;
        sStopSet.insert("COMMA");
        sStopSet.insert("RPARENT");
        CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
        return ;
    }
    PARATABLENode->InsertSiblings(iIdentifier, 1);
    CSymbolTableItem* iSymbolTableItem = new CSymbolTableItem(iIdentifier->GetValue(), CAT_PARAM, sType, nParaNum * 4, "0", 0, 0, true);
    sm_vSymbolTable.push_back(iSymbolTableItem);
}

CASTNode* CParser::__CompStmt()
{
    CASTNode* COMPSTMTNode = new CASTNode(COMPSTMT);
    COMPSTMTNode->AddChild(__ConstDec());
    COMPSTMTNode->AddChild(__VarDec());
    COMPSTMTNode->AddChild(__StmtList());
#ifdef DEBUG_PARSER
    cout << "<CompStmt>" << endl;
#endif
    return COMPSTMTNode;
}

CASTNode* CParser::__StmtList()
{
    string sTemp = "";
    CASTNode* STMTLISTNode = new CASTNode(STMTLIST);
    while ((__isMatch("IFTK")||__isMatch("WHILETK")||__isMatch("FORTK")||__isMatch("SCANFTK")||__isMatch("PRINTFTK")||__isMatch("RETURNTK")||__isMatch("LBRACE")||__isMatch("IDEN")||__isMatch("SEMICN"))){
        //if (__isMatch("LBRACE"))
            //CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
        STMTLISTNode->InsertSiblings(__Statement(), 0);
        if (__isMatch("CHARCON")||__isMatch("INTCON")){
            CError::PrintErrorMsg(50);
            set<string> sStopSet;
            sStopSet.insert("SEMICN");
            CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
        }
        if (__isMatch("CONSTTK")||__isMatch("INTTK")||__isMatch("VOIDTK")||__isMatch("CHARTK")){
            CError::PrintErrorMsg(51);
            set<string> sStopSet;
            sStopSet.insert("SEMICN");
            CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
        }
        if (!((__isMatch("IFTK")||__isMatch("WHILETK")||__isMatch("FORTK")||__isMatch("SCANFTK")||__isMatch("PRINTFTK")||__isMatch("RETURNTK")||__isMatch("LBRACE")||__isMatch("IDEN")||__isMatch("SEMICN")||__isMatch("RBRACE")))){
            CError::PrintErrorMsg(29);
            set<string> sStopSet;
            sStopSet.insert("SEMICN");
            sStopSet.insert("RBRACE");
            CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
            CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
        }
    }
#ifdef DEBUG_PARSER
    cout << "<StmtList>" << endl;
#endif
    return STMTLISTNode;
}

CASTNode* CParser::__Statement()
{
    string sTemp = "";
    CASTNode* pTemp = NULL;
    if (__isMatch("IFTK"))
        return __CondStmt();
    else if (__isMatch("WHILETK"))
        return __WhileStmt();
    else if (__isMatch("FORTK"))
        return __ForStmt();
    else if (__isMatch("SCANFTK")){
        pTemp = __ScanStmt();
        if (__isMatch("SEMICN"))
            CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
        else
            CError::PrintErrorMsg(17);
        return pTemp;
    }
    else if (__isMatch("PRINTFTK")){
        pTemp = __PrintStmt();
        if (__isMatch("SEMICN"))
            CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
        else
            CError::PrintErrorMsg(17);
        return pTemp;
    }
    else if (__isMatch("RETURNTK")){
        pTemp = __ReturnStmt();
        if (__isMatch("SEMICN"))
            CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
        else
            CError::PrintErrorMsg(17);
        return pTemp;
    }
    else if (__isMatch("LBRACE")){
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
        pTemp = __StmtList();
        if (!__isMatch("RBRACE"))
            CError::PrintErrorMsg(15);
        else
            CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
        return pTemp;
    }
    else{
        CLexicalAnalysis::GetNextSymbol(&sm_sCode, sm_sSymbol, sm_sValue, sTemp, 1);
        if (__isMatch("IDEN")&&sTemp=="LPARENT"){
            pTemp = __CallStmt();
            if (__isMatch("SEMICN"))
                CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
            else
                CError::PrintErrorMsg(17);
            return pTemp;
        }
        else if (sTemp=="ASSIGN"||sTemp=="LBRACK"){
            pTemp = __AssignStmt();
            if (__isMatch("SEMICN"))
                CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
            else
                CError::PrintErrorMsg(17);
            return pTemp;
        }
        else if (__isMatch("SEMICN"))
            CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
        else{
            CError::PrintErrorMsg(29);
            set<string> sStopSet;
            sStopSet.insert("SEMICN");
            sStopSet.insert("RBRACE");
            CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
            CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
        }
    }
    return NULL;
}

CASTNode* CParser::__ReturnStmt()
{
    __SetHasReturn(true);
    CASTNode* RETURNSTMTNode = new CASTNode(RETURNSTMT);
    string sType = sm_vSymbolTable[FindNameInAll(sm_sLocation)]->GetType();
    CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    if (__isMatch("LPARENT")){
        if (sType=="void")
            CError::PrintErrorMsg(11);
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
        RETURNSTMTNode->AddChild(__Expression());
        if (RETURNSTMTNode->ChildNum()==0)
            CError::PrintErrorMsg(12);
        if (__isMatch("RPARENT"))
            CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
        else
            CError::PrintErrorMsg(5);
    }
    else if (sType!="void")
        CError::PrintErrorMsg(12);
#ifdef DEBUG_PARSER
    cout << "<ReturnStmt>" << endl;
#endif
    return RETURNSTMTNode;
}

CASTNode* CParser::__ScanStmt()
{
    CASTNode* SCANSTMTNode = new CASTNode(SCANSTMT);
    CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    if (__isMatch("LPARENT"))
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    else
        CError::PrintErrorMsg(13);
    if (__isMatch("IDEN")){
        string sID = sm_sValue;
        int nIndex = FindNameInLocalAndGlobal(sID, sm_sLocation);
        if (nIndex==-1){
            CError::PrintErrorMsg(7, sID);
            __ErrorSymbolTableItem(sID, CAT_VAR, sm_sLocation);
            nIndex = FindNameInLocalAndGlobal(sID, sm_sLocation);
        }
        else{
            if (sm_vSymbolTable[nIndex]->GetCatagory()!=CAT_VAR&&sm_vSymbolTable[nIndex]->GetCatagory()!=CAT_PARAM)
                CError::PrintErrorMsg(25);
        }
        SCANSTMTNode->InsertSiblings(__Identifier(), 0);
    }
    else{
        CError::PrintErrorMsg(43);
        set<string> sStopSet;
        sStopSet.insert("COMMA");
        sStopSet.insert("RPARENT");
        sStopSet.insert("SEMICN");
        CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
    }
    while (__isMatch("COMMA")){
         CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
         if (__isMatch("IDEN"))
             SCANSTMTNode->InsertSiblings(__Identifier(), 0);
         else{
             CError::PrintErrorMsg(43);
             set<string> sStopSet;
             sStopSet.insert("COMMA");
             sStopSet.insert("RPARENT");
             sStopSet.insert("SEMICN");
             CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
         }
    }
    if (__isMatch("RPARENT"))
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    else
        CError::PrintErrorMsg(5);
#ifdef DEBUG_PARSER
    cout << "<ScanStmt>" << endl;
#endif
    return SCANSTMTNode;
}

CASTNode* CParser::__CondStmt()
{
    CASTNode* CONDSTMTNode = new CASTNode(CONDSTMT);
    CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    if (__isMatch("LPARENT"))
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    else{
        CError::PrintErrorMsg(13);
        CLexicalAnalysis::JumpRead(sm_sExpressionFIRST, &sm_sCode, sm_sSymbol, sm_sValue);
    }
    CONDSTMTNode->AddChild(__Condition());
    if (__isMatch("RPARENT"))
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    else{
        CError::PrintErrorMsg(5);
        CLexicalAnalysis::JumpRead(sm_sStatementFIRST, &sm_sCode, sm_sSymbol, sm_sValue);
    }
    CONDSTMTNode->AddChild(__Statement());
    if (__isMatch("ELSETK")){
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
        CONDSTMTNode->AddChild(__Statement());
    }
#ifdef DEBUG_PARSER
    cout << "<CondStmt>" << endl;
#endif
    return CONDSTMTNode;
}

CASTNode* CParser::__Condition()
{
    CASTNode* FirstExprNode = __Expression();
    CASTNode* RelOpNode = new CASTNode(OTHER);
    CASTNode* SecondExprNode = NULL;
    if (__isMatch("LSS")||__isMatch("LEQ")||__isMatch("GRE")||__isMatch("GEQ")||__isMatch("EQL")||__isMatch("NEQ")){
        if (__isMatch("LSS"))
            RelOpNode->SetNodeKind(LSS);
        else if (__isMatch("LEQ"))
            RelOpNode->SetNodeKind(LEQ);
        else if (__isMatch("GRE"))
            RelOpNode->SetNodeKind(GRE);
        else if (__isMatch("GEQ"))
            RelOpNode->SetNodeKind(GEQ);
        else if (__isMatch("EQL"))
            RelOpNode->SetNodeKind(EQL);
        else if (__isMatch("NEQ"))
            RelOpNode->SetNodeKind(NEQ);
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
        SecondExprNode = __Expression();
        RelOpNode->AddChild(FirstExprNode);
        RelOpNode->AddChild(SecondExprNode);
        return RelOpNode;
    }
    else if (__isMatch("PLUS")||__isMatch("MINU")||__isMatch("IDEN")||__isMatch("INTCON")||__isMatch("CHARCON")||__isMatch("LPARENT")){
        CError::PrintErrorMsg(44);
        RelOpNode->SetNodeKind(EQL);
        SecondExprNode = __Expression();
        RelOpNode->AddChild(FirstExprNode);
        RelOpNode->AddChild(SecondExprNode);
        return RelOpNode;
    }
    return FirstExprNode;
}

CASTNode* CParser::__WhileStmt()
{
    CASTNode* WHILESTMTNode = new CASTNode(WHILESTMT);
    CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    if (__isMatch("LPARENT"))
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    else{
        CError::PrintErrorMsg(13);
        CLexicalAnalysis::JumpRead(sm_sExpressionFIRST, &sm_sCode, sm_sSymbol, sm_sValue);
    }
    WHILESTMTNode->AddChild(__Condition());
    if (__isMatch("RPARENT"))
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    else{
        CError::PrintErrorMsg(5);
        CLexicalAnalysis::JumpRead(sm_sStatementFIRST, &sm_sCode, sm_sSymbol, sm_sValue);
    }
    WHILESTMTNode->AddChild(__Statement());
#ifdef DEBUG_PARSER
    cout << "<WhileStmt>" << endl;
#endif
    return WHILESTMTNode;
}

CASTNode* CParser::__PrintStmt()
{
    CASTNode* PRINTSTMTNode = new CASTNode(PRINTSTMT);
    CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    if (__isMatch("LPARENT"))
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    else{
        CError::PrintErrorMsg(13);
        set<string> sStopSet = sm_sExpressionFIRST;
        sStopSet.insert("STRCON");
        CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
    }
    if (!__isMatch("STRCON"))
        PRINTSTMTNode->AddChild(__Expression());
    else{
        PRINTSTMTNode->AddChild(new CASTNode(sm_sValue));
        PRINTSTMTNode->GetChild(0)->SetNodeKind(STRING);
        int nStringIndex = -1;
        for (int i=0;i<sm_vFuncIndex[0]->GetIndex();i++){
            if (sm_vSymbolTable[i]->GetConstValue()==PRINTSTMTNode->GetChild(0)->GetValue()){
                nStringIndex = i;
                break;
            }
        }
        if (nStringIndex==-1){
            string sTemp = CSymbolTableItem::GenStringIndex();
            CSymbolTableItem* iSymbolTableItem = new CSymbolTableItem(sTemp, CAT_STRING, "int", 0, PRINTSTMTNode->GetChild(0)->GetValue(), 0, 0, true);
            sm_vSymbolTable.insert(sm_vSymbolTable.begin() + sm_vFuncIndex[0]->GetIndex(), iSymbolTableItem);
            PRINTSTMTNode->GetChild(0)->SetQuadTemp(sTemp);
            for (unsigned int i=0;i<sm_vFuncIndex.size();i++)
                sm_vFuncIndex[i]->PlusIndex();
        }
        else
            PRINTSTMTNode->GetChild(0)->SetQuadTemp(sm_vSymbolTable[nStringIndex]->GetName());
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
        if (__isMatch("COMMA")){
            CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
            PRINTSTMTNode->AddChild(__Expression());
        }
        else{
            if(!__isMatch("RPARENT")){
                CError::PrintErrorMsg(45);
                set<string> sStopSet;
                sStopSet.insert("RPARENT");
                sStopSet.insert("SEMICN");
                CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
            }
        }
    }
    if (__isMatch("RPARENT"))
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    else{
        CError::PrintErrorMsg(5);
        set<string> sStopSet;
        sStopSet.insert("SEMICN");
        CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
    }
#ifdef DEBUG_PARSER
    cout << "<PrintStmt>" << endl;
#endif
    return PRINTSTMTNode;
}

CASTNode* CParser::__AssignStmt()
{
    CASTNode* ASSIGNSTMTNode = new CASTNode(ASSIGNSTMT);
    string sID = sm_sValue;
    int nIndex = FindNameInLocalAndGlobal(sID, sm_sLocation);
    if (nIndex==-1){
        CError::PrintErrorMsg(7, sID);
        __ErrorSymbolTableItem(sID, CAT_VAR, sm_sLocation);
        nIndex = FindNameInLocalAndGlobal(sID, sm_sLocation);
    }
    else{
        if (sm_vSymbolTable[nIndex]->GetCatagory()!=CAT_VAR&&sm_vSymbolTable[nIndex]->GetCatagory()!=CAT_ARRAY&&sm_vSymbolTable[nIndex]->GetCatagory()!=CAT_PARAM){
            CError::PrintErrorMsg(26);
            set<string> sStopSet;
            sStopSet.insert("SEMICN");
            CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
            return ASSIGNSTMTNode;
        }
    }
    ASSIGNSTMTNode->AddChild(__Identifier());
    ASSIGNSTMTNode->GetChild(0)->SetNodeKind(ASSIGN_LEFT_ID);
    if (__isMatch("ASSIGN")){
        if (sm_vSymbolTable[nIndex]->GetCatagory()!=CAT_VAR&&sm_vSymbolTable[nIndex]->GetCatagory()!=CAT_PARAM){
            CError::PrintErrorMsg(46, sID);
            set<string> sStopSet;
            sStopSet.insert("SEMICN");
            CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
            return ASSIGNSTMTNode;
        }
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
        ASSIGNSTMTNode->AddChild(__Expression());
    }
    else if (__isMatch("LBRACK")){
        if (sm_vSymbolTable[nIndex]->GetCatagory()!=CAT_ARRAY){
            CError::PrintErrorMsg(9, sID);
            set<string> sStopSet;
            sStopSet.insert("SEMICN");
            CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
            return ASSIGNSTMTNode;
        }
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
        CASTNode* ExprNode = __Expression();
        int nTemp = atoi(ExprNode->GetValue().c_str());
        if (nTemp>=sm_vSymbolTable[nIndex]->GetArraySize()||nTemp<0)
            CError::PrintErrorMsg(52);
        if (ExprNode->GetNodeKind() == MINU && ExprNode->ChildNum()==1 && ExprNode->GetChild(0)->GetNodeKind() == INTEGER)
            CError::PrintErrorMsg(52);
        ASSIGNSTMTNode->AddChild(ExprNode);
        if (!__isMatch("RBRACK"))
            CError::PrintErrorMsg(6);
        else{
            CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
            if (!__isMatch("ASSIGN")){
                CError::PrintErrorMsg(19);
                set<string> sStopSet;
                sStopSet.insert("SEMICN");
                CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
            }
            else{
                CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
                ASSIGNSTMTNode->AddChild(__Expression());
            }
        }
    }
    else{
        CError::PrintErrorMsg(23);
        set<string> sStopSet;
        sStopSet.insert("SEMICN");
        CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
    }
#ifdef DEBUG_PARSER
    cout << "<AssignStmt>" << endl;
#endif
    return ASSIGNSTMTNode;
}

CASTNode* CParser::__CallStmt()
{
    __SetLeafFunc(false);
    CASTNode* CALLSTMTNode = new CASTNode(CALLSTMT);
    string sID = sm_sValue;
    int nIndex = FindNameInAll(sID);
    if (nIndex==-1){
        CError::PrintErrorMsg(7, sID);
        set<string> sStopSet;
        sStopSet.insert("SEMICN");
        CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
        return CALLSTMTNode;
    }
    else{
        if (sm_vSymbolTable[nIndex]->GetCatagory()!=CAT_FUNC){
            CError::PrintErrorMsg(8, sID);
            set<string> sStopSet;
            sStopSet.insert("SEMICN");
            CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
            return CALLSTMTNode;
        }
    }
    CALLSTMTNode->AddChild(__Identifier());
    int nParaNum = 0;
    if (__isMatch("LPARENT"))
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    else{
        CError::PrintErrorMsg(13);
        set<string> sStopSet = sm_sExpressionFIRST;
        sStopSet.insert("RPARENT");
        CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
    }
    if (!__isMatch("RPARENT")){
        CALLSTMTNode->InsertSiblings(__Expression(), 1);
        nParaNum++;
        while (__isMatch("COMMA")){
            CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
            CALLSTMTNode->InsertSiblings(__Expression(), 1);
            nParaNum++;
        }
        if (__isMatch("RPARENT"))
            CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
        else{
            CError::PrintErrorMsg(5);
            set<string> sStopSet;
            sStopSet.insert("SEMICN");
            CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
        }
    }
    else
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    if (sm_vSymbolTable[nIndex]->GetFuncParamNum()!=nParaNum)
        CError::PrintErrorMsg(47);
#ifdef DEBUG_PARSER
    cout << "<CallStmt> " << CALLSTMTNode->GetChild(0)->GetValue() << endl;
#endif
    return CALLSTMTNode;
}

CASTNode* CParser::__ForStmt()
{
    CASTNode* FORSTMTNode = new CASTNode(FORSTMT);
    CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    if (__isMatch("LPARENT"))
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    else
        CError::PrintErrorMsg(13);
    if (__isMatch("IDEN")){
        string sID = sm_sValue;
        int nIndex = FindNameInLocalAndGlobal(sID, sm_sLocation);
        if (nIndex==-1){
            CError::PrintErrorMsg(7, sID);
            __ErrorSymbolTableItem(sID, CAT_VAR, sm_sLocation);
            nIndex = FindNameInLocalAndGlobal(sID, sm_sLocation);
        }
        else{
            if (sm_vSymbolTable[nIndex]->GetCatagory()!=CAT_VAR&&sm_vSymbolTable[nIndex]->GetCatagory()!=CAT_PARAM)
                CError::PrintErrorMsg(26);
        }
        FORSTMTNode->AddChild(__Identifier());
    }
    else{
        CError::PrintErrorMsg(48);
        set<string> sStopSet;
        sStopSet.insert("ASSIGN");
        sStopSet.insert("IDEN");
        sStopSet.insert("SEMICN");
        CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
    }
    if (__isMatch("ASSIGN"))
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    else{
        CError::PrintErrorMsg(48);
        set<string> sStopSet;
        sStopSet.insert("SEMICN");
        CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
    }
    FORSTMTNode->AddChild(__Expression());
    if (__isMatch("SEMICN"))
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    else{
        CError::PrintErrorMsg(17);
        set<string> sStopSet = sm_sExpressionFIRST;
        sStopSet.insert("SEMICN");
        CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
    }
    FORSTMTNode->AddChild(__Condition());
    if (__isMatch("SEMICN"))
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    else{
        CError::PrintErrorMsg(17);
        set<string> sStopSet;
        sStopSet.insert("IDEN");
        CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
    }
    if (!__isMatch("IDEN")){
        CError::PrintErrorMsg(48);
        set<string> sStopSet;
        sStopSet.insert("ASSIGN");
        sStopSet.insert("IDEN");
        CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
    }
    else{
        string sID = sm_sValue;
        int nIndex = FindNameInLocalAndGlobal(sID, sm_sLocation);
        if (nIndex==-1){
            CError::PrintErrorMsg(7, sID);
            __ErrorSymbolTableItem(sID, CAT_VAR, sm_sLocation);
            nIndex = FindNameInLocalAndGlobal(sID, sm_sLocation);
        }
        else{
            if (sm_vSymbolTable[nIndex]->GetCatagory()!=CAT_VAR&&sm_vSymbolTable[nIndex]->GetCatagory()!=CAT_PARAM)
                CError::PrintErrorMsg(26);
        }
        FORSTMTNode->InsertSiblings(__Identifier(), 0);
    }
    if (__isMatch("ASSIGN"))
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    else{
        CError::PrintErrorMsg(48);
        set<string> sStopSet;
        sStopSet.insert("IDEN");
        CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
    }
    if (!__isMatch("IDEN")){
        CError::PrintErrorMsg(48);
        set<string> sStopSet;
        sStopSet.insert("PLUS");
        sStopSet.insert("MINU");
        CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
    }
    else{
        string sID = sm_sValue;
        int nIndex = FindNameInLocalAndGlobal(sID, sm_sLocation);
        if (nIndex==-1){
            CError::PrintErrorMsg(7, sID);
            __ErrorSymbolTableItem(sID, CAT_VAR, sm_sLocation);
            nIndex = FindNameInLocalAndGlobal(sID, sm_sLocation);
        }
        else{
            if (sm_vSymbolTable[nIndex]->GetCatagory()!=CAT_VAR&&sm_vSymbolTable[nIndex]->GetCatagory()!=CAT_CONST&&sm_vSymbolTable[nIndex]->GetCatagory()!=CAT_PARAM)
                CError::PrintErrorMsg(27);
        }
        FORSTMTNode->InsertSiblings(__Identifier(), 0);
    }
    if (__isMatch("PLUS")){
        FORSTMTNode->AddChild(new CASTNode("+"));
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    }
    else if (__isMatch("MINU")){
        FORSTMTNode->AddChild(new CASTNode("-"));
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    }
    else
        CError::PrintErrorMsg(20);
    if (__isMatch("INTCON")){
        FORSTMTNode->AddChild(new CASTNode(sm_sValue));
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    }
    else{
        CError::PrintErrorMsg(28);
        set<string> sStopSet;
        sStopSet.insert("RPARENT");
        CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
    }
    if (__isMatch("RPARENT"))
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    else{
        CError::PrintErrorMsg(5);
        CLexicalAnalysis::JumpRead(sm_sStatementFIRST, &sm_sCode, sm_sSymbol, sm_sValue);
    }
    FORSTMTNode->AddChild(__Statement());
#ifdef DEBUG_PARSER
    cout << "<ForStmt>" << endl;
#endif
    return FORSTMTNode;
}

CASTNode* CParser::__Expression()
{
    CASTNode* ExprNode = new CASTNode(OTHER);
    bool isMINUS = false;
    if (__isMatch("MINU")){
        isMINUS = true;
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    }
    else if (__isMatch("PLUS"))
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
    CASTNode* LeftTermNode = __Term();
    CASTNode* FatherNode = NULL;
    while (__isMatch("PLUS")||__isMatch("MINU")){
        FatherNode = new CASTNode(OTHER);
        if (__isMatch("PLUS"))
            FatherNode->SetNodeKind(PLUS);
        else
            FatherNode->SetNodeKind(MINU);
        FatherNode->AddChild(LeftTermNode);
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
        FatherNode->AddChild(__Term());
        LeftTermNode = FatherNode;
    }
    if (isMINUS){
        ExprNode->SetNodeKind(MINU);
        ExprNode->AddChild(LeftTermNode);
    }
    else
        ExprNode = LeftTermNode;
#ifdef DEBUG_PARSER
    cout << "<Expression>" << endl;
#endif
    return ExprNode;
}

CASTNode* CParser::__Term()
{
    CASTNode* LeftFactorNode = __Factor();
    CASTNode* FatherNode = NULL;
    while (__isMatch("MULT")||__isMatch("DIV")){
        FatherNode = new CASTNode(OTHER);
        if (__isMatch("MULT"))
            FatherNode->SetNodeKind(MULT);
        else
            FatherNode->SetNodeKind(DIV);
        FatherNode->AddChild(LeftFactorNode);
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
        FatherNode->AddChild(__Factor());
        LeftFactorNode = FatherNode;
    }
    return LeftFactorNode;
}

CASTNode* CParser::__Factor()
{
    if (__isMatch("PLUS")||__isMatch("MINU")||__isMatch("INTCON"))
        return __Integer();
    else if (__isMatch("CHARCON")){
        string sChar = sm_sValue;
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
        CASTNode* pNode = new CASTNode(sChar);
        pNode->SetNodeKind(CHARACTER);
        return pNode;
    }
    else if (__isMatch("LPARENT")){
        CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
        CASTNode* ExprNode = __Expression();
        if (__isMatch("RPARENT"))
            CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
        else
            CError::PrintErrorMsg(5);
        return ExprNode;
    }
    else if (__isMatch("IDEN")){
        string sTemp = "";
        string sID = sm_sValue;
        CLexicalAnalysis::GetNextSymbol(&sm_sCode, sm_sSymbol, sm_sValue, sTemp, 1);
        if (sTemp=="LPARENT"){
            int nIndex = FindNameInAll(sID);
            if (nIndex==-1){
                CError::PrintErrorMsg(7, sID);
                CASTNode* IntNode = new CASTNode(INTEGER);
                IntNode->SetValue("0");
                return IntNode;
            }
            else if (sm_vSymbolTable[nIndex]->GetType()=="void"){
                CError::PrintErrorMsg(8, sID);
                CASTNode* IntNode = new CASTNode(INTEGER);
                IntNode->SetValue("0");
                return IntNode;
            }
            else if (sm_vSymbolTable[nIndex]->GetCatagory()!=CAT_FUNC){
                CError::PrintErrorMsg(9, sID);
                CASTNode* IntNode = new CASTNode(INTEGER);
                IntNode->SetValue("0");
                return IntNode;
            }
            else
                return __CallStmt();
        }
        else{
            CASTNode* IDNode = __Identifier();
            int nIndex = FindNameInLocalAndGlobal(IDNode->GetValue(), sm_sLocation);
            if (nIndex==-1){
                CError::PrintErrorMsg(7, IDNode->GetValue());
                __ErrorSymbolTableItem(IDNode->GetValue(), CAT_VAR, sm_sLocation);
                nIndex = FindNameInLocalAndGlobal(IDNode->GetValue(), sm_sLocation);
            }
            if (__isMatch("LBRACK")){
                if (sm_vSymbolTable[nIndex]->GetCatagory()!=CAT_ARRAY)
                    CError::PrintErrorMsg(9, IDNode->GetValue());
                CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
                CASTNode* ExprNode = __Expression();
                int nTemp = atoi(ExprNode->GetValue().c_str());
                if (nTemp>=sm_vSymbolTable[nIndex]->GetArraySize()||nTemp<0)
                    CError::PrintErrorMsg(52);
                if (ExprNode->GetNodeKind() == MINU && ExprNode->ChildNum()==1 && ExprNode->GetChild(0)->GetNodeKind() == INTEGER)
                    CError::PrintErrorMsg(52);
                if (!__isMatch("RBRACK"))
                    CError::PrintErrorMsg(6);
                else
                    CLexicalAnalysis::GetSymbol(&sm_sCode, sm_sSymbol, sm_sValue);
                IDNode->InsertSiblings(ExprNode, 0);
            }
            else if (sm_vSymbolTable[nIndex]->GetCatagory()==CAT_ARRAY)
                CError::PrintErrorMsg(10);
            return IDNode;
        }
    }
    else{
        CError::PrintErrorMsg(49);
        set<string> sStopSet;
        sStopSet.insert("SEMICN");
        sStopSet.insert("RPARENT");
        sStopSet.insert("RBRACK");
        CLexicalAnalysis::JumpRead(sStopSet, &sm_sCode, sm_sSymbol, sm_sValue);
    }
    return NULL;
}

void CParser::__FillOffset()
{
    int nOffset = -8;
    vector<CQuadruple*> vQuads = CTraverser::GetQuads();
    string sFuncName = sm_vFuncIndex[0]->GetFuncName();
    for (unsigned int i=0;i<sm_vFuncIndex.size();i++){
        string sFuncName = sm_vFuncIndex[i]->GetFuncName();
        nOffset = -8;
        for (unsigned int j = sm_vFuncIndex[i]->GetIndex()+1;j<sm_vSymbolTable.size()&&sm_vSymbolTable[j]->GetCatagory()!=CAT_FUNC;j++){
            if (sm_vSymbolTable[j]->GetCatagory()==CAT_CONST||sm_vSymbolTable[j]->GetCatagory()==CAT_VAR){
                sm_vSymbolTable[j]->SetOffset(nOffset);
                nOffset -= 4;
            }
            else if (sm_vSymbolTable[j]->GetCatagory()==CAT_ARRAY){
                sm_vSymbolTable[j]->SetOffset(nOffset);
                nOffset -= (sm_vSymbolTable[j]->GetArraySize() * 4);
            }
        }

        int k = 0;
        int nTempVarNum = 0;
        if (sFuncName!="main"){
            while (vQuads[k]->GetOp()!=FUNC_LABEL_PREFIX + sFuncName)
                k++;
        }
        else{
            while (vQuads[k]->GetOp()!="main")
                k++;
        }
        k++;
        while(k<(int)(vQuads.size())&&vQuads[k]->GetQuadType()!=QUAD_TYPE_FUNCLABEL){
            vQuads[k]->SetFuncIndex(i);
            string sTemp = vQuads[k]->GetArg1();
            if (CQuadruple::isTemp(sTemp)&&!sm_vFuncIndex[i]->isInTempVarTable(sTemp)){
                sm_vFuncIndex[i]->InsertTempVarTable(new CSymbolTableItem(sTemp, CAT_VAR, "int", nTempVarNum * (-4) + nOffset, "0", 0, 0, true));
                nTempVarNum++;
            }
            sTemp = vQuads[k]->GetArg2();
            if (CQuadruple::isTemp(sTemp)&&!sm_vFuncIndex[i]->isInTempVarTable(sTemp)){
                sm_vFuncIndex[i]->InsertTempVarTable(new CSymbolTableItem(sTemp, CAT_VAR, "int", nTempVarNum * (-4) + nOffset, "0", 0, 0, true));
                nTempVarNum++;
            }
            sTemp = vQuads[k]->GetResult();
            if (CQuadruple::isTemp(sTemp)&&!sm_vFuncIndex[i]->isInTempVarTable(sTemp)){
                sm_vFuncIndex[i]->InsertTempVarTable(new CSymbolTableItem(sTemp, CAT_VAR, "int", nTempVarNum * (-4) + nOffset, "0", 0, 0, true));
                nTempVarNum++;
            }
            k++;
        }

    }
}

void CParser::__ErrorSymbolTableItem(string sName, Catagory eCatagory, string sLocation)
{
    CSymbolTableItem* iSymbolTableItem = new CSymbolTableItem(sName, eCatagory, "int", 0, "0", 0, 0, true);
    int nIndex;
    for (unsigned int i=0;i<sm_vFuncIndex.size();i++){
        if (sm_vFuncIndex[i]->GetFuncName()==sLocation)
            nIndex = sm_vFuncIndex[i]->GetIndex();
    }
    nIndex++;
    if (eCatagory==CAT_CONST)
        sm_vSymbolTable.insert(sm_vSymbolTable.begin() + nIndex, iSymbolTableItem);
    else{
        while (nIndex<(int)(sm_vSymbolTable.size())&&sm_vSymbolTable[nIndex]->GetCatagory()==CAT_CONST)
            nIndex++;
        sm_vSymbolTable.insert(sm_vSymbolTable.begin() + nIndex, iSymbolTableItem);
    }
    for (unsigned int i=0;i<sm_vFuncIndex.size();i++){
        if (sm_vFuncIndex[i]->GetIndex()>=nIndex)
            sm_vFuncIndex[i]->PlusIndex();
    }
}

vector<CSymbolTableItem*> CParser::GetSymbolTable() { return sm_vSymbolTable; }

vector<CFuncIndex*> CParser::GetFuncIndex() { return sm_vFuncIndex; }

int CParser::FindNameInLocalAndGlobal(string sName, string sLocation)
{
    for (unsigned int i=0;i<sm_vFuncIndex.size();i++){
        if (sm_vFuncIndex[i]->GetFuncName()==sLocation){
            int nStart = sm_vFuncIndex[i]->GetIndex();
            int nEnd = (i==sm_vFuncIndex.size()-1)?(sm_vSymbolTable.size()-1):(sm_vFuncIndex[i+1]->GetIndex()-1);
            for (int j=nStart;j<=nEnd;j++){
                if (sm_vSymbolTable[j]->GetName()==sName)
                    return j;
            }
        }
    }
    if (sm_vSymbolTable.size()<=0)
        return -1;
    int nEnd = (sm_vFuncIndex.size()==0)?(sm_vSymbolTable.size()-1):(sm_vFuncIndex[0]->GetIndex()-1);
    for (int j=0;j<=nEnd;j++){
        if (sm_vSymbolTable[j]->GetName()==sName)
            return j;
    }
    return -1;
}

int CParser::FindNameInLocal(string sName, string sLocation)
{
    for (unsigned int i=0;i<sm_vFuncIndex.size();i++){
        if (sm_vFuncIndex[i]->GetFuncName()==sLocation){
            int nStart = sm_vFuncIndex[i]->GetIndex();
            int nEnd = (i==sm_vFuncIndex.size()-1)?(sm_vSymbolTable.size()-1):(sm_vFuncIndex[i+1]->GetIndex()-1);
            for (int j=nStart;j<=nEnd;j++){
                if (sm_vSymbolTable[j]->GetName()==sName)
                    return j;
            }
        }
    }
    return -1;
}

int CParser::FindNameInAll(string sName)
{
    for (unsigned int j=0;j<sm_vSymbolTable.size();j++){
        if (sm_vSymbolTable[j]->GetName()==sName)
            return j;
    }
    return -1;
}

int CParser::FindNameInGlobal(string sName)
{
    if (sm_vSymbolTable.size()<=0)
        return -1;
    int nEnd = (sm_vFuncIndex.size()==0)?(sm_vSymbolTable.size()-1):(sm_vFuncIndex[0]->GetIndex()-1);
    for (int j=0;j<=nEnd;j++){
        if (sm_vSymbolTable[j]->GetName()==sName)
            return j;
    }
    return -1;
}

#ifdef DEBUG_SYMBOL_TABLE
void CParser::PrintSymbolTable(ofstream &fout)
{
    fout << "Name\t\tCatagory\tType\tConstValue\tOffset\tArraySize\tFuncParamNum\tLeafFunc" << endl;
    for (unsigned int i=0;i<sm_vSymbolTable.size();i++)
        sm_vSymbolTable[i]->PrintSymbolTableItem(fout);
}
#endif