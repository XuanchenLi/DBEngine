#ifndef MM_PAGEHANDLER_H
#define MM_PAGEHANDLER_H

#include "utils/RC.h"
#include "FM/FM_Bid.h"
#include "FM/FM_FileHandler.h"
#include "MM_BufferUnit.h"
#include "MM_PageHeader.h"

class MM_BufferUnit;

class MM_PageHandler {
public:
    MM_PageHandler(): buf(nullptr){}
    ~MM_PageHandler();
    MM_PageHandler(const MM_PageHandler & rhs);
    MM_PageHandler& operator= (const MM_PageHandler & rhs);

    RC Pin();
    RC Unpin();
    RC Force();
    RC SetDirty();

    RC SetPage(MM_BufferUnit*);
    MM_PageHdr GetHeader() const{return pHdr;}
    RC SetHeader(const MM_PageHdr& h);
    FM_Bid GetBid() const {return buf->GetBid();}
    char* GetPtr(int off);
    const char* GetPtr(int off) const;
private:
    MM_BufferUnit* buf;
    MM_PageHeader pHdr;
    //FM_FileHandler* fHandle;
};  

#endif