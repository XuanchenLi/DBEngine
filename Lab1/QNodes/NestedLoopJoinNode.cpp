#include "NestedLoopJoinNode.h"

RC NestedLoopJoinNode::Reset() {
    DB_Iterator::Reset();
    if (lhsIter == nullptr || rhsIter == nullptr) {
        return FAILURE;
    }
    lhsIter->Reset();
    rhsIter->Reset();
    conflict = lhsIter->IsConflict() || rhsIter->IsConflict() || conflict;
    done = !lhsIter->HasNext() || !rhsIter->HasNext();
    if (done) {
        return SUCCESS;
    }
    RM_Record lRec, rRec;
    while(lhsIter->HasNext()) {
        lRec = lhsIter->NextRec();
        while(rhsIter->HasNext()) {
            rRec = rhsIter->NextRec();
            if (validJoinOpt(lRec, lhsIter->GetMeta(), rRec, rhsIter->GetMeta(), limits)) {
                lNextRec = lRec, rNextRec = rRec;
                return SUCCESS;
            }
        }
        rhsIter->Reset();
    }
    done = true;
    return SUCCESS;
}

RC NestedLoopJoinNode::SetMeta(const RM_TblMeta &m) {
    DB_Iterator::SetMeta(m);
    if (content == nullptr)
        delete[] content;
    content = new char[m.GetMaxLen()];
    return SUCCESS;
}

RM_Record NestedLoopJoinNode::NextRec() {
    RM_Record res;
    if (!HasNext()) {
        return res;
    }
    res.addr = content;
    ProjectMemory(content, meta, lNextRec, lhsIter->GetMeta(), lNames, rNextRec, rhsIter->GetMeta(), rNames);
    res.InitPrefix(meta);
    RM_Record lRec, rRec;
    lRec = lNextRec;
    while(rhsIter->HasNext()) {
        rRec = rhsIter->NextRec();
        if (validJoinOpt(lRec, lhsIter->GetMeta(), rRec, rhsIter->GetMeta(), limits)) {
            lNextRec = lRec, rNextRec = rRec;
            return res;
        }
    }
    rhsIter->Reset();
    while(lhsIter->HasNext()) {
        lRec = lhsIter->NextRec();
        while(rhsIter->HasNext()) {
            rRec = rhsIter->NextRec();
            if (validJoinOpt(lRec, lhsIter->GetMeta(), rRec, rhsIter->GetMeta(), limits)) {
                lNextRec = lRec, rNextRec = rRec;
                return res;
            }
        }
        rhsIter->Reset();
    }
    done = true;
    return res;
}