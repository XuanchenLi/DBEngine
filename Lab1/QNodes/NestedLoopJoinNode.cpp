#include "NestedLoopJoinNode.h"

RC NestedLoopJoinNode::Reset() {
    if (lhsIter == nullptr || rhsIter == nullptr) {
        return FAILURE;
    }
    if (hasReseted)
        return SUCCESS;
    DB_Iterator::Reset();
    hasReseted = true;
    lhsIter->Reset();
    rhsIter->Reset();
    conflict = lhsIter->IsConflict() || rhsIter->IsConflict() || conflict;
    done = !lhsIter->HasNext() || !rhsIter->HasNext();
    //std::cout<<lhsIter->HasNext()<<" "<<rhsIter->HasNext()<<std::endl;
    if (done) {
        return SUCCESS;
    }
    //std::cout<<"12121212\n";
    RM_Record lRec, rRec;
    while(lhsIter->HasNext()) {
        lRec = lhsIter->NextRec();
        while(rhsIter->HasNext()) {
            rRec = rhsIter->NextRec();
            if (validJoinOpt(lRec, lhsIter->GetMeta(), rRec, rhsIter->GetMeta(), limits)) {
                if (lRec.addr == nullptr)
                    std::cout<<"mmm\n";
                else 
                    std::cout<<"yyy\n";
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
    hasReseted = false;
    res.addr = content;
    //std::cout<<"1231212\n";
    ProjectMemory(content, meta, lNextRec, lhsIter->GetMeta(), lNames, rNextRec, rhsIter->GetMeta(), rNames);
    //std::cout<<"12312412\n";
    res.InitPrefix(meta);
    //std::cout<<"21312312\n"<<std::endl;
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