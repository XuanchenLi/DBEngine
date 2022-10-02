#ifndef RM_RECPREFIX_H
#define RM_RECPREFIX_H

#include <string>
#include "main.h"

template <size_t s>
class RM_RecPrefix {
public:
    bool used;
    PosInfo posInfo[s];
};
typedef struct PosInfo {
    int start;
    int len;
}PosInfo;


#endif