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
    //friend class IM_IdxIterator;
    BTreeNode(dbType attrType, int attrLen);
    BTreeNode& operator=(const BTreeNode& oth);
    BTreeNode(const BTreeNode& oth);
    ~BTreeNode(); 
    void SetTypeLen(dbType t, int len){attrType = t; attrLen = len;}
    RC SetData(const MM_PageHandler & pHdl);
    RC InitHead();
    RC SetPage(MM_PageHandler& pHdl);
    RC InsertPair(void* key, const RM_Rid& ptr);
    RC DeleteSinglePair(void* key, const RM_Rid& ptr);
    RC DeletePair(void* key, const RM_Rid& ptr);
    RC EraseNPair(int n);
    //RC Remove(const void* key);
    RC ReplaceKey(void* src, void*tar);

    RM_Rid GetSon(void* pData);
    RM_Rid GetFirstSon(void* pData);
    std::pair<void*, RM_Rid> GetLeftBro(const RM_Rid& ptr);
    std::pair<void*, RM_Rid> GetRightBro(const RM_Rid& ptr);
    int GetMaxPNum()const {return pHdr.slotCnt;}
    int GetKeyNum() const {return keys.size();}
    int GetPtrNum() const {return Ptrs.size();}
    int GetBlkNum() const {return bid.num;}
    bool isRoot() const {return pHdr.preFreePage == 0;}
    bool isLeaf() const {return pHdr.nextFreePage == -1;}
    int GetParent() const {return pHdr.preFreePage;}
    bool Contain(void* key, const RM_Rid& ptr);
    bool ContainKey(void* key);
    void* GetKey(int i) {
        if (i >= keys.size())
            return nullptr;
        auto p = keys.begin();
        while (i) {
            p++;
            i--;
        }
        return *p;
    }
    RM_Rid GetPtr(int i) {
        if (i >= Ptrs.size())
            return RM_Rid();
        auto p = Ptrs.begin();
        while (i) {
            p++;
            i--;
        }
        return *p;
    }
    RC Clear();
private:
    bool Less(const void*, const void*) const;
    bool Greater(const void*, const void*) const;
    bool Equal(const void*, const void*) const;
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