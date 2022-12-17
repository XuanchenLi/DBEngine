#ifndef INDEXDIRECTACCESSNODE_H
#define INDEXDIRECTACCESSNODE_H
#include "Common/DB_Iterator.h"
#include "IM/IM_IdxIterator.h"


class IndexDirectAccessNode : public DB_Iterator {
public:
    RM_Record NextRec();
    RC Reset();
    RC SetMeta(const RM_TblMeta &m);
    bool HasNext()const;
    void SetSrcIter(IM_IdxIterator* src) {
        srcIter = src;
    }
    IndexDirectAccessNode():DB_Iterator() {srcIter = nullptr;}
    ~IndexDirectAccessNode() {
        if (content != nullptr) {
            delete[] content;
        }
    }
    IndexDirectAccessNode(const IndexDirectAccessNode& rhs) : DB_Iterator(rhs) {
        content = new char[meta.GetMaxLen()];
        memcpy(content, rhs.content, meta.GetMaxLen());
        srcIter = rhs.srcIter;
    }
    IndexDirectAccessNode& operator=(const IndexDirectAccessNode& rhs) {
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
    IndexDirectAccessNode(IndexDirectAccessNode&& rhs) : DB_Iterator(rhs) {
        content = rhs.content;
        srcIter = rhs.srcIter;
        rhs.srcIter = nullptr;
        rhs.content = nullptr;
    }
    IndexDirectAccessNode& operator=(IndexDirectAccessNode&& rhs) {
        if (&rhs == this) {
            return *this;
        }
        DB_Iterator::operator=(rhs);
        srcIter = rhs.srcIter;
        content = rhs.content;
        rhs.content = nullptr;
        rhs.srcIter = nullptr;
        return *this;
    }
private:
    IM_IdxIterator* srcIter;
    char* content;
};

#endif