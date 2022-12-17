#include "RM_Record.h"

void convert2Double(dbType type, RM_Record& rec, const RM_TblMeta& meta, int pos, double* ptr) {
    switch(type) {
        case DB_BOOL:
            bool srcBPtr;
            rec.GetColData(meta, pos, &srcBPtr);
            if (srcBPtr) {
                *ptr = 1;
            }else {
                *ptr = 0;
            }
            break;
        case DB_INT:
            int srcIPtr;
            rec.GetColData(meta, pos, &srcIPtr);
            *ptr = srcIPtr;
            break;
        case DB_DOUBLE:
            break;
        case DB_STRING:
            break;
    }
    return;
}


bool validJoinOpt(RM_Record& lRec, const RM_TblMeta& lMeta, 
                RM_Record& rRec, const RM_TblMeta& rMeta,
                std::vector<DB_JoinOpt>& limits) {
    for (auto limItem: limits) {
        int lIdx = lMeta.GetIdxByName(limItem.lColName);
        int rIdx = rMeta.GetIdxByName(limItem.rColName);
        if (lIdx == NOT_EXIST || rIdx == NOT_EXIST) {
            return false;
        }
        int lPos = lMeta.GetPosByName(limItem.lColName);
        int rPos = rMeta.GetPosByName(limItem.rColName);
        dbType lType = lMeta.type[lIdx], rType = lMeta.type[rIdx];
        if (lMeta.type[lIdx] == DB_STRING) {
            char* lPtr = new char[lMeta.length[lIdx]];
            lRec.GetColData(lMeta, lPos, lPtr);
            char* rPtr = new char[rMeta.length[rIdx]];
            rRec.GetColData(rMeta, rPos, rPtr);
            if (!comp(DB_STRING, limItem.optr, lPtr, rPtr)) {
                delete[] lPtr;
                delete[] rPtr;
                return false;
            }
            delete[] lPtr;
            delete[] rPtr;
        }else {
            double lPtr;
            convert2Double(lType, lRec, lMeta, lPos, &lPtr);
            double rPtr;
            convert2Double(rType, rRec, rMeta, rPos, &rPtr);
            if (!comp(DB_DOUBLE, limItem.optr, &lPtr, &rPtr)) {
                return false;
            }
        }
    }
    return true;
}