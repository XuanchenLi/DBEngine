#ifndef RM_TBLMETA_H
#define RM_TBLMETA_H

#include <string>
#include <string.h>
#include "main.h"

struct RM_TblMeta {
    std::string dbName;
    std::string tblName;
    int colNum;
    int type[MAXCOLNUM];
    std::string colName[MAXCOLNUM];
    bool isPrimary[MAXCOLNUM];
    int length[MAXCOLNUM];
    int isDynamic[MAXCOLNUM];
    int colPos[MAXCOLNUM];

};



#endif