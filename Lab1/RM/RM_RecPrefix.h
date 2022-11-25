#ifndef RM_RECPREFIX_H
#define RM_RECPREFIX_H

#include <string>
#include <string.h>
#include "main.h"


typedef struct PosInfo {
    int start;
    int end;
}PosInfo;


class RM_RecPrefix {
public:
    RM_RecPrefix() {
        posInfo = nullptr;
    }
    RM_RecPrefix(int len) {
        posInfo = new PosInfo[len];
    }
    ~RM_RecPrefix() {
        if (posInfo!= nullptr)
            delete[] posInfo;
    }
    RM_RecPrefix(const RM_RecPrefix& oth) {
        if (posInfo!= nullptr)
            delete[] posInfo;
        used = oth.used;
        dColNum = oth.dColNum;
        posInfo = new PosInfo[oth.dColNum];
        memcpy(posInfo, oth.posInfo, sizeof(PosInfo)*dColNum);
    }
    RM_RecPrefix& operator=(const RM_RecPrefix& oth) {
        if (this == &oth)
            return *(this);
        if (posInfo!= nullptr)
            delete[] posInfo;
        used = oth.used;
        dColNum = oth.dColNum;
        posInfo = new PosInfo[oth.dColNum];
        if (oth.posInfo != nullptr)
            memcpy(posInfo, oth.posInfo, sizeof(PosInfo)*dColNum);
        return *this;
    }
    int size() {
        return sizeof(bool) + dColNum*sizeof(posInfo) + sizeof(int);
    }
    bool used;  //是否空闲
    int dColNum;  //可变长字段数目
    PosInfo* posInfo;  //可变长字段偏移信息
};


#endif