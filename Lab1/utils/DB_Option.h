#ifndef DB_OPTION_H
#define DB_OPTION_H

#include <string>
#include <unistd.h>
#include <string.h>
#include "utils/Optrs.h"
#include "utils/DBType.h"

typedef struct DB_Option {
    db_optr optr;
    std::string colName;
    dbType type;
    union {
        char sData[64];
        int iData;
        double lfData;
        bool bData;
    }data;

}DB_Opt;

typedef struct DB_NumericOption {
    db_optr optr;
    int colPos;
    dbType type;
    union {
        char sData[64];
        int iData;
        double lfData;
        bool bData;
    }data;
public:
    bool operator<(const DB_NumericOption& rhs) const {
        if (rhs.optr != optr)
            return optr<rhs.optr;
        return comp(type, LESS, (void*)&data.sData, (void*)&rhs.data.sData);
    }
    bool test(const void* ptr) const {
        return comp(type, optr, ptr, data.sData);
    }

}DB_NumOpt;

typedef struct DB_Condition {
    db_optr optr;
    std::string lTblName;
    std::string lColName;
    std::string rTblName;
    std::string rColName;
    bool isConst;
    dbType type;
    union {
        char sData[64];
        int iData;
        double lfData;
        bool bData;
    }data;
}DB_Cond;

typedef struct DB_NumericCondition {
    db_optr optr;
    std::string lTblName;
    int lColPos;
    std::string rTblName;
    int rColPos;
    bool isConst;
    dbType type;
    union {
        char sData[64];
        int iData;
        double lfData;
        bool bData;
    }data;
}DB_NumCond;


DB_Opt TransToOpt(DB_Cond cond) {
    DB_Opt opt;
    if (!cond.isConst) {
        printf("Bad Transform!\n");
        return opt;
    }
    opt.optr = cond.optr;
    opt.type = cond.type;
    opt.colName = cond.lColName;
    memcpy(&opt.data, &cond.data, sizeof(cond.data));
    return opt;
}


#endif