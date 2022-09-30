#ifndef RM_TABLEHANDLER_H
#define RM_TABLEHANDLER_H

#include <utility>
#include <string>
#include <iostream>
#include "utils/RC.h"
#include "FM/FM_FileHandler.h"

struct RM_Rid;
struct RM_Record;
class FM_FileHandler;
class RM_TblIterator;

class RM_TableHandler {
friend class RM_TblIterator;

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
        return *this;
        //r.fHandler = nullptr;
    }
    
    RC OpenTbl(const char* tblPath);
    RC CloseTbl();
    RC InsertRec(const RM_Record&);
    RC GetRec(const RM_Rid&, RM_Record&);
    RC GetNextFreeSlot(RM_Rid&);
    RC GetIter(RM_TblIterator& iter);

    //void test() {std::cout<<fHandler->GetFileHdr().firstFreeHole<<std::endl;}
private:
    FM_FileHandler* fHandler;
    //int recLen;
    bool isChanged;
};


#endif