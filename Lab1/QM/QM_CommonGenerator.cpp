#include "QM_CommonGenerator.h"
#include "main.h"
#include "utils/DB_Option.h"
#include "IM/IM_Manager.h"
#include "IM/IM_IdxHandler.h"
#include "IM/IM_IdxIterator.h"
#include "RM/RM_TableHandler.h"
#include "RM/RM_TblIterator.h"
#include "QNodes/IndexDirectAccessNode.h"
#include "QNodes/NestedLoopJoinNode.h"
#include "QNodes/ProjectionNode.h"
#include <map>
#include <algorithm>

ProjectionNode* FormProjectionNode(const RM_TblMeta& relMeta, const std::vector<std::string>&selAttrs) {
    ProjectionNode* pIter = new ProjectionNode();
    RM_TblMeta pMeta;
    int num = 0;
    for (auto attr : selAttrs) {
        int idx = relMeta.GetIdxByName(attr);
        pMeta.isDynamic[num] = relMeta.isDynamic[idx];
        pMeta.colPos[num] = num;
        pMeta.type[num] = relMeta.type[idx];
        pMeta.length[num] = relMeta.length[idx];
        pMeta.colName[num] = attr;
        num++;
    }
    pMeta.dbName = relMeta.dbName;
    pMeta.tblName = relMeta.tblName;
    pMeta.colNum = num;
    pIter->SetMeta(pMeta);
    return pIter;
}
NestedLoopJoinNode* FormNestedLoopJoinNode(DB_Iterator* lhsIter, DB_Iterator* rhsIter,
                                const std::vector<MRelAttr>& rSelAttrs, 
                                const std::vector<DB_Cond>& conds) {
    std::vector<DB_JoinOpt> joinConds;
    for (auto cond : conds) {
        joinConds.push_back(TransToJoinOpt(cond));
    }
    //std::cout<<conds.size()<<std::endl;
    std::vector<std::string>lN, rN;
    auto lhsMeta = lhsIter->GetMeta();
    auto rhsMeta = rhsIter->GetMeta();
    for (int i = 0; i < lhsMeta.colNum; ++i)
        lN.push_back(lhsMeta.colName[i]);
    for (int i = 0; i < rhsMeta.colNum; ++i)
        rN.push_back(rhsMeta.colName[i]);
    RM_TblMeta joinMeta = lhsMeta;
    for (auto attr : rSelAttrs) {
        int idx = joinMeta.colNum;
        int srcIdx = rhsMeta.GetIdxByName(attr.colName);
        joinMeta.isDynamic[idx] = rhsMeta.isDynamic[srcIdx];
        joinMeta.length[idx] = rhsMeta.length[srcIdx];
        joinMeta.type[idx] = rhsMeta.type[srcIdx];
        joinMeta.colName[idx] = rhsMeta.colName[srcIdx];
        joinMeta.colPos[idx] = idx;
        joinMeta.colNum++;
    }
    NestedLoopJoinNode* nIter = new NestedLoopJoinNode();
    nIter->SetSrcIter(lhsIter, rhsIter);
    nIter->SetNames(lN, rN);
    nIter->SetMeta(joinMeta);
    nIter->SetLimits(joinConds);
    nIter->Reset();
    return nIter;
}


