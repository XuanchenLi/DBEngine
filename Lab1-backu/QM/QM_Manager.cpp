#include <iostream>
#include <algorithm>
#include "QM_Manager.h"
#include "main.h"
#include "RM/RM_TableHandler.h"
#include "IM/IM_Manager.h"
#include "RM/RM_TblIterator.h"


bool QM_Manager::ValidAttr(const std::vector<RelAttr>& attrs) {
    RM_TableHandler tHdl((DBT_DIR + COL_DIC_NAME).c_str());
    RM_TblIterator iter;

    std::string dbName = WORK_DIR;
    dbName = dbName.substr(0, dbName.length() - 1);
    dbName = dbName.substr(dbName.find_last_of('/'));

    RM_Record rec;
    std::vector<DB_Opt> lims;
    DB_Opt opt;
    opt.colName = "dbName";
    opt.optr = EQUAL;
    opt.type = DB_STRING;
    strcpy(opt.data.sData, dbName.c_str());
    lims.push_back(opt);
    for (auto item : attrs) {
        opt.colName = "tblName";
        strcpy(opt.data.sData, item.tblName.c_str());
        lims.push_back(opt);
        opt.colName = "colName";
        strcpy(opt.data.sData, item.colName.c_str());
        lims.push_back(opt);
        iter.SetLimits(lims);
        tHdl.GetIter(iter);
        rec = iter.NextRec();
        if (rec.rid.num == -1) {
            printf("Attribute Not Found.\n");
            return false;
        }
        lims.pop_back();
        lims.pop_back();
    }
    return true;

}


bool QM_Manager::ValidRel(const std::vector<std::string>& attrs) {
    RM_TableHandler tHdl((DBT_DIR + TBL_DIC_NAME).c_str());
    RM_TblIterator iter;

    std::string dbName = WORK_DIR;
    dbName = dbName.substr(0, dbName.length() - 1);
    dbName = dbName.substr(dbName.find_last_of('/'));

    RM_Record rec;
    std::vector<DB_Opt> lims;
    DB_Opt opt;
    opt.colName = "dbName";
    opt.optr = EQUAL;
    opt.type = DB_STRING;
    strcpy(opt.data.sData, dbName.c_str());
    lims.push_back(opt);
    for (auto item : attrs) {
        opt.colName = "tblName";
        strcpy(opt.data.sData, item.c_str());
        lims.push_back(opt);
        iter.SetLimits(lims);
        tHdl.GetIter(iter);
        rec = iter.NextRec();
        if (rec.rid.num == -1) {
            printf("Relation Not Found.\n");
            return false;
        }
        lims.pop_back();
    }
    return true;

}



RC QM_Manager::Select(std::vector<RelAttr>& selAttrs, std::vector<std::string>& relations, std::vector<DB_Cond>& conditions) {
    if (selAttrs.size() == 0 || relations.size() == 0) {
        return BAD_QUERY;
    }
    for (auto cond : conditions) {
        int flag = 0;
        for (auto rel: relations) {
            if (cond.lTblName == rel) {
                flag++;
            }
            if (cond.isConst && cond.rTblName == rel) {
                flag++;
            }
            if ((cond.isConst && flag == 2) || (!cond.isConst && flag == 1))
                break;
        }
        if ((cond.isConst && flag == 2) || (!cond.isConst && flag == 1))
            continue;
        else {
            return BAD_QUERY;
        }
    }
    for (auto selA : selAttrs) {
        bool flag = false;
        for (auto rel: relations) {
            if (selA.tblName == rel) {
                flag = true;
                break;
            }
        }
        if (!flag) {
            return BAD_QUERY;
        }
        
    }
    
    if (!ValidRel(relations)) {
        return FAILURE;
    }
        
    std::vector<RelAttr> allAttrs;
    allAttrs.insert(allAttrs.begin(), selAttrs.begin(), selAttrs.end());
    for (auto item : conditions) {
        allAttrs.push_back(RelAttr(item.lTblName, item.lColName));
        if (!item.isConst) {
            allAttrs.push_back(RelAttr(item.rTblName, item.rColName));
        }
    }
    std::sort(allAttrs.begin(), allAttrs.end());
    allAttrs.erase(std::unique(allAttrs.begin(), allAttrs.end()), allAttrs.end());
    if (!ValidAttr(allAttrs)) {
        return FAILURE;
    }
    
    if (relations.size() == 1) {
        DB_Opt cond;
        std::vector<DB_Opt> lims;
        for (auto item : conditions) {
            if (item.isConst) {
                cond.optr = item.optr;
                cond.type = item.type;
                cond.colName = item.lColName;
                memcpy(cond.data.sData, item.data.sData, sizeof(cond.data));
                lims.push_back(cond);
            }
        }
        std::string rel = relations[0];
        std::vector<int>avlIdx;
        IM_Manager iMgr;
        //iMgr.GetAvailIndex(rel, avlIdx);
        if (avlIdx.empty()) {
            //printf("");
            RM_TableHandler tHandler((WORK_DIR + rel).c_str());
            auto meta = tHandler.GetMeta();
            std::vector<RelNumAttr> numSelAttrs;
            RelNumAttr numAttr;
            numAttr.tblName = rel;
            for (auto i : selAttrs) {
                numAttr.colPos = meta.GetPosByName(i.colName);
                int id = meta.GetIdxByName(i.colName);
                numAttr.len = meta.length[id];
                numAttr.type = meta.type[id];
                numSelAttrs.push_back(numAttr);
            }
            RM_TblIterator iter;
            iter.SetLimits(lims);
            tHandler.GetIter(iter);
            auto rec = iter.NextRec();
            while(rec.rid.num != -1) {
                for (auto a : numSelAttrs) {
                    rec.GetColData(meta, a.colPos, &a.data.sData);
                    switch (a.type) {
                        case DB_INT:
                            printf("%d ", a.data.iData);
                            break;
                        case DB_BOOL:
                            if (a.data.bData) {
                                printf("True ");
                            }else {
                                printf("False ");
                            }
                            break;
                        case DB_DOUBLE:
                            printf("%lf ", a.data.lfData);
                            break;
                        case DB_STRING:
                            printf("%64s ", a.data.sData);
                            break;
                    }
                }
                puts("");
                rec = iter.NextRec();
            }
            
        }else {

        }
    }

}