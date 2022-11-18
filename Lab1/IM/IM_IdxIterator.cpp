#include <iostream>
#include <algorithm>
#include <string>
#include "IM_IdxIterator.h"
#include "FM/FM_Manager.h"
#include "RM/RM_RecHeader.h"
#include "RM/RM_TableHandler.h"
#include "main.h"

extern FM_Manager *fM_Manager;
void IM_IdxIterator::StandardizeLimits() {
    conflict = false;
    if (limits.size() <= 1)
        return;
    std::vector<DB_NumOpt> opts[6];
    for (auto i : limits) {
        opts[i.optr - 1].push_back(i);
    }
    for (int i = 0; i < 6; ++i) {
        std::sort(opts[i].begin(), opts[i].end());
        opts[i].erase(std::unique(opts[i].begin(), opts[i].end()), opts[i].end());
    }
    if (opts[EQUAL - 1].size() >= 2) {
        conflict = true;
        return;
    }
    if (opts[LESS - 1].size() >= 2) {
        auto t = opts[LESS - 1].front();
        opts[LESS - 1].clear();
        opts[LESS - 1].push_back(t);
    }
    if (opts[GREATER - 1].size() >= 2) {
        auto t = opts[GREATER - 1].back();
        opts[GREATER - 1].clear();
        opts[GREATER - 1].push_back(t);
    }
    if (opts[NOT_LESS - 1].size() >= 2) {
        auto t = opts[NOT_LESS - 1].back();
        opts[NOT_LESS - 1].clear();
        opts[NOT_LESS - 1].push_back(t);
    }
    if (opts[NOT_GREATER - 1].size() >= 2) {
        auto t = opts[NOT_GREATER - 1].front();
        opts[NOT_GREATER - 1].clear();
        opts[NOT_GREATER - 1].push_back(t);
    }
    if (!opts[LESS - 1].empty() && !opts[NOT_GREATER - 1].empty()) {
        if (comp(opts[LESS - 1][0].type, NOT_GREATER, &opts[LESS - 1][0], &opts[NOT_GREATER - 1][0])) {
            opts[NOT_GREATER - 1].clear();
        }
        else {
            opts[LESS - 1].clear();
        }
    }
    if (!opts[GREATER - 1].empty() && !opts[NOT_LESS - 1].empty()) {
        if (comp(opts[GREATER - 1][0].type, NOT_LESS, &opts[GREATER - 1][0], &opts[NOT_LESS - 1][0])) {
            opts[NOT_LESS - 1].clear();
        }
        else {
            opts[GREATER - 1].clear();
        }
    }
    if (!opts[EQUAL - 1].empty() && !opts[NOT_EQUAL - 1].empty() && comp(opts[EQUAL - 1][0].type, EQUAL, &opts[EQUAL - 1][0], &opts[NOT_EQUAL - 1][0])) {
        conflict = true;
        return;
    }
    if (!opts[LESS - 1].empty() && !opts[GREATER - 1].empty() && comp(opts[LESS - 1][0].type, NOT_GREATER, &opts[LESS - 1][0], &opts[GREATER - 1][0])) {
        conflict = true;
        return;
    }
    if (!opts[LESS - 1].empty() && !opts[NOT_LESS - 1].empty() && comp(opts[LESS - 1][0].type, NOT_GREATER, &opts[LESS - 1][0], &opts[NOT_LESS - 1][0])) {
        conflict = true;
        return;
    }
    if (!opts[NOT_GREATER - 1].empty() && !opts[GREATER - 1].empty() && comp(opts[NOT_GREATER - 1][0].type, NOT_GREATER, &opts[NOT_GREATER - 1][0], &opts[GREATER - 1][0])) {
        conflict = true;
        return;
    }
    if (!opts[NOT_GREATER - 1].empty() && !opts[NOT_LESS - 1].empty() && comp(opts[NOT_GREATER - 1][0].type, LESS, &opts[NOT_GREATER - 1][0], &opts[NOT_LESS - 1][0])) {
        conflict = true;
        return;
    }
    limits.clear();
    for (int i = 0; i < 6; ++i) {
        limits.insert(limits.end(), opts[i].begin(), opts[i].end());
    }

}


RC IM_IdxIterator::Reset() {
    done = false;
    int gtOptIdx = -1, nlOptIdx = -1;
    for (int i = 0; i < limits.size(); ++i) {
        if (limits[i].optr == GREATER) {
            gtOptIdx = i;
        }
        else if (limits[i].optr == NOT_LESS) {
            nlOptIdx = i;
        }
    }
    if (gtOptIdx != -1 || nlOptIdx != -1) {
        int idx = gtOptIdx;
        if (idx == -1)
            idx = nlOptIdx;
        idxHandler.GetQueryLeaf(limits[idx].data.sData ,curLeaf);
        while(true) {
            for (int i = 0; i < curLeaf.GetKeyNum(); ++i) {
                bool flag = true;
                for (auto it : limits) {
                    if (!it.test(curLeaf.GetKey(i))) {
                        flag = false;
                        break;
                    }
                }
                if (flag) {
                    curSlot = i;
                    return SUCCESS;
                }
            }
            if (curLeaf.GetKeyNum() == curLeaf.GetPtrNum()) {
                done = true;
                return SUCCESS;
            }else {
                idxHandler.GetNextLeaf(curLeaf);
            }
        }

    }else {
        idxHandler.GetFirstLeaf(curLeaf);
        curSlot = 0;
    }
    return SUCCESS;
}

std::pair<void*, RM_Rid> IM_IdxIterator::NextPair() {
    if (done || conflict) {
        return std::make_pair(nullptr, RM_Rid());
    }
    auto res = std::make_pair(curLeaf.GetKey(curSlot), curLeaf.GetPtr(curSlot));
    while(true) {
            for (int i = curSlot + 1; i < curLeaf.GetKeyNum(); ++i) {
                bool flag = true;
                for (auto it : limits) {
                    if (!it.test(curLeaf.GetKey(i))) {
                        flag = false;
                        break;
                    }
                }
                if (flag) {
                    curSlot = i;
                    return res;
                }
            }
            if (curLeaf.GetKeyNum() == curLeaf.GetPtrNum()) {
                done = true;
                break;
            }else {
                idxHandler.GetNextLeaf(curLeaf);
            }
        }
    return res;
}

RM_Record IM_IdxIterator::NextRec() {
    RM_Record res;
    res.rid.num = -1;
    auto pr = NextPair();
    if (pr.first == nullptr) {
        return res;
    }

    std::string path = idxHandler.GetIdxPath();
    if (path[path.length() - 1] == '/') {
        path = path.substr(0, path.length() - 1);
    }  
    //std::cout<<tmp<<std::endl;
    int idx = path.find_last_of('/');
    if (idx != std::string::npos) {
        path = path.substr(0, idx);
    }
    RM_TableHandler tHandler(path.c_str());
 
    if (tHandler.GetFileHdr().blkCnt <= pr.second.num)
    {
        return res;
    }
    res.rid = pr.second;
    MM_PageHandler pHdr;
    RM_RecHdr tmpRHdr;
    int off = sizeof(MM_PageHdr) + res.rid.slot * sizeof(RM_RecHdr);
    gBuffer->GetPage(FM_Bid(tHandler.GetFd(), res.rid.num), pHdr);
    memcpy(&tmpRHdr, pHdr.GetPtr(off), sizeof(RM_RecHdr));
    res.len = tmpRHdr.len;
    res.addr = pHdr.GetPtr(tmpRHdr.off);
    res.InitPrefix(tHandler.GetMeta());
    return res;
}