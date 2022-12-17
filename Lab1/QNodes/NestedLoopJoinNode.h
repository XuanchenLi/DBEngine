#ifndef NESTEDLOOPJOINNODE_H
#define NESTEDLOOPJOINNODE_H

#include "Common/DB_Iterator.h"

class NestedLoopJoinNode : public DB_Iterator {

public:
    RM_Record NextRec();
    RC Reset();
    RC SetLimits(std::vector<DB_JoinOpt>& lims);
    RC SetMeta(const RM_TblMeta &m);
    RC SetNames(const std::vector<std::string> lN, const std::vector<std::string> rN) {
        lNames = lN;
        rNames = rN;
    }
    //bool HasNext()const; //use default is ok
    NestedLoopJoinNode():DB_Iterator() {lhsIter = rhsIter = nullptr;}
    ~NestedLoopJoinNode() {
        if (content != nullptr) {
            delete[] content;
        }
    }
    NestedLoopJoinNode(const NestedLoopJoinNode& rhs) : DB_Iterator(rhs) {
        content = new char[meta.GetMaxLen()];
        memcpy(content, rhs.content, meta.GetMaxLen());
        limits = rhs.limits;
        lNames = rhs.lNames;
        rNames = rhs.rNames;
        lhsIter = rhs.lhsIter;
        rhsIter = rhs.rhsIter;
    }
    NestedLoopJoinNode& operator=(const NestedLoopJoinNode& rhs) {
        DB_Iterator::operator=(rhs);
        if (&rhs == this) {
            return *this;
        }
        if (content != nullptr) 
            delete [] content;
        content = new char[meta.GetMaxLen()];
        memcpy(content, rhs.content, meta.GetMaxLen());
        limits = rhs.limits;
        lNames = rhs.lNames;
        rNames = rhs.rNames;
        lhsIter = rhs.lhsIter;
        rhsIter = rhs.rhsIter;
        return *this;
    }
    NestedLoopJoinNode(NestedLoopJoinNode&& rhs) : DB_Iterator(rhs) {
        content = rhs.content;
        limits = rhs.limits;
        lNames = rhs.lNames;
        rNames = rhs.rNames;
        lhsIter = rhs.lhsIter;
        rhsIter = rhs.rhsIter;
        rhs.content = nullptr;
        rhs.lhsIter = rhs.rhsIter = nullptr;
    }
    NestedLoopJoinNode& operator=(NestedLoopJoinNode&& rhs) {
        if (&rhs == this) {
            return *this;
        }
        DB_Iterator::operator=(rhs);
        limits = rhs.limits;
        lNames = rhs.lNames;
        rNames = rhs.rNames;
        content = rhs.content;
        lhsIter = rhs.lhsIter;
        rhsIter = rhs.rhsIter;
        rhs.content = nullptr;
        rhs.lhsIter = rhs.rhsIter = nullptr;
        return *this;
    }

private:
    RM_Record lNextRec, rNextRec;
    DB_Iterator* lhsIter, *rhsIter;
    std::vector<DB_JoinOpt> limits;
    std::vector<std::string> lNames, rNames;
    char* content;
};

#endif