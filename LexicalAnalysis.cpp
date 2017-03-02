#include "LexicalAnalysis.h"

int CLexicalAnalysis::sm_iCharPointer = 0;
string CLexicalAnalysis::sm_sWord = "";
int CLexicalAnalysis::sm_iNumber = 0;
map<string, string> CLexicalAnalysis::sm_mReservedWord = map<string, string>();
int CLexicalAnalysis::sm_iColNumber = 1;
int CLexicalAnalysis::sm_iLineNumber = 1;

CLexicalAnalysis::CLexicalAnalysis() {}

CLexicalAnalysis::~CLexicalAnalysis() {}

void CLexicalAnalysis::GetSymbol(const string* sCode, string& sSymbol, string& sValue)
{
    sSymbol = "";
    sValue = "";
    if(!isEOF(sCode)){
        __InitMap();
        __SetWord("");
        string sPresentChar = __PresentChar(sCode);
        while ((sPresentChar==" ")||(sPresentChar=="\n")||(sPresentChar=="\t")){
            if (sPresentChar == "\n"){
                SetLineNumber(LineNumber() + 1);
#ifdef DEBUG_LINE_NUMBER
                cout << "\n[Line " << LineNumber() << "]" << endl;
#endif
                SetColNumber(0);
            }
            __AddCharPointer();
            sPresentChar = __PresentChar(sCode);
        }
        if (__isLetter(sPresentChar)){
            while (__isLetter(sPresentChar)||__isDigit(sPresentChar)){
                sm_sWord += sPresentChar;
                __AddCharPointer();
                sPresentChar = __PresentChar(sCode);
            }
            __DecCharPointer();
            string sTempString = sm_sWord;
            __Lowercase(sm_sWord);
            map<string, string>::iterator it = sm_mReservedWord.find(sm_sWord);
            sSymbol = (it == sm_mReservedWord.end()) ? "IDEN" : sm_mReservedWord[sm_sWord];
            sValue = sm_sWord;
        }
        else if (__isDigit(sPresentChar)){
            if (sPresentChar=="0")
                sValue = "0";
            else{
                while (__isDigit(sPresentChar)){
                    sm_sWord += sPresentChar;
                    __AddCharPointer();
                    sPresentChar = __PresentChar(sCode);
                }
                __DecCharPointer();
                sValue = sm_sWord;
            }
            sSymbol = "INTCON";
        }
        else if (sPresentChar == "+"){
            sSymbol = "PLUS";
            sValue = "+";
        }
        else if (sPresentChar == "-"){
            sSymbol = "MINU";
            sValue = "-";
        }
        else if (sPresentChar == "*"){
            sSymbol = "MULT";
            sValue = "*";
        }
        else if (sPresentChar == "/"){
            sSymbol = "DIV";
            sValue = "/";
        }
        else if (sPresentChar == "<"){
            __AddCharPointer();
            sPresentChar = __PresentChar(sCode);
            if (sPresentChar == "="){
                sSymbol = "LEQ";
                sValue = "<=";
            }
            else{
                __DecCharPointer();
                sSymbol = "LSS";
                sValue = "<";
            }
        }
        else if (sPresentChar == ">"){
            __AddCharPointer();
            sPresentChar = __PresentChar(sCode);
            if (sPresentChar == "="){
                sSymbol = "GEQ";
                sValue = ">=";
            }
            else{
                __DecCharPointer();
                sSymbol = "GRE";
                sValue = ">";
            }
        }
        else if (sPresentChar == "="){
            __AddCharPointer();
            sPresentChar = __PresentChar(sCode);
            if (sPresentChar == "="){
                sSymbol = "EQL";
                sValue = "==";
            }
            else{
                __DecCharPointer();
                sSymbol = "ASSIGN";
                sValue = "=";
            }
        }
        else if (sPresentChar == "!"){
            __AddCharPointer();
            sPresentChar = __PresentChar(sCode);
            if (sPresentChar != "="){
                __DecCharPointer();
                CError::PrintErrorMsg(0);
            }
            sSymbol = "NEQ";
            sValue = "!=";
        }
        else if (sPresentChar == ";"){
            sSymbol = "SEMICN";
            sValue = ";";
        }
        else if (sPresentChar == ","){
            sSymbol = "COMMA";
            sValue = ",";
        }
        else if (sPresentChar == "("){
            sSymbol = "LPARENT";
            sValue = "(";
        }
        else if (sPresentChar == ")"){
            sSymbol = "RPARENT";
            sValue = ")";
        }
        else if (sPresentChar == "["){
            sSymbol = "LBRACK";
            sValue = "[";
        }
        else if (sPresentChar == "]"){
            sSymbol = "RBRACK";
            sValue = "]";
        }
        else if (sPresentChar == "{"){
            sSymbol = "LBRACE";
            sValue = "{";
        }
        else if (sPresentChar == "}"){
            sSymbol = "RBRACE";
            sValue = "}";
        }
        else if (sPresentChar == "'"){
            __AddCharPointer();
            sPresentChar = __PresentChar(sCode);
            if (__isLetter(sPresentChar)||__isDigit(sPresentChar)
                ||sPresentChar=="+"||sPresentChar=="-"||
                sPresentChar=="*"||sPresentChar=="/"){
                    sSymbol = "CHARCON";
                    sValue = sPresentChar;
                    __AddCharPointer();
                    sPresentChar = __PresentChar(sCode);
            }
            else{
                CError::PrintErrorMsg(2);
                sSymbol = "CHARCON";
                sValue = ' ';
                __AddCharPointer();
                sPresentChar = __PresentChar(sCode);
            }
            if (sPresentChar != "'"){
                CError::PrintErrorMsg(1);
                __DecCharPointer();
            }
        }
        else if (sPresentChar == "\""){
            do{
                __AddCharPointer();
                sPresentChar = __PresentChar(sCode);
                if ((sPresentChar != "\"")&&(sPresentChar != "\n")&&(!isEOF(sCode))){
                    if ((sPresentChar >= " ")&&(sPresentChar <= "~"))
                        sm_sWord += sPresentChar;
                    else
                        CError::PrintErrorMsg(3);
                }
            }while ((sPresentChar != "\"")&&(sPresentChar != "\n")&&(!isEOF(sCode)));
            if (sPresentChar=="\""){
                sSymbol = "STRCON";
                sValue = sm_sWord;
            }
            else if (sPresentChar=="\n"||isEOF(sCode)){
                sSymbol = "STRCON";
                sValue = sm_sWord;
                CError::PrintErrorMsg(4);
                __DecCharPointer();
            }
        }
        else if (sPresentChar == "")
            sSymbol = "EOF";
        __AddCharPointer();
    }
#ifdef DEBUG_LEXICALANALYSIS
    cout << sSymbol  <<  "\t" << sValue << endl;
#endif
}

