#ifndef RM_TABLEHANDLER_H
#define RM_TABLEHANDLER_H

#include <utility>
#include <string>
#include <iostream>
#include "RM/RM_Record.h"
#include "utils/RC.h"
#include "FM/FM_FileHandler.h"
#include "RM_TblMeta.h"
#include "RM/RM_RecAux.h"

struct RM_Rid;
class FM_FileHandler;
class RM_TblIterator;
//class IM_IdxIterator;
struct RM_RecAux;

class RM_TableHandler {
friend class RM_TblIterator;
//friend class IM_IdxIterator;

public:
    ~RM_TableHandler();
    RM_TableHandler(){fHandler = nullptr;}
    RM_TableHandler(const char* tblPath);
    
    RM_TableHandler& operator=(const RM_TableHandler& other) {
        if (this == &other)
            return *this;
        if (fHandler != nullptr)
            delete fHandler;
        fHandler = new FM_FileHandler;
        *fHandler = *other.fHandler;
        isChanged = other.isChanged;
        metaData = other.metaData;
        return *this;
        //r.fHandler = nullptr;
    }
    
    RC OpenTbl(const char* tblPath);
    RC CloseTbl();
    template<typename T>
    RC Insert(T);

    //TODO
    //RC Insert(RM_RecAux);
    
    RC GetRec(const RM_Rid&, RM_Record&);
    RC GetIter(RM_TblIterator& iter);
    RC InsertRec(const RM_RecAux&);
    int GetFd()const {return fHandler->GetFd();}
    FM_FileHdr GetFileHdr()const {return fHandler->GetFileHdr();}

    const RM_TblMeta& GetMeta() const {
        return metaData;
    }

    //void test() {std::cout<<fHandler->GetFileHdr().firstFreeHole<<std::endl;}
private:
    RC InitTblMeta();
    RC InsertRec(const RM_Record&);
    RC GetNextFreeSlot(RM_Rid&);
    RC GetNextFreeSlot(RM_Rid&, int len); // len包括记录前缀长不包括头部长
    FM_FileHandler* fHandler;
    //int recLen;
    RM_TblMeta metaData;
    bool isChanged;
};

template<typename T>
RC RM_TableHandler::Insert(T obj) {
    RM_Record rec;
    rec.addr = new char[sizeof(T)];
    rec.len = sizeof(T);
    int rc = this->GetNextFreeSlot(rec.rid);
    if (rc!=SUCCESS)
        return rc;

    rec.SetContent<T>(obj);
    rc = this->InsertRec(rec);
    delete[] rec.addr;
    return rc;
}



#endif