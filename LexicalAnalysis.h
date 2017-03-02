#ifndef LEXICALANALYSIS_H
#define LEXICALANALYSIS_H

#include <iostream>
#include <string>
#include <map>
#include <set>
#include "Error.h"
#include "Debug.h"

using namespace std;

class CLexicalAnalysis
{
    static int sm_iCharPointer;
    static string sm_sWord;
    static int sm_iNumber;
    static map<string, string> sm_mReservedWord;
    static int sm_iLineNumber;
    static int sm_iColNumber;

    static void __InitMap();
    static void __Insert2Map(string sName, string sMNCode);
    static void __SetWord(string sWord);
    static string __Word();
    static int __Number();
    static int __CharPointer();
    static string __PresentChar(const string* sCode);
    static void __AddCharPointer();
    static void __DecCharPointer();
    static bool __isLetter(string sChar);
    static bool __isDigit(string sChar);
    static void __Lowercase(string& sString);

public:
    CLexicalAnalysis();
    ~CLexicalAnalysis();
    static void GetSymbol(const string* sCode, string& sSymbol, string& sValue);
    static bool isEOF(const string* sCode);
    static int LineNumber();
    static int ColNumber();
    static int CharPointer();
    static string Word();
    static int Number();
    static void SetLineNumber(int iLineNumber);
    static void SetColNumber(int iColNumber);
    static void SetCharPointer(int iCharPointer);
    static void SetWord(string sWord);
    static void SetNumber(int iNumber);
    static void GetNextSymbol(const string* sCode, string& sSymbol, string& sValue, string& sNextSymbol, int iTimes);
    static void JumpRead(set<string> sStopSet, const string* sCode, string& sSymbol, string& sValue);
};

#endif