void CLexicalAnalysis::__Insert2Map(string sName, string sMNCode)
{
    sm_mReservedWord.insert(map<string,string>::value_type(sName, sMNCode));
}

void CLexicalAnalysis::__InitMap()
{
    __Insert2Map("const","CONSTTK");
    __Insert2Map("int","INTTK");
    __Insert2Map("char","CHARTK");
    __Insert2Map("void","VOIDTK");
    __Insert2Map("main","MAINTK");
    __Insert2Map("if","IFTK");
    __Insert2Map("then","THENTK");
    __Insert2Map("else","ELSETK");
    __Insert2Map("while","WHILETK");
    __Insert2Map("for","FORTK");
    __Insert2Map("scanf","SCANFTK");
    __Insert2Map("printf","PRINTFTK");
    __Insert2Map("return","RETURNTK");
}

string CLexicalAnalysis::__Word() {return sm_sWord;}

void CLexicalAnalysis::__SetWord(string sWord) {sm_sWord = sWord;}

int CLexicalAnalysis::__Number() {return sm_iNumber;}

int CLexicalAnalysis::__CharPointer() {return sm_iCharPointer;}

string CLexicalAnalysis::__PresentChar(const string* sCode)
{
    return ((__CharPointer()>=0)&&((unsigned int)__CharPointer()<=(*sCode).length()-1))?((*sCode).substr(__CharPointer(), 1)):("");
}

void CLexicalAnalysis::__AddCharPointer()
{
    sm_iCharPointer++;
    SetColNumber(ColNumber() + 1);
}

void CLexicalAnalysis::__DecCharPointer()
{
    sm_iCharPointer--;
    SetColNumber(ColNumber() - 1);
}

bool CLexicalAnalysis::__isLetter(string sChar) {return ((sChar>="a")&&(sChar<="z"))||((sChar>="A")&&(sChar<="Z"))||(sChar=="_");}

bool CLexicalAnalysis::__isDigit(string sChar) {return (sChar>="0")&&(sChar<="9");}

void CLexicalAnalysis::__Lowercase(string& sString)
{
    for(unsigned int i = 0; i < sString.size(); i++){
        if( sString[i] >= 'A' && sString[i] <= 'Z' )
            sString[i] -= 'A' - 'a';
    }
}

bool CLexicalAnalysis::isEOF(const string* sCode) {return ((unsigned int)sm_iCharPointer >= (*sCode).length()); }

int CLexicalAnalysis::LineNumber() {return sm_iLineNumber;}

int CLexicalAnalysis::ColNumber() {return sm_iColNumber;}

int CLexicalAnalysis::CharPointer() {return sm_iCharPointer;}

string CLexicalAnalysis::Word() {return sm_sWord;}

int CLexicalAnalysis::Number() {return sm_iNumber;}

void CLexicalAnalysis::SetLineNumber(int iLineNumber) {sm_iLineNumber = iLineNumber;}

void CLexicalAnalysis::SetColNumber(int iColNumber) {sm_iColNumber = iColNumber;}

void CLexicalAnalysis::SetCharPointer(int iCharPointer) {sm_iCharPointer = iCharPointer;}

void CLexicalAnalysis::SetWord(string sWord) {sm_sWord = sWord;}

void CLexicalAnalysis::SetNumber(int iNumber) {sm_iNumber = iNumber;}

void CLexicalAnalysis::GetNextSymbol(const string* sCode, string& sSymbol, string& sValue, string& sNextSymbol, int iTimes)
{
    string sTemp1 = "", sTemp2 = "";
    int iCharPointer = CharPointer();
    string sWord = Word();
    int iNumber = Number();
    int iLineNumber = LineNumber();
    int iColNumber = ColNumber();
    for (int i=0;i<iTimes;i++)
        GetSymbol(sCode, sTemp1, sTemp2);
    SetCharPointer(iCharPointer);
    SetWord(sWord);
    SetNumber(iNumber);
    SetLineNumber(iLineNumber);
    SetColNumber(iColNumber);
    sNextSymbol = sTemp1;
}

void CLexicalAnalysis::JumpRead(set<string> sStopSet, const string* sCode, string& sSymbol, string& sValue)
{
    while(sStopSet.find(sSymbol)==sStopSet.end()){
        GetSymbol(sCode, sSymbol, sValue);
        if (sSymbol=="")
            break;
    }
}