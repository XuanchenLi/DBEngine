#include <iostream>
#include <algorithm>
#include "QM_Manager.h"
#include "main.h"
#include "RM/RM_TableHandler.h"
#include "IM/IM_Manager.h"
#include "RM/RM_TblIterator.h"
#include "utils/RelAttrUtil.h"


RC QM_Manager::Select(std::vector<MRelAttr>& selAttrs, std::vector<std::string>& relations, std::vector<DB_Cond>& conditions) {
    if (selAttrs.size() == 0 || relations.size() == 0) {
        return BAD_QUERY;
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
    if (!ValidAttr(allAttrs)) {
        return FAILURE;
    }
    //生成计划
    DB_Iterator* planRoot = generator->generate(
        selAttrs,
        relations,
        conditions
    );
    

}
