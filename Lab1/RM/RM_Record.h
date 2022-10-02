#ifndef RM_RECORD_H
#define RM_RECORD_H

#include <tuple>
#include <iostream>
#include "RM_Rid.h"
#include "utils/RC.h"

struct RM_Record {

    RM_Rid rid;
    char* addr;
    int len;
    
    template <typename T>
    RC SetContent(T t) {
        if (addr == nullptr)
            return INVALID_OPTR;
        memcpy(addr, (char*)&t, sizeof(t));
        len = sizeof(t);
        return SUCCESS;
    }


};


#endif