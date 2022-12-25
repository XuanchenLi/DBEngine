#ifndef NESTEDLOOPJOINNODE_H
#define NESTEDLOOPJOINNODE_H

#include "Common/DB_Iterator.h"

class NestedLoopJoinNode : public DB_Iterator {

public:
    RM_Record NextRec();
    RC Reset();
    
    RC SetLimits(std::vector<DB_JoinOpt>& lims) {
        limits = lims;
        hasReseted = false;
    }
    RC SetMeta(const RM_TblMeta &m);
    RC SetNames(const std::vector<std::string> lN, const std::vector<std::string> rN) {
        lNames = lN;
        rNames = rN;
        return SUCCESS;
    }
    RC SetSrcIter(DB_Iterator* lhs, DB_Iterator* rhs) {
        lhsIter = lhs;
        rhsIter = rhs;
        hasReseted = false;
        return SUCCESS;
    }
    //bool HasNext()const; //use default is ok
    DB_Iterator* clone() {return new NestedLoopJoinNode(*this);}
    NestedLoopJoinNode():DB_Iterator() {lhsIter = rhsIter = nullptr;content=nullptr;}
    ~NestedLoopJoinNode() {
        if (content != nullptr) {
            delete[] content;
        }
        if (lhsIter != nullptr)
            delete lhsIter;
        if (rhsIter != nullptr) 
            delete rhsIter;
    }
    NestedLoopJoinNode(const NestedLoopJoinNode& rhs) : DB_Iterator(rhs) {
        content = new char[meta.GetMaxLen()];
        memcpy(content, rhs.content, meta.GetMaxLen());
        limits = rhs.limits;
        lNames = rhs.lNames;
        rNames = rhs.rNames;
        lhsIter = rhs.lhsIter->clone();
        rhsIter = rhs.rhsIter->clone();
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
        lhsIter = rhs.lhsIter->clone();
        rhsIter = rhs.rhsIter->clone();
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