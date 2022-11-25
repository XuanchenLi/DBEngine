#ifndef IM_IDXITERATOR_H
#define IM_IDXITERATOR_H


#include "utils/RC.h"
#include "IM_IdxHandler.h"
#include "BTreeNode.h"
#include "RM/RM_Record.h"
#include "utils/DB_Option.h"
#include "utils/RC.h"
#include "RM/RM_Rid.h"
#include <vector>


class IM_IdxIterator {
public: 
    IM_IdxIterator(dbType t, int len):attrType(t), attrLen(len), curLeaf(t, len){
        gtOptIdx = -1, nlOptIdx = -1, eqOptIdx = -1, lOptIdx = -1, ngOptIdx = -1;
    }
    RC SetLimits(const std::vector<DB_Opt>& rawLim);
    RC SetLimits(const std::vector<DB_NumOpt>& oLim) {
        limits = oLim;
        //printf(":123\n");
        StandardizeLimits();
        //printf(":12223\n");
        Reset();
        //printf(":1222223\n");
        return SUCCESS;
    }
    std::pair<void*, RM_Rid> NextPair();
    RM_Record NextRec();
    RC Reset();
    void SetIdxHandler(IM_IdxHandler& i) {
        idxHandler = i;
    }
    bool HasNext()const {return !done;}
    void SetTypeLen(dbType t, int len){attrType = t; attrLen = len;curLeaf.SetTypeLen(attrType, attrLen);}

private:
    void StandardizeLimits();
    int gtOptIdx, nlOptIdx, eqOptIdx, lOptIdx, ngOptIdx;
    IM_IdxHandler idxHandler;
    BTreeNode curLeaf;
    dbType attrType;
    int attrLen;
    int curSlot;
    std::vector<DB_NumOpt> limits;
    bool conflict;
    bool done;
};



#endif