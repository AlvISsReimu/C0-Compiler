#ifndef ERROR_H
#define ERROR_H

#include <iostream>
#include <iomanip>

using namespace std;

#define ERROR_TYPE_SUM 100

class CError
{
    static int sm_nErrorSum;
    static string sm_gsErrorInfo[ERROR_TYPE_SUM];

public:
    CError(void);
    ~CError(void);
    static void PrintErrorMsg(int iErrorNo, string sExtra = "");
    static int GetErrorSum();
};

#endif