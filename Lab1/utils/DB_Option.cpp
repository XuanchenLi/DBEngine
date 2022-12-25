#include "DB_Option.h"


DB_Opt TransToOpt(const DB_Cond cond) {
    DB_Opt opt;
    if (!cond.isConst) {
        printf("Bad Transform!\n");
        return opt;
    }
    opt.optr = cond.optr;
    opt.type = cond.type;
    opt.colName = cond.lColName;
    switch (cond.type) {
        case DB_INT:
            opt.data.iData = cond.data.iData;
            break;
        case DB_STRING:
            strcpy(opt.data.sData, cond.data.sData);
            break;
        case DB_DOUBLE:
            opt.data.lfData = cond.data.lfData;
            break;
        case DB_BOOL:
            opt.data.bData = cond.data.bData;
            break;
    }
    //memcpy(&opt.data.sData, &cond.data.sData, sizeof(cond.data));
    return opt;
}

DB_JoinOpt TransToJoinOpt(const DB_Cond cond) {
    DB_JoinOpt opt;
    if (cond.isConst) {
        printf("Bad Transform!\n");
        return opt;
    }
    opt.optr = cond.optr;
    opt.type = cond.type;
    opt.lColName = cond.lColName;
    opt.rColName = cond.rColName;
    return opt;
}

db_optr InverseOptr(db_optr optr) {
    switch(optr) {
        case LESS:
            return GREATER;
        case GREATER:
            return LESS;
        case NOT_LESS:
            return NOT_GREATER;
        case NOT_GREATER:
            return NOT_LESS;
        default:
            return optr;
    }
}