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
    RC SetLimits(const std::vector<DB_NumOpt>& oLim) {
        limits = oLim;
        StandardizeLimits();
        Reset();
        return SUCCESS;
    }
    std::pair<void*, RM_Rid> NextPair();
    RM_Record NextRec();
    RC Reset();
    void SetIdxHandler(IM_IdxHandler& i) {
        idxHandler = i;
    }

private:
    void StandardizeLimits();

    IM_IdxHandler idxHandler;
    BTreeNode curLeaf;
    int curSlot;
    std::vector<DB_NumOpt> limits;
    bool conflict;
    bool done;
};



#endif