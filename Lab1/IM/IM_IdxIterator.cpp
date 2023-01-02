#include <iostream>
#include <algorithm>
#include <string>
#include "IM_IdxIterator.h"
#include "FM/FM_Manager.h"
#include "RM/RM_RecHeader.h"
#include "RM/RM_TableHandler.h"
#include "RM/RM_TblIterator.h"
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
        if (comp(opts[LESS - 1][0].type, NOT_GREATER, &opts[LESS - 1][0].data.sData, &opts[NOT_GREATER - 1][0].data.sData)) {
            opts[NOT_GREATER - 1].clear();
        }
        else {
            opts[LESS - 1].clear();
        }
    }
    if (!opts[GREATER - 1].empty() && !opts[NOT_LESS - 1].empty()) {
        if (comp(opts[GREATER - 1][0].type, NOT_LESS, &opts[GREATER - 1][0].data.sData, &opts[NOT_LESS - 1][0].data.sData)) {
            opts[NOT_LESS - 1].clear();
        }
        else {
            opts[GREATER - 1].clear();
        }
    }
    if (!opts[EQUAL - 1].empty() && !opts[NOT_EQUAL - 1].empty() && comp(opts[EQUAL - 1][0].type, EQUAL, &opts[EQUAL - 1][0].data.sData, &opts[NOT_EQUAL - 1][0].data.sData)) {
        conflict = true;
        return;
    }
    if (!opts[LESS - 1].empty() && !opts[GREATER - 1].empty() && comp(opts[LESS - 1][0].type, NOT_GREATER, &opts[LESS - 1][0].data.sData, &opts[GREATER - 1][0].data.sData)) {
        conflict = true;
        return;
    }
    if (!opts[LESS - 1].empty() && !opts[NOT_LESS - 1].empty() && comp(opts[LESS - 1][0].type, NOT_GREATER, &opts[LESS - 1][0].data.sData, &opts[NOT_LESS - 1][0].data.sData)) {
        conflict = true;
        return;
    }
    if (!opts[NOT_GREATER - 1].empty() && !opts[GREATER - 1].empty() && comp(opts[NOT_GREATER - 1][0].type, NOT_GREATER, &opts[NOT_GREATER - 1][0].data.sData, &opts[GREATER - 1][0].data.sData)) {
        conflict = true;
        return;
    }
    if (!opts[NOT_GREATER - 1].empty() && !opts[NOT_LESS - 1].empty() && comp(opts[NOT_GREATER - 1][0].type, LESS, &opts[NOT_GREATER - 1][0].data.sData, &opts[NOT_LESS - 1][0].data.sData)) {
        conflict = true;
        return;
    }
    if (!opts[EQUAL - 1].empty()) {
        auto t = opts[EQUAL - 1][0].type;
        if (!opts[NOT_LESS - 1].empty() && comp(t, LESS, &opts[EQUAL - 1][0].data.sData, &opts[NOT_LESS - 1][0].data.sData)) {
            conflict = true;
            return;
        }
        if (!opts[GREATER - 1].empty() && comp(t, NOT_GREATER, &opts[EQUAL - 1][0].data.sData, &opts[GREATER - 1][0].data.sData)) {
            conflict = true;
            return;
        }
        if (!opts[LESS - 1].empty() && comp(t, NOT_LESS, &opts[EQUAL - 1][0].data.sData, &opts[LESS - 1][0].data.sData)) {
            conflict = true;
            return;
        }
        if (!opts[NOT_GREATER - 1].empty() && comp(t, GREATER, &opts[EQUAL - 1][0].data.sData, &opts[NOT_GREATER - 1][0].data.sData)) {
            conflict = true;
            return;
        }
    }
    limits.clear();
    for (int i = 0; i < 6; ++i) {
        limits.insert(limits.end(), opts[i].begin(), opts[i].end());
    }

}

RM_Record IM_IdxIterator::GetRecord(std::pair<void*, RM_Rid>& pr) {
    RM_Record res;
    RM_TableHandler tHandler(tblPath.c_str());
    //std::cout<<path<<std::endl;
    if (tHandler.GetFileHdr().blkCnt <= pr.second.num)
    {
        std::cout<<"block " << pr.second.num <<" out of range "<<tHandler.GetFileHdr().blkCnt<<std::endl;
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



RC IM_IdxIterator::Reset() {
    if (hasReseted)
        return SUCCESS;
    //DB_Iterator::Reset();
    hasReseted = true;
    done = false;
    gtOptIdx = -1, nlOptIdx = -1, eqOptIdx = -1, lOptIdx = -1, ngOptIdx = -1;
    for (int i = 0; i < limits.size(); ++i) {
        if (limits[i].optr == GREATER) {
            gtOptIdx = i;
        }
        else if (limits[i].optr == NOT_LESS) {
            nlOptIdx = i;
        }else if (limits[i].optr == EQUAL) {
            //printf("sasdas\n");
            eqOptIdx = i;
        }else if (limits[i].optr == LESS) {
            lOptIdx = i;
        }else if (limits[i].optr == NOT_GREATER) {
            ngOptIdx = i;
        }
    }
    if (eqOptIdx != -1) {
        idxHandler.GetQueryLeaf(limits[eqOptIdx].data.sData ,curLeaf);
    }
    else if (gtOptIdx != -1 || nlOptIdx != -1) {
        int idx = gtOptIdx;
        if (idx == -1)
            idx = nlOptIdx;
        idxHandler.GetQueryLeaf(limits[idx].data.sData ,curLeaf);

    }else {
        idxHandler.GetFirstLeaf(curLeaf);
        
    }
    curSlot = 0;
    while(true) {
            for (int i = 0; i < curLeaf.GetKeyNum(); ++i) {
                bool flag = true;
                for (auto it : limits) {
                    //printf("fasf\n");
                    if (!it.test(curLeaf.GetKey(i))) {
                        flag = false;
                        break;
                    }
                }
                if (flag) {
                    curSlot = i;
                    if (!outerLimits.empty()) {
                        auto pr = std::make_pair(curLeaf.GetKey(curSlot), curLeaf.GetPtr(curSlot));
                        RM_Record rec = GetRecord(pr);
                        if (rec.valid(meta, outerLimits)) {
                            return SUCCESS;
                        }else {
                            continue;
                        }
                    }
                    return SUCCESS;
                }
            }
            if (curLeaf.GetKeyNum() == curLeaf.GetPtrNum()) {
                done = true;
                return SUCCESS;
            }else {
                idxHandler.GetNextLeaf(curLeaf);
                curSlot = 0;
                if (eqOptIdx != -1 && comp(attrType, LESS, limits[eqOptIdx].data.sData, curLeaf.GetKey(0))) {
                    done = true;
                    return SUCCESS;
                }else if (lOptIdx != -1 && comp(attrType, NOT_GREATER, limits[lOptIdx].data.sData, curLeaf.GetKey(0))) {
                    done = true;
                    return SUCCESS;
                }else if (ngOptIdx != -1 && comp(attrType, LESS, limits[ngOptIdx].data.sData, curLeaf.GetKey(0))) {
                    done = true;
                    return SUCCESS;
                }

            }
    }
    return SUCCESS;
}

std::pair<void*, RM_Rid> IM_IdxIterator::NextPair() {
    if (done || conflict) {
        return std::make_pair(nullptr, RM_Rid());
    }
    hasReseted = false;
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
                    if (!outerLimits.empty()) {
                        auto pr = std::make_pair(curLeaf.GetKey(curSlot), curLeaf.GetPtr(curSlot));
                        RM_Record rec = GetRecord(pr);
                        if (rec.valid(meta, outerLimits)) {
                            return res;
                        }else {
                            continue;
                        }
                    }
                    return res;
                }
            }
            if (curLeaf.GetKeyNum() == curLeaf.GetPtrNum()) {
                //printf("done\n");
                done = true;
                break;
            }else {
                idxHandler.GetNextLeaf(curLeaf);
                curSlot = -1;
                if (eqOptIdx != -1 && comp(attrType, LESS, limits[eqOptIdx].data.sData, curLeaf.GetKey(0))) {
                    //printf("asd\n");
                    done = true;
                    break;
                }else if (lOptIdx != -1 && comp(attrType, NOT_GREATER, limits[lOptIdx].data.sData, curLeaf.GetKey(0))) {
                    done = true;
                    break;
                }else if (ngOptIdx != -1 && comp(attrType, LESS, limits[ngOptIdx].data.sData, curLeaf.GetKey(0))) {
                    done = true;
                    break;
                }
            }
        }
    return res;
}


RM_Record IM_IdxIterator::NextRec() {
    RM_Record res;
    res.rid.num = -1;
    auto pr = NextPair();
    if (pr.first == nullptr) {
        std::cout<<"null pointer\n";
        return res;
    }
    return GetRecord(pr);
}

RC IM_IdxIterator::SetLimits(const std::vector<DB_Opt>& rawLim) {
    std::vector<DB_NumOpt> nLims;

    RM_TableHandler tHdl((DBT_DIR + COL_DIC_NAME).c_str());
    RM_TblIterator iter;

    std::string dbName = WORK_DIR;
    dbName = dbName.substr(0, dbName.length() - 1);
    dbName = dbName.substr(dbName.find_last_of('/') + 1);

    RM_Record rec;
    std::vector<DB_Opt> lims;
    DB_Opt cond;
    cond.colName = "dbName";
    cond.optr = EQUAL;
    cond.type = DB_STRING;
    strcpy(cond.data.sData, dbName.c_str());
    lims.push_back(cond);
    cond.colName = "tblName";
    std::string path = idxHandler.GetIdxPath();
    //std::cout<<path<<std::endl;
    if (path[path.length() - 1] == '/') {
        path = path.substr(0, path.length() - 1);
    }  
    //std::cout<<tmp<<std::endl;
    int idx = path.find_last_of('/');
    if (idx != std::string::npos) {
        path = path.substr(idx + 1);
    }
    path = path.substr(0, path.find_last_of('_'));
    //std::cout<<dbName<<" "<<path<<std::endl;
    strcpy(cond.data.sData, path.c_str());
    lims.push_back(cond);
    cond.colName = "colName";
    DB_NumOpt opt;
    //RM_Record rec;
    for (auto item : rawLim) {
        opt.optr = item.optr;
        opt.type = item.type;
        memcpy(&opt.data.sData, &item.data.sData, sizeof(item.data));
        strcpy(cond.data.sData, item.colName.c_str());
        lims.push_back(cond);
        tHdl.GetIter(iter);
        iter.SetLimits(lims);
        //rec = iter.NextRec();
        if (!iter.HasNext()) {
            printf("Attribute Not Found.\n");
            return false;
        }
        rec = iter.NextRec();
        rec.GetColData(tHdl.GetMeta(), 5, &opt.colPos);
        nLims.push_back(opt);
        lims.pop_back();
    }
    std::cout<<nLims.size()<<std::endl;
    return SetLimits(nLims);

}