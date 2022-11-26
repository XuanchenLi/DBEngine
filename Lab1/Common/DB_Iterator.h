#ifndef DB_ITERATOR_H
#define DB_ITERATOR_H
#include "utils/RC.h"
#include "utils/DB_Option.h"
#include "RM/RM_Record.h"
#include <vector>

class DB_Iterator {
public:
    bool HasNext()const {return !done;}
    virtual RM_Record NextRec() = 0;
    virtual RC SetLimits(const std::vector<DB_Opt>& rawLim) = 0;
    RC Reset() {done = false; conflict = false;};

protected:
    bool done;
    bool conflict;
private:

};



#endif