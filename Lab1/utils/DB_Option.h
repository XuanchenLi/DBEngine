#ifndef DB_OPTION_H
#define DB_OPTION_H

#include <string>
#include "utils/Optrs.h"
#include "utils/DBType.h"

typedef struct DB_Option {
    optr optr;
    std::string colName;
    dbType type;
    union {
        char sData[64];
        int iData;
        double lfData;
        bool bData;
    }data;

}DB_Opt;



#endif