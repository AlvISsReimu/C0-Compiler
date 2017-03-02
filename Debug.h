#ifndef DEBUG_H
#define DEBUG_H

//#define DEBUG_USE_CODE_TXT
//#define DEBUG_LEXICALANALYSIS
//#define DEBUG_PARSER
//#define DEBUG_LINE_NUMBER
//#define DEBUG_GEN_DROP_TEMP
//#define DEBUG_TEST_TREE
//#define DEBUG_TEST_QUAD
//#define DEBUG_SYMBOL_TABLE
//#define DEBUG_GENCODE_PRINTQUAD
#define DEBUG_OPTIMIZE

#ifdef DEBUG_TEST_TREE
#include <fstream>
#endif

#ifdef DEBUG_TEST_QUAD
#include <fstream>
#endif

#ifdef DEBUG_SYMBOL_TABLE
#include <fstream>
#endif

#endif