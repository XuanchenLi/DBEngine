#include "IndexDirectAccessNode.h"


RM_Record IndexDirectAccessNode::NextRec() {
    RM_Record rec;
    rec.rid.num = -1;
    if (srcIter == nullptr) {
        std::cout<<"null pointer error!\n";
        return rec;
    }
    if (done || conflict) {
        return rec;
    }
    if (content == nullptr) {
        return rec;
    }
    hasReseted = false;
    std::pair<void*, RM_Rid> pr = srcIter->NextPair();
    done = !srcIter->HasNext();
    if (pr.first == nullptr) {
        return rec;
    }
    rec.InitPrefix(meta);
    rec.addr = content;
    auto srcMeta = srcIter->GetMeta();
    //只支持单字段索引
    int srcIdx = srcMeta.GetIdxByName(meta.colName[0]);
    const bool tl = false;
    int ti = 0;
    meta.isDynamic[0] = false;  //确保定长
    memcpy(content, &tl, sizeof(bool));
    memcpy(content + sizeof(bool), &ti, sizeof(int));
    switch (meta.type[0]) {
        case DB_INT:
            memcpy(content + sizeof(bool) + sizeof(int), pr.first, sizeof(int));
            break;
        case DB_BOOL:
            memcpy(content + sizeof(bool) + sizeof(int), pr.first, sizeof(bool));
            break;
        case DB_STRING:
            memcpy(content + sizeof(bool) + sizeof(int), pr.first, meta.length[0]);
            break;
        case DB_DOUBLE:
            memcpy(content + sizeof(bool) + sizeof(int), pr.first, sizeof(double));
            break;
    }

    return rec;
}
RC IndexDirectAccessNode::Reset() {
    DB_Iterator::Reset();
    if (srcIter == nullptr) {
        return FAILURE;
    }
    if (hasReseted)
        return SUCCESS;
    hasReseted = true;
    srcIter->Reset();
    conflict = srcIter->IsConflict();
    done = !srcIter->HasNext();
    return SUCCESS;
}
RC IndexDirectAccessNode::SetMeta(const RM_TblMeta &m) {
    meta = m;
    if (content != nullptr) {
        delete[] content;
    }
    content = new char[m.GetMaxLen()];
    return SUCCESS;
}
bool IndexDirectAccessNode::HasNext() const {
    if (srcIter == nullptr) {
            return false;
    }
    return srcIter->HasNext();
}