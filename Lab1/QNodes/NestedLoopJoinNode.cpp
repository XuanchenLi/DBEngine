#include "NestedLoopJoinNode.h"

RC NestedLoopJoinNode::Reset() {
    DB_Iterator::Reset();
    if (lhsIter == nullptr || rhsIter == nullptr) {
        return FAILURE;
    }
    lhsIter->Reset();
    rhsIter->Reset();
    conflict = lhsIter->IsConflict() || rhsIter->IsConflict();
    done = !lhsIter->HasNext() || !rhsIter->HasNext();
    return SUCCESS;
}