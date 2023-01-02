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
    DB_Iterator() {done = conflict = hasReseted = false;}
    virtual bool HasNext()const {return !done;}
    bool IsConflict() const {return conflict;}
    virtual RM_Record NextRec() = 0;
    //virtual RC SetLimits(const std::vector<DB_Opt>& rawLim) {};
    virtual RC Reset() {done = false; conflict = false; return SUCCESS;};
    virtual RC SetMeta(const RM_TblMeta &m) {
        meta = m;
        return SUCCESS;
    }
    virtual void Print(int level) {
        std::string li(8 * level, '-');
        std::cout<<li + kind <<std::endl;
    }
    RM_TblMeta GetMeta() {return meta;}
    std::string GetKind() {return kind;}
    virtual ~DB_Iterator(){}
    virtual DB_Iterator* clone() = 0;
protected:
    bool done;
    bool conflict;
    bool hasReseted;
    RM_TblMeta meta;
    std::string kind;
private:

};


#endif