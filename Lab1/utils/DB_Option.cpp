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