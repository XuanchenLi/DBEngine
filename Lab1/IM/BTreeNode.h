#ifndef BTREENODE_H
#define BTREENODE_H

#include "utils/RC.h"
#include "utils/DBType.h"
#include "FM/FM_Bid.h"
#include "RM/RM_Rid.h"
#include "MM/MM_PageHandler.h"
#include "IM/IM_IdxHandler.h"
#include <vector>
#include <list>



class BTreeNode {
public:
    friend class IM_IdxHandler;
    BTreeNode(dbType attrType, int attrLen);
    BTreeNode& operator=(const BTreeNode& oth);
    BTreeNode(const BTreeNode& oth);
    ~BTreeNode(); 

    RC SetData(const MM_PageHandler& pHdl);
    RC InitHead();
    RC SetPage(MM_PageHandler& pHdl);
    RC InsertPair(void* key, const RM_Rid& ptr);
    RC EraseNPair(int n);
    RC Remove(const void* key);
    RM_Rid GetSon(void* pData);
    int GetMaxPNum()const {return pHdr.slotCnt;}
    int GetKeyNum() const {return keys.size();}
    int GetBlkNum() const {return bid.num;}
    bool isRoot() const {return pHdr.preFreePage == -1;}
    int GetParent() const {return pHdr.preFreePage;}

private:
    bool Less(const void*, const void*) const;
    bool Greater(const void*, const void*) const;
    int GetStartOff() const;

    dbType attrType;
    int attrLen;
    std::list<RM_Rid> Ptrs;
    std::list<void*> keys;
    FM_Bid bid;
    MM_PageHeader pHdr;
    bool isChanged;
};



#endif