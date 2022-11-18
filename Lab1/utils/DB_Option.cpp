#include "DB_Option.h"


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