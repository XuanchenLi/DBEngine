#include <iostream>
#include <algorithm>
#include "QM_Manager.h"
#include "main.h"
#include "RM/RM_TableHandler.h"
#include "IM/IM_Manager.h"
#include "RM/RM_TblIterator.h"
#include "utils/RelAttrUtil.h"


RC QM_Manager::Select(std::vector<MRelAttr>& selAttrs, std::vector<std::string>& relations, std::vector<DB_Cond>& conditions) {
    //std::cout<<"112\n";
    if (selAttrs.size() == 0 || relations.size() == 0) {
        return BAD_QUERY;
    }
    if (relations.size() == 1) {
        for (int i = 0; i < selAttrs.size(); ++i) {
            selAttrs[i].tblName = relations[0];
        }
        for (int i = 0; i < conditions.size(); ++i) {
            conditions[i].lTblName = relations[0];
            if (!conditions[i].isConst)
                conditions[i].rTblName = relations[0];
        }
    }
    //检查属性
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
    //检查约束
    for (auto item : conditions) {
        int flag = 0;
        for (auto rel : relations) {
            if (item.lTblName == rel) {
                flag++;
                if (!item.isConst)
                    break;
            }
            if (!item.isConst && item.rTblName == rel) {
                flag++;
                if (flag == 2)
                    break;
            }
        }
        if ((item.isConst && flag == 1) || (!item.isConst && flag == 2))
            continue;
        else
            return BAD_QUERY;
    }
    //检查存在性
    std::cout<<"testing relations...\n";
    if (!ValidRel(relations)) {
        return NOT_EXIST;
    }
    std::vector<MRelAttr> allAttrs;
    allAttrs.insert(allAttrs.begin(), selAttrs.begin(), selAttrs.end());
    for (auto item : conditions) {
        allAttrs.push_back(MRelAttr(item.lTblName, item.lColName));
        if (!item.isConst) {
            allAttrs.push_back(MRelAttr(item.rTblName, item.rColName));
        }
    }
    std::sort(allAttrs.begin(), allAttrs.end());
    allAttrs.erase(std::unique(allAttrs.begin(), allAttrs.end()), allAttrs.end());
    std::cout<<"testing attributes...\n";
    if (!ValidAttr(allAttrs)) {
        return FAILURE;
    }
    std::cout<<"constructing plan...\n";
    //生成计划
    DB_Iterator* planRoot = generator->generate(
        selAttrs,
        relations,
        conditions
    );
    planRoot->Reset();
    std::cout<<"---------Plan---------\n";
    planRoot->Print(0);
    auto rMeta = planRoot->GetMeta();
    for (int i = 0; i < rMeta.colNum; ++i) {
        int idx = rMeta.GetIdxByPos(i);
        printf("%-32s", rMeta.colName[idx].c_str());
    }
    puts("");
    std::string li(32*(rMeta.colNum), '-');
    puts(li.c_str());
    RM_Record rec;
    int iData;
    double lfData;
    char sData[64];
    int cnt = 0;
    while(planRoot->HasNext()) {
        cnt++;
        rec = planRoot->NextRec();
        for (int i = 0; i < rMeta.colNum; ++i) {
            int idx = rMeta.GetIdxByPos(i);
            switch(rMeta.type[idx]) {
                case DB_INT:
                    rec.GetColData(rMeta, i, &iData);
                    printf("%-32d", iData);
                    break;
                case DB_DOUBLE:
                    rec.GetColData(rMeta, i, &lfData);
                    printf("%-32lf", lfData);
                    break;
                case DB_STRING:
                    rec.GetColData(rMeta, i, sData);
                    printf("%-32s", sData);
                    break;
            }
        }
        puts("");
    }
    puts(li.c_str());
    printf("%d rows in set\n", cnt);
    return SUCCESS;
}
