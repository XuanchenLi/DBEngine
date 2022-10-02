#ifndef MM_BUFFER_H
#define MM_BUFFER_H

#include <unordered_map>
#include <list>
#include "FM/FM_Bid.h"
#include "utils/RC.h"


class MM_PageHandler;
class MM_BufferUnit;
struct Bid_Hash;


class MM_Buffer {

public:
    //MM_Buffer();
    MM_Buffer(unsigned int);
    ~MM_Buffer();
    MM_Buffer(const MM_Buffer &)=delete;
    MM_Buffer& operator= (const MM_Buffer &)=delete;

    RC GetPage(FM_Bid, MM_PageHandler&);
    RC Unpin(FM_Bid);
    RC Pin(FM_Bid);
    RC Clear();
    RC Clear(int fd);
    RC ForcePage(FM_Bid);
    RC ForcePage(int fd);
    RC SetDirty(FM_Bid);

    int size() {return hashTbl.size();}

private:
    int GetEmpty();
    
    MM_BufferUnit* units;
    std::unordered_map<FM_Bid, int, Bid_Hash> hashTbl;
    std::list<int> victimList;
    std::list<int> freeList;
};

#endif