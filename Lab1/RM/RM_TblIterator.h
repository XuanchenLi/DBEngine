#ifndef RM_TBLITERATOR_H
#define RM_TBLITERATOR_H


#include <vector>
#include "utils/RC.h"
#include "utils/Optrs.h"
#include "utils/DB_Option.h"
#include "RM_TableHandler.h"

class RM_Record;

class RM_TblIterator {

friend class RM_TableHandler;
public:
    RM_TblIterator(){ curPNum = 1; curSNum = -1;done=false;}
    RM_TblIterator(const char* tblName) {SetTbl(tblName);};
    RC SetTbl(const char* tblName);
    RC SetLimits(const std::vector<DB_Opt>&);
    RM_Record NextRec();  //num为-1代表读到尾
    RC Reset();

private:
    //bool valid();
    RM_TableHandler tHandler;
    int curPNum;
    int curSNum;
    std::vector<DB_Opt> limits;
    bool done;
};


#endif