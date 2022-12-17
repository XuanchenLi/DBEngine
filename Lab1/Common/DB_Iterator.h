#ifndef DB_ITERATOR_H
#define DB_ITERATOR_H
#include "utils/RC.h"
#include "utils/DB_Option.h"
#include "RM/RM_Record.h"
#include "RM/RM_TblMeta.h"
#include <vector>

RC ProjectMemory(char* target, const RM_TblMeta &meta, const RM_Record& srcRec, const RM_TblMeta &srcMeta);
RC ProjectMemory(char* content, 
                const RM_TblMeta &meta,
                const RM_Record& lRec, const RM_TblMeta &lMeta, std::vector<std::string> lNames, 
                const RM_Record& rRec, const RM_TblMeta &rMeta,std::vector<std::string> rNames);

class DB_Iterator {
public:
    DB_Iterator() {done = conflict = false;}
    virtual bool HasNext()const {return !done;}
    bool IsConflict() const {return conflict;}
    virtual RM_Record NextRec() = 0;
    //virtual RC SetLimits(const std::vector<DB_Opt>& rawLim) {};
    virtual RC Reset() {done = false; conflict = false;};
    virtual RC SetMeta(const RM_TblMeta &m) {
        meta = m;
        return SUCCESS;
    }
    RM_TblMeta GetMeta() {return meta;}
    virtual ~DB_Iterator(){}
protected:
    bool done;
    bool conflict;
    RM_TblMeta meta;
private:

};


#endif