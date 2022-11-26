#ifndef DBTYPE_H
#define DBTYPE_H
#include <cstddef>
#include <string.h>
#include "Optrs.h"

template<size_t s>
class SString {
public:
    char msg[s];
};

enum dbType {
    DB_INT = 0,
    DB_DOUBLE = 1,
    DB_STRING = 2,
    DB_BOOL = 3,
};


#endif