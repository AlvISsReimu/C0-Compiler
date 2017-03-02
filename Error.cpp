#include "Error.h"
#include "LexicalAnalysis.h"

int CError::sm_nErrorSum = 0;
string CError::sm_gsErrorInfo[ERROR_TYPE_SUM] = {
    "'!' operator has no effect; did you intend '!='?",             /* 0 */
    "missing single quotation after a char",                        /* 1 */
    "char must only be '+', '-', '*', '/', digit or letter",        /* 2 */
    "string must use characters among ASCII 32,33,35-126",          /* 3 */
    "missing double quotation after a string",                      /* 4 */
    "missing ')'",                                                  /* 5 */
    "missing ']'",                                                  /* 6 */
    "undefined identifier: ",                                       /* 7 */
    "the identifier is not a name of function: ",                   /* 8 */
    "the identifier is not an array: ",                             /* 9 */
    "missing index of array:  ",                                    /* 10 */
    "void function cannot return a value",                          /* 11 */
    "non-void function must return a value",                        /* 12 */
    "missing '('",                                                  /* 13 */
    "missing '{'",                                                  /* 14 */
    "missing '}'",                                                  /* 15 */
    "program should end",                                           /* 16 */
    "missing ';'",                                                  /* 17 */
    "multiply defined symbol found: ",                              /* 18 */
    "missing '='",                                                  /* 19 */
    "missing '+' or '-'",                                           /* 20 */
    "a program must contain main function",                         /* 21 */
    "missing \"void\"",                                             /* 22 */
    "incomplete assign statement",                                  /* 23 */
    "'+|-' should not be put before 0.",                            /* 24 */
    "identifier in scanf must be simple variable",                  /* 25 */
    "expression must be a modifiable lvalue",                       /* 26 */
    "expression's rvalue must be variable or constant",             /* 27 */
    "a positive integer is required",                               /* 28 */
    "invalid statement",                                            /* 29 */
    "identifier is required in const definition",                   /* 30 */
    "incomplete in const definition",                               /* 31 */
    "invalid initial const value: ",                                /* 32 */
    "const definition must be before var definition",               /* 33 */
    "incomplete var definition",                                    /* 34 */
    "array's length must greater than 0",                           /* 35 */
    "invalid array's index: ",                                      /* 36 */
    "main function must be void",                                   /* 37 */
    "main function cannot be with params",                          /* 38 */
    "function needs a name",                                        /* 39 */
    "function should return a value",                               /* 40 */
    "missing type identifier (int|char)",                           /* 41 */
    "missing param name",                                           /* 42 */
    "params in scanf statement must be simple variable",            /* 43 */
    "missing relation operator",                                    /* 44 */
    "missing ','",                                                  /* 45 */
    "identifier here must be simple variable: ",                    /* 46 */
    "mismatched number of parameters",                              /* 47 */
    "incomplete for statement",                                     /* 48 */
    "invalid expression",                                           /* 49 */
    "const cannot be used here",                                    /* 50 */
    "definition is misplaced",                                      /* 51 */
    "array index out of bounds",                                    /* 52 */
};

CError::CError(void) {}

CError::~CError(void) {}

void CError::PrintErrorMsg(int iErrorNo, string sExtra)
{
    sm_nErrorSum++;
    if (sm_nErrorSum==1)
        cout << "    Line  Col  No.  Error Info" << endl;
    cout << left << setw(4) << sm_nErrorSum
        << left << setw(6) << CLexicalAnalysis::LineNumber()
        << left << setw(5) << CLexicalAnalysis::ColNumber()
        << left << setw(5) << iErrorNo
        << sm_gsErrorInfo[iErrorNo] << sExtra << endl;
}

int CError::GetErrorSum() { return sm_nErrorSum; }