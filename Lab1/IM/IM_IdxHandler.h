#ifndef IM_IDXHANDLER_H
#define IM_IDXHANDLER_H

#include "utils/RC.h"
#include "utils/DBType.h"
#include "FM/FM_FileHandler.h"
#include "RM/RM_Rid.h"
#include <string>

class BTreeNode;
class IM_IdxIterator;
class IM_IdxHandler {
public: 
    //friend class IM_IdxIterator;
    ~IM_IdxHandler();
    IM_IdxHandler():isChanged(false), fHandler(nullptr){}
    IM_IdxHandler(const char* idxPath);
    IM_IdxHandler& operator=(const IM_IdxHandler& other) {
        if (this == &other)
            return *this;
        if (fHandler != nullptr)
            delete fHandler;
        fHandler = new FM_FileHandler;
        *fHandler = *other.fHandler;
        isChanged = other.isChanged;
        attrType = other.attrType;
        attrLen = other.attrLen;
        colPos = other.colPos;
        //maxPtrNum = other.maxPtrNum;
        return *this;
        //r.fHandler = nullptr;
    }
    IM_IdxHandler(const IM_IdxHandler& other) {
        fHandler = new FM_FileHandler;
        *fHandler = *other.fHandler;
        isChanged = other.isChanged;
        attrType = other.attrType;
        attrLen = other.attrLen;
        colPos = other.colPos;
        //maxPtrNum = other.maxPtrNum;
    }


    RC InsertEntry(void *pData, const RM_Rid &rid);  
    RC DeleteEntry(void *pData, const RM_Rid &rid);  
    RC UpdateEntry(void *pData, const RM_Rid &rid, void*nData, const RM_Rid & nRid);  
    RC OpenIdx(const char* idxPath);   
    RC CloseIdx();    
    dbType GetType() const {return attrType;}
    int GetLen() const {return attrLen;}     
    RC Traverse();
    RC GetFirstLeaf(BTreeNode& L);
    RC GetLastLeaf(BTreeNode& L); 
    RC GetQueryLeaf(void* pData, BTreeNode& L);    
    RC GetNextLeaf(BTreeNode& L);         
    std::string GetIdxPath() const {return idxPath;}
    RC GetIter(IM_IdxIterator& iter);
private:
    RC FindInsertPos(void* pData, BTreeNode& L);
    RC DeleteEntry(BTreeNode& L, void *pData, const RM_Rid &rid);  
    RC FindLeaf(void* pData, const RM_Rid &rid, BTreeNode& L);
    RC InsertInParent(BTreeNode&N1, void*key, BTreeNode& N2);

    bool isChanged;
    FM_FileHandler* fHandler;
    dbType attrType;
    std::string idxPath;
    int attrLen;
    int colPos;
    //int maxPtrNum;  // n
};



#endif