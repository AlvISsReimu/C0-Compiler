#ifndef BASICBLOCK_H
#define BASICBLOCK_H

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <set>
#include <algorithm>
#include <iterator>
#include "Quadruple.h"
#ifdef DEBUG_OPTIMIZE
#include "DAG.h"
#endif

using namespace std;

#define BLOCK_TAG_UNDEFINED 0
#define BLOCK_TAG_IN 1

class CBasicBlock
{
    string m_sBlockName;
    string m_sLabel;
    vector<CQuadruple*> m_vQuads;
    vector<CBasicBlock*> m_vSuccessor;
#ifdef DEBUG_OPTIMIZE
    set<string> m_sUseSet;
    set<string> m_sDefSet;
    vector<string> m_vInSet;
    vector<string> m_vOutSet;
#endif

    static int sm_nBlockNo;
    static vector<CBasicBlock*> sm_vBasicBlocks;
#ifdef DEBUG_OPTIMIZE
    void __GenUseDefSet();
#endif
    static CBasicBlock* __GenStartBlock(string sLabel);
    static CBasicBlock* __GenEndBlock();
    static void __ResetBlockNo();

#ifdef DEBUG_OPTIMIZE
    static string __Calc(string sArg1, string sOp, string sArg2);
    static bool __GenInOutSet(vector<CBasicBlock*> vBasicBlocks);
#endif

public:
    CBasicBlock(string sLabel);
    CBasicBlock(string sBlockName, string sLabel);
    ~CBasicBlock();
    string GetBlockName();
    string GetLabel();
    vector<CQuadruple*> GetQuads();
#ifdef DEBUG_OPTIMIZE
    vector<string> GetInSet();
    CDAG GenDAG();
#endif
    void AddSuccessor(CBasicBlock* pBasicBlock);
    void SetQuads(vector<CQuadruple*> vQuads);
    
    static vector<vector<CBasicBlock*>> GenBasicBlocks(vector<CQuadruple*> vQuads);
};

#endif