DB_Iterator* QM_CommonGenerator::generate(const std::vector<MRelAttr>& selAttrs, 
                        const std::vector<std::string>& relations, 
                        const std::vector<DB_Cond>& conditions) {
    //std::cout<<conditions.size()<<std::endl;
    std::vector<DB_Iterator*> iterVec = generateLeaf(
        selAttrs, relations, conditions
    );
    if (iterVec.size() == 1)
        return iterVec[0];
    std::vector<MRelAttr> rSelectedAttrs;
    std::vector<DB_Cond> relatedConds;
    std::map<std::string, int> dupName;
    auto meta0 = iterVec[0]->GetMeta();
    for  (int i = 0; i < meta0.colNum; ++i) {
        dupName[meta0.colName[i]] = 0;
    }
    for (int i = 1; i < iterVec.size(); ++i) {
        rSelectedAttrs.clear();
        for (auto attr : selAttrs) {
            if (attr.tblName == relations[i]) {
                rSelectedAttrs.push_back(attr);
            }
        }
        relatedConds.clear();
        for (auto cond : conditions) {
            if (!cond.isConst && cond.rTblName != cond.lTblName) {  //除去自约束和常量约束
                if (cond.lTblName == relations[i] ) {
                    swap(cond.lTblName, cond.rTblName);
                    swap(cond.lColName, cond.rColName);
                    cond.optr = InverseOptr(cond.optr);
                }
                relatedConds.push_back(cond);
            }
        }

        auto metai = iterVec[i]->GetMeta();
        for (int j = 0; j < metai.colNum; ++j) {
            if (dupName.find(metai.colName[j]) == dupName.end()) {
                dupName[metai.colName[j]] = 0;
            }else {
                //重名字段
                int id = ++dupName[metai.colName[j]];  //获取id
                std::string sfx = "(" + std::to_string(id) + ")";
                //修改属性名
                for (int k = 0; k < rSelectedAttrs.size(); ++k) {
                    if (rSelectedAttrs[k].colName == metai.colName[j]) {
                        rSelectedAttrs[k].colName += sfx;
                        break;
                    }
                }
                for (int k = 0; k < relatedConds.size(); ++k) {
                    if (relatedConds[k].rColName == metai.colName[j]) {
                       relatedConds[k].rColName += sfx;
                    }
                }
                metai.colName[j] += sfx;
            }
        }
        iterVec[i]->SetMeta(metai);
        //std::cout<<"ashddassha\n"<<std::endl;
        iterVec[0] = FormNestedLoopJoinNode(
            iterVec[0], iterVec[i], rSelectedAttrs, relatedConds
        );
    }

    return iterVec[0];
}


DB_Iterator* QM_CommonGenerator::generateOne(
        const std::string& relName,
        const std::vector<std::string>& selAttrs,
        const std::vector<DB_Cond>& conditions
    ) {
    RM_TableHandler tHandler((WORK_DIR + relName).c_str());
    auto relMeta = tHandler.GetMeta();
    std::vector<DB_Opt> singleConds;
    std::vector<std::string> uniqueSet;
    std::vector<std::string> condUniqueSet;
    for (auto attr: selAttrs) {
        uniqueSet.push_back(attr);
    }
    for (auto cond : conditions) {
        if (cond.lTblName == relName) {
            uniqueSet.push_back(cond.lColName);
            condUniqueSet.push_back(cond.lColName);
        }
        if (cond.rTblName == relName) {
            uniqueSet.push_back(cond.rColName);
            condUniqueSet.push_back(cond.rColName);
        }
        if (cond.isConst) {
            singleConds.push_back(TransToOpt(cond));
        }
        //自约束
    }
    //去重
    std::sort(uniqueSet.begin(), uniqueSet.end());
    auto it = std::unique(uniqueSet.begin(), uniqueSet.end());
    uniqueSet.erase(it, uniqueSet.end());
    std::vector<std::string>(uniqueSet).swap(uniqueSet);

    std::sort(condUniqueSet.begin(), condUniqueSet.end());
    it = std::unique(condUniqueSet.begin(),condUniqueSet.end());
    condUniqueSet.erase(it, condUniqueSet.end());
    std::vector<std::string>(condUniqueSet).swap(condUniqueSet);


    IM_Manager idxManager;
    std::vector<std::pair<std::string, int>> avlIdxList;
    idxManager.GetAvailIndex(relName, avlIdxList);
    //表上无索引，只可返回普通迭代器
    if (avlIdxList.empty()) {
        RM_TblIterator* iter = new RM_TblIterator();
        tHandler.GetIter(*iter);
        iter->SetLimits(singleConds);
        //std::cout<<singleConds[0].data.sData<<std::endl;
        if (selAttrs.size() == relMeta.colNum) {
            return iter;
        }
        ProjectionNode* pIter = FormProjectionNode(relMeta, selAttrs);
        pIter->SetSrcIter(iter);
        pIter->Reset();
        //printf("asasasd\n");
        return pIter;
    }
    //表上有索引
    if (uniqueSet.size() == 1) {  //查询只涉及一个属性
        std::string attrName = *uniqueSet.begin();
        int pos = -1;
        for (auto idxPair : avlIdxList) {
            if (idxPair.first == attrName) {
                pos = idxPair.second;
            }
        }
        if (pos == -1) {  //该属性上无索引
            //返回普通迭代器
            RM_TblIterator* iter = new RM_TblIterator();
            tHandler.GetIter(*iter);
            iter->SetLimits(singleConds);
            //std::cout<<singleConds.size();
            if (selAttrs.size() == relMeta.colNum) {
                return iter;
            }
            ProjectionNode* pIter = FormProjectionNode(relMeta, selAttrs);
            pIter->SetSrcIter(iter);
            pIter->Reset();
            return pIter;
        }else {  //该属性上有索引
            //返回索引直接访问迭代器
            IM_IdxHandler iHdl;
            idxManager.OpenIndex((WORK_DIR + relName).c_str(), pos, iHdl);
            IM_IdxIterator* idxIter = new IM_IdxIterator(iHdl.GetType(), iHdl.GetLen());
            iHdl.GetIter(*idxIter);
            idxIter->SetLimits(singleConds);
            IndexDirectAccessNode* idxAceNode = new IndexDirectAccessNode();
            idxAceNode->SetSrcIter(idxIter);
            RM_TblMeta meta;
            int attrIdx = relMeta.GetIdxByName(attrName);
            meta.dbName = relMeta.dbName;
            meta.tblName = relMeta.tblName;
            meta.isDynamic[0] = false;
            meta.colNum = 1;
            meta.colPos[0] = 0;
            meta.type[0] = relMeta.type[attrIdx];
            meta.length[0] = relMeta.length[attrIdx];
            meta.colName[0] = attrName;
            idxAceNode->SetMeta(meta);
            idxAceNode->Reset();
            return idxAceNode;
        }
    }else {  //查询涉及多个属性
        int idxPos = -1;
        if (!condUniqueSet.empty()) {
            for (auto idxPair : avlIdxList) {  //查找可用索引提前判断的约束
                //由于先前去重，此处可以二分查找
                if (std::binary_search(condUniqueSet.begin(), condUniqueSet.end(), idxPair.first)) {
                   idxPos = idxPair.second;
                   break; 
                }
            }
        }
        if (idxPos == -1) {
            //没有可以利用索引提前判断的约束，返回普通迭代器
            RM_TblIterator* iter = new RM_TblIterator();
            tHandler.GetIter(*iter);
            //std::cout<<singleConds[0].<<std::endl;
            iter->SetLimits(singleConds);
            if (selAttrs.size() == relMeta.colNum) {
                return iter;
            }
            ProjectionNode* pIter = FormProjectionNode(relMeta, selAttrs);
            pIter->SetSrcIter(iter);
            pIter->Reset();
            return pIter;
        }else {
            //利用索引提前判断部分约束
            //std::cout<<"hhh\n";
            IM_IdxHandler iHdl;
            idxManager.OpenIndex((WORK_DIR + relName).c_str(), idxPos, iHdl);
            IM_IdxIterator* idxIter = new IM_IdxIterator(iHdl.GetType(), iHdl.GetLen());
            iHdl.GetIter(*idxIter);
            std::vector<DB_Opt> idxConds;
            std::vector<DB_Opt> outerConds;
            std::string idxAttrName = relMeta.GetNameByPos(idxPos);
            for (auto cond : singleConds) {
                if (cond.colName == idxAttrName) {
                    idxConds.push_back(cond);
                }
                else {
                    outerConds.push_back(cond);
                }
            }
            idxIter->SetOuterLimits(outerConds);
            //std::cout<<idxConds.size()<<std::endl;
            idxIter->SetLimits(idxConds);
            if (selAttrs.size() == relMeta.colNum) {
                return idxIter;
            }
            ProjectionNode* pIter = FormProjectionNode(relMeta, selAttrs);
            pIter->SetSrcIter(idxIter);
            pIter->Reset();
            return pIter;
        }
    }

}
std::vector<DB_Iterator*> QM_CommonGenerator::generateLeaf(const std::vector<MRelAttr>& selAttrs, 
                        const std::vector<std::string>& relations, 
                        const std::vector<DB_Cond>& conditions) {
    std::vector<DB_Iterator*> resVec;
    std::vector<std::string> attrs;
    std::vector<DB_Cond> conds;
    for (auto relName : relations) {
        attrs.clear();
        conds.clear();
        for (auto attr : selAttrs) {
            if (attr.tblName == relName)
                attrs.push_back(attr.colName);
        }
        bool isAux = false;
        if (attrs.empty())
            isAux = true;
        for (auto cond : conditions) {
            if (cond.lTblName == relName || cond.rTblName == relName) {
                conds.push_back(cond);
                if (isAux) {
                    if (cond.lTblName == relName) {
                        attrs.push_back(cond.lColName);
                    }
                    if (cond.rTblName == relName) {
                        attrs.push_back(cond.rColName);
                    }
                }
            }
        }
        //std::cout<<"11212\n";
        resVec.push_back(
            generateOne(
                relName, attrs, conds
            )
        );
    }
    return resVec;
}