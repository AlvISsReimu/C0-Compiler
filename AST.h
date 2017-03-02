#ifndef AST_H
#define AST_H

#define MAX_CHILD 6

#include <iostream>
#include <string>
#include "LexicalAnalysis.h"

using namespace std;

typedef enum NodeKind{
    PLUS, MINU, MULT, DIV,
    LSS, LEQ, GRE, GEQ, EQL, NEQ,
    PARATABLE,
    CONSTDEC, CONSTDEF, VARDEC, VARDEF, FUNCDEF,
    COMPSTMT, CONDSTMT, WHILESTMT, FORSTMT, CALLSTMT, ASSIGNSTMT, SCANSTMT, PRINTSTMT, RETURNSTMT, STMTLIST,
    PROGRAM,
    OTHER,
    TYPE, IDENTIFIER, INTEGER, ASSIGN_LEFT_ID, STRING, CHARACTER
};

#ifdef DEBUG_TEST_TREE
const char * const NodeKindString[] = {
    "PLUS", "MINU", "MULT", "DIV",
    "LSS", "LEQ", "GRE", "GEQ", "EQL", "NEQ",
    "PARATABLE",
    "CONSTDEC", "CONSTDEF", "VARDEC", "VARDEF", "FUNCDEF",
    "COMPSTMT", "CONDSTMT", "WHILESTMT", "FORSTMT", "CALLSTMT", "ASSIGNSTMT", "SCANSTMT", "PRINTSTMT", "RETURNSTMT", "STMTLIST",
    "PROGRAM",
    "OTHER",
    "TYPE", "IDENTIFIER", "INTEGER", "ASSIGN_LEFT_ID", "STRING", "CHARACTER"
};
#endif

class CASTNode
{
    NodeKind m_eNodeKind;
    CASTNode* m_pChild[MAX_CHILD];
    CASTNode* m_pSibling;
    string m_sValue;
    string m_sQuadTemp;
    int m_iLine;

public:
    CASTNode(NodeKind eNodeKind);
    CASTNode(string sValue);
    ~CASTNode();

    NodeKind GetNodeKind();
    CASTNode* GetChild(int iIndex);
    CASTNode* GetSibling();
    string GetValue();
    string GetQuadTemp();
    int GetLine();

    void SetNodeKind(NodeKind eNodeKind);
    void AddChild(CASTNode* pChild);
    void SetSibling(CASTNode* pSibling);
    void SetValue(string sValue);
    void SetQuadTemp(string sQuadTemp);
    void SetLine(int iLine);

    void InsertSiblings(CASTNode* pSibling, int iChildIndex);
    int ChildNum();
    int SiblingNum(int iIndex);

#ifdef DEBUG_TEST_TREE
    static void TestTree(CASTNode* pAST, int iLayer, ofstream &fout);
    static void PrintTab(int iNum, ofstream &fout);
#endif
};

#endif