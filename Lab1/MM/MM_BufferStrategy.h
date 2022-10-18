#ifndef MM_BUFFERSTRATEGY_H
#define MM_BUFFERSTRATEGY_H
#include "MM_Buffer.h"
#include "MM/MM_BufferUnit.h"

class MM_BufferStrategy {
public:
    virtual int GetVictim(std::list<int>&, MM_BufferUnit*) = 0;
};


#endif