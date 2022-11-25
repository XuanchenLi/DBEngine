#ifndef MM_STRATEGYCLOCKSWEEP_H
#define MM_STRATEGYCLOCKSWEEP_H
#include "MM_Buffer.h"
#include "MM_BufferUnit.h"
#include "MM_BufferStrategy.h"

class MM_StrategyClockSweep : public MM_BufferStrategy{
public:
    int GetVictim(std::list<int>& victimList, MM_BufferUnit* units) {
        int res = -1;
        auto it = victimList.begin();
        if (victimList.empty()) {
            return res;
        }
        while(true) {
                if (units[*it].GetUsedCnt() == 0) {
                    res = *it;
                    victimList.remove(*it);
                    break;
                } else {
                   units[*it].SetUsedCnt(units[*it].GetUsedCnt() - 1); 
                   it++;
                   if (it == victimList.end())
                        it = victimList.begin();
                }
            }
        return res; 
    }
};


#endif