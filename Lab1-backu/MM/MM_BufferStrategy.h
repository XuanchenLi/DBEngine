#ifndef MM_BUFFERSTRATEGY_H
#define MM_BUFFERSTRATEGY_H
#include "MM_Buffer.h"

class MM_BufferUnit;
class MM_BufferStrategy {
public:
    virtual int GetVictim(std::list<int>&, MM_BufferUnit*) = 0;
};


#endif