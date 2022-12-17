#include <iostream>
#include "ProjectionNode.h"

RM_Record ProjectionNode::NextRec() {
    RM_Record rec;
    rec.rid.num = -1;
    if (srcIter == nullptr) {
        std::cout<<"null pointer error!\n";
        return rec;
    }
    if (done || conflict) {
        return rec;
    }
    RM_Record srcRec = srcIter->NextRec();
    done = !srcIter->HasNext();
    rec.InitPrefix(meta);
    rec.addr = content;
    ProjectMemory(content, meta, srcRec, srcIter->GetMeta());
    return rec;
}

RC ProjectionNode::SetMeta(const RM_TblMeta &m) {
    DB_Iterator::SetMeta(m);
    if (content != nullptr) {
        delete[] content;
    }
    content = new char[m.GetMaxLen()];
    return SUCCESS;
}
