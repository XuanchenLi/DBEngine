#ifndef PROJECTIONNODE_H
#define PROJECTIONNODE_H

#include "Common/DB_Iterator.h"
#include "RM/RM_TblMeta.h"

class ProjectionNode :public DB_Iterator {

public:
    ProjectionNode():DB_Iterator() {srcIter = nullptr;}
    RC SetSrcIter(DB_Iterator* src) {
        srcIter = src;
        return Reset();
    }
    RM_Record NextRec();
    RC Reset() {
        DB_Iterator::Reset();
        if (srcIter == nullptr) {
            return FAILURE;
        }
        srcIter->Reset();
        conflict = srcIter->IsConflict();
        done = !srcIter->HasNext();
        return SUCCESS;
    }
    bool HasNext()const {
        if (srcIter == nullptr) {
            return false;
        }
        return srcIter->HasNext();
    }
    ~ProjectionNode() {
        if (content != nullptr) {
            delete[] content;
        }
    }
    ProjectionNode(const ProjectionNode& rhs) : DB_Iterator(rhs) {
        content = new char[meta.GetMaxLen()];
        memcpy(content, rhs.content, meta.GetMaxLen());
        srcIter = rhs.srcIter;
    }
    ProjectionNode& operator=(const ProjectionNode& rhs) {
        DB_Iterator::operator=(rhs);
        if (&rhs == this) {
            return *this;
        }
        if (content != nullptr) 
            delete [] content;
        content = new char[meta.GetMaxLen()];
        memcpy(content, rhs.content, meta.GetMaxLen());
        srcIter = rhs.srcIter;
        return *this;
    }
    ProjectionNode(ProjectionNode&& rhs) : DB_Iterator(rhs) {
        content = rhs.content;
        srcIter = rhs.srcIter;
        rhs.content = nullptr;
        rhs.srcIter = nullptr;
    }
    ProjectionNode& operator=(ProjectionNode&& rhs) {
        if (&rhs == this) {
            return *this;
        }
        DB_Iterator::operator=(rhs);
        content = rhs.content;
        srcIter = rhs.srcIter;
        rhs.content = nullptr;
        rhs.srcIter = nullptr;
        return *this;
    }
    RC SetMeta(const RM_TblMeta &m);
private:
    //RC ProjectMemory(const RM_Record& srcRec, const RM_TblMeta &m);
    DB_Iterator* srcIter;
    char* content;
};

#endif