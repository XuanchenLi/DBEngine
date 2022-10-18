#ifndef MM_STRATEGYLRU_H
#define MM_STRATEGYLRU_H
#include "MM_Buffer.h"
#include "MM_BufferStrategy.h"

class MM_StrategyLRU : public MM_BufferStrategy{
public:
    int GetVictim(std::list<int>& victimList, MM_BufferUnit* units) {
        if (victimList.empty())
            return OUT_OF_RANGE_ERROR;
        int res = victimList.front();
        victimList.pop_front();
        return res;
    }
};


#endif