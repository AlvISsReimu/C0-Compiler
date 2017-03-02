#ifndef TRAVERSER_H
#define TRAVERSER_H

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include "AST.h"
#include "Quadruple.h"
#include "Parser.h"

using namespace std;

#define MAX_LABEL_STACK 102400
#define TYPE_SIZE 4

class CLabelStack
{
    string m_gLabelStack[MAX_LABEL_STACK];
    int m_iStackTop;

public:
    CLabelStack();
    int GetSize();
    bool isStackEmpty();
    bool isStackFull();
    string GetElement(int iIndex);
    bool Push(string sLabel);
    string Pop();
};

typedef vector<CQuadruple*>::iterator QuadsIterator;

class CTraverser
{
    static CLabelStack sm_iLabelStack;
    static vector<CQuadruple*> sm_vQuads;
    static string sm_sLocation;

    static void __GenCalcQuads(CASTNode* pTreeNode);
    static void __GenCondStmtQuads(CASTNode* pTreeNode);
    static void __GenWhileStmtQuads(CASTNode* pTreeNode);
    static void __GenForStmtQuads(CASTNode* pTreeNode);
    static void __GenCondQuads(CASTNode* pTreeNode, string sLable);
    static void __GenAssignQuads(CASTNode* pTreeNode);
    static void __GenReturnQuads(CASTNode* pTreeNode);
    static void __GenFuncDefQuads(CASTNode* pTreeNode);
    static void __GenPrintStmtQuads(CASTNode* pTreeNode);
    static void __GenScanStmtQuads(CASTNode* pTreeNode);
    static void __GenCallStmtQuads(CASTNode* pTreeNode);

    static int __CountSiblings(CASTNode* pNode);
    static void __SetLocation(string sLocation);
    static string __GetLocation();

public:
    CTraverser();
    ~CTraverser();
    static void ASTTraversal(CASTNode* pTreeNode);
    static vector<CQuadruple*> GetQuads();
    static void OptimizeLabels();

#ifdef DEBUG_TEST_QUAD
    static void PrintQuads(ofstream &fout);
#endif
};

#endif