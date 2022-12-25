#ifndef IM_IDXITERATOR_H
#define IM_IDXITERATOR_H


#include "utils/RC.h"
#include "Common/DB_Iterator.h"
#include "IM_IdxHandler.h"
#include "BTreeNode.h"
#include "RM/RM_Record.h"
#include "utils/DB_Option.h"
#include "utils/RC.h"
#include "RM/RM_Rid.h"
#include <vector>


class IM_IdxIterator : public DB_Iterator{
public: 
    IM_IdxIterator(dbType t, int len):attrType(t), attrLen(len), curLeaf(t, len){
        gtOptIdx = -1, nlOptIdx = -1, eqOptIdx = -1, lOptIdx = -1, ngOptIdx = -1;
        limits.clear(), outerLimits.clear();
        kind = "Index Iterator";
    }
    RC SetOuterLimits(const std::vector<DB_Opt>& rawLim) {
        outerLimits = rawLim;
        hasReseted = false;
        return SUCCESS;
    }
    RC SetLimits(const std::vector<DB_Opt>& rawLim);
    RC SetLimits(const std::vector<DB_NumOpt>& oLim) {
        limits = oLim;
        //printf(":123\n");
        StandardizeLimits();
        //printf(":12223\n");
        hasReseted = false;
        Reset();
        //printf(":1222223\n");
        return SUCCESS;
    }
    std::pair<void*, RM_Rid> NextPair();
    RM_Record NextRec();
    RC Reset();
    RM_Record GetRecord(std::pair<void*, RM_Rid>& pr);
    void SetIdxHandler(IM_IdxHandler& i) {
        idxHandler = i;
        std::string path = idxHandler.GetIdxPath();
        //std::cout<<path<<std::endl;
        if (path[path.length() - 1] == '/') {
            path = path.substr(0, path.length() - 1);
        }  
        //std::cout<<tmp<<std::endl;
        int idx = path.find_last_of('_');
        if (idx != std::string::npos) {
            path = path.substr(0, idx);
        }
        tblPath = path;
        hasReseted = false;
        //RM_TableHandler tHandler(tblPath.c_str());
        //meta = tHandler.GetMeta();
    }
    //bool HasNext()const {return !done;}
    void SetTypeLen(dbType t, int len){attrType = t; attrLen = len;curLeaf.SetTypeLen(attrType, attrLen);}
    DB_Iterator* clone() {return new IM_IdxIterator(*this);}
private:
    void StandardizeLimits();
    int gtOptIdx, nlOptIdx, eqOptIdx, lOptIdx, ngOptIdx;
    IM_IdxHandler idxHandler;
    BTreeNode curLeaf;
    dbType attrType;
    int attrLen;
    int curSlot;
    std::vector<DB_NumOpt> limits;
    std::string tblPath;
    //RM_TblMeta tblMeta;
    std::vector<DB_Opt> outerLimits;
    //bool conflict;
    //bool done;
};



#endif