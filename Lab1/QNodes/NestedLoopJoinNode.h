#ifndef NESTEDLOOPJOINNODE_H
#define NESTEDLOOPJOINNODE_H

#include "Common/DB_Iterator.h"

class NestedLoopJoinNode : public DB_Iterator {

public:
    RM_Record NextRec();
    RC Reset();
    RC SetMeta(const RM_TblMeta &m);
    bool HasNext()const;
    NestedLoopJoinNode():DB_Iterator() {lhsIter = rhsIter = nullptr;}
    ~NestedLoopJoinNode() {
        if (content != nullptr) {
            delete[] content;
        }
    }
    NestedLoopJoinNode(const NestedLoopJoinNode& rhs) : DB_Iterator(rhs) {
        content = new char[meta.GetMaxLen()];
        memcpy(content, rhs.content, meta.GetMaxLen());
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
        lhsIter = rhs.lhsIter;
        rhsIter = rhs.rhsIter;
        return *this;
    }
    NestedLoopJoinNode(NestedLoopJoinNode&& rhs) : DB_Iterator(rhs) {
        content = rhs.content;
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
        content = rhs.content;
        lhsIter = rhs.lhsIter;
        rhsIter = rhs.rhsIter;
        rhs.content = nullptr;
        rhs.lhsIter = rhs.rhsIter = nullptr;
        return *this;
    }

private:
    DB_Iterator* lhsIter, *rhsIter;
    char* content;
};

#endif