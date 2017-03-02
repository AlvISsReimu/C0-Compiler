#include "main.h"

int main()
{
#ifdef DEBUG_USE_CODE_TXT
    string sPath = "code.txt";
#else
    string sPath;
    cout << "Please enter the path of the file: ";
    getline(cin, sPath);
#endif
    ifstream fin(sPath);
    if (!fin)
        cout << "Cannot find the file." << endl;
    else{
        string sCode = "";
        char c;
        while ((c = fin.get()) != EOF)
            sCode += c;
        fin.close();
        if (!CLexicalAnalysis::isEOF(&sCode))
            CParser::Parser(&sCode);
        if (CError::GetErrorSum()!=0)
            cout << "Error num: " << CError::GetErrorSum() << endl;
        else{
            vector<vector<CBasicBlock*>> vBasicBlocks = CBasicBlock::GenBasicBlocks(CTraverser::GetQuads());
            //CDAG vDAG = vBasicBlocks[0][5]->GenDAG();
            ofstream fout_MIPS("mips.asm");
#ifdef DEBUG_OPTIMIZE
            CCodeGeneration::GenCode(fout_MIPS, vBasicBlocks, CParser::GetSymbolTable(), CParser::GetFuncIndex());
#else
            CCodeGeneration::GenCode(fout_MIPS, CTraverser::GetQuads(), CParser::GetSymbolTable(), CParser::GetFuncIndex());
#endif
            fout_MIPS.close();
        }
    }
    return 0;
}