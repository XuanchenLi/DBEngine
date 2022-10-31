#include "IM_Manager.h"
#include "FM/FM_Manager.h"
#include "RM/RM_TableHandler.h"
#include "RM/RM_TblIterator.h"
#include "main.h"
#include "utils/stringUtil.h"
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <tuple>
#include <iostream>


RC IM_Manager::FindIndex(const char* tblPath, int colPos) const {
    auto name = GetDbTblName(tblPath);

    //数据字典查索引项
    RM_TableHandler tblHdl((DBT_DIR + IDX_DIC_NAME).c_str());
    auto meta = tblHdl.GetMeta();
    RM_TblIterator iter;
    tblHdl.GetIter(iter);

    std::vector<DB_Opt> lims;
    DB_Opt opt;
    opt.colName = "dbName";
    opt.optr = EQUAL;
    opt.type = DB_STRING;
    strcpy(opt.data.sData, name.first.c_str());
    lims.push_back(opt);
    opt.colName = "tblName";
    opt.optr = EQUAL;
    opt.type = DB_STRING;
    strcpy(opt.data.sData, name.second.c_str());
    lims.push_back(opt);
    opt.colName = "colPos";
    opt.optr = EQUAL;
    opt.type = DB_INT;
    opt.data.iData = colPos;
    lims.push_back(opt);
    iter.SetLimits(lims);

    RM_Record rec;
    rec = iter.NextRec();
    tblHdl.CloseTbl();
    return rec.rid.num;
}


RC IM_Manager::CreateIndex(const char* tblPath, int colPos) {
    int rc = FindIndex(tblPath, colPos);
    if (rc != -1) {
        std::cout<<"Index on col "<<colPos<<" is exist.\n";
        return SUCCESS;
    }
    auto name = GetDbTblName(tblPath);
    //创建数据字典文件
    int st = fM_Manager->CreateTblFile((tblPath + ("_" + std::to_string(colPos))).c_str());
    if (st != SUCCESS)
        return st;

    //插入数据字典项
    RM_RecAux rAux;
    rAux.strValue.push_back(
        std::make_pair("dbName",  name.first)
    );
    rAux.strValue.push_back(
        std::make_pair("tblName",  name.second)
    );
    rAux.iValue.push_back(
        std::make_pair("colPos", colPos)
    );
    RM_TableHandler tblHdl((DBT_DIR + IDX_DIC_NAME).c_str());
    tblHdl.InsertRec(rAux);
    tblHdl.CloseTbl();

    IM_IdxHandler iHdl((tblPath + ("_" + std::to_string(colPos))).c_str());
    RM_TblIterator iter;
    tblHdl.OpenTbl(tblPath);
    tblHdl.GetIter(iter);
    RM_Record rec;
    rec = iter.NextRec();
    
    while(rec.rid.num != -1) {
        void* tp = nullptr;
        switch(iHdl.GetType()) {
        case DB_INT:
            tp = new int;
            break;
        case DB_DOUBLE:
            tp = new double;
            break;
        case DB_STRING:
            tp = new char[iHdl.GetLen()];
            break;
        case DB_BOOL:
            tp = new bool;
            break;
        }
        rec.GetColData(tblHdl.GetMeta(), colPos, tp);
        //printf("%s\n", tp);
        iHdl.InsertEntry(tp, rec.rid);

        rec = iter.NextRec();
    }
    
    tblHdl.CloseTbl();
    iHdl.CloseIdx();

    return SUCCESS;

}
RC IM_Manager::ClearIndex(const char* tblPath, int colPos) {
    int rc = FindIndex(tblPath, colPos);
    if (rc == -1) {
        std::cout<<"Index on col "<<colPos<<" is not exist.\n";
        return SUCCESS;
    }
    auto name = GetDbTblName(tblPath);

    //插入数据字典项
    RM_TableHandler tblHdl;
    IM_IdxHandler iHdl((tblPath + ("_" + std::to_string(colPos))).c_str());
    RM_TblIterator iter;
    tblHdl.OpenTbl(tblPath);
    tblHdl.GetIter(iter);
    RM_Record rec;
    rec = iter.NextRec();

    void* tp = nullptr;
    switch(iHdl.GetType()) {
        case DB_INT:
            tp = new int;
            break;
        case DB_DOUBLE:
            tp = new double;
            break;
        case DB_STRING:
            tp = new char[iHdl.GetLen()];
            break;
        case DB_BOOL:
            tp = new bool;
            break;
    }
    
    while(rec.rid.num != -1) {
        
        rec.GetColData(tblHdl.GetMeta(), colPos, tp);
        //printf("%s\n", tp);
        iHdl.DeleteEntry(tp, rec.rid);
        rec = iter.NextRec();
    }
    
    tblHdl.CloseTbl();
    iHdl.CloseIdx();
    return SUCCESS;
}


RC IM_Manager::DestroyIndex(const char* tblPath, int colPos) {
    int rc = FindIndex(tblPath, colPos);
    if (rc == -1) {
        std::cout<<"Index on col "<<colPos<<" not exist.\n";
        return SUCCESS;
    }
    //删除文件
    int st = fM_Manager->DeleteFile((tblPath + ("_" + std::to_string(colPos))).c_str());
    if (st != SUCCESS)
        return st;


    //TODO 删除数据字典索引项


    return SUCCESS;
}


RC IM_Manager::OpenIndex(const char* tblPath, int colPos,  IM_IdxHandler& handler) {
    int rc = FindIndex(tblPath, colPos);
    if (rc == -1) {
        std::cout<<"Index on col "<<colPos<<" not exist.\n";
        return NOT_EXIST;
    }
    rc = handler.OpenIdx((tblPath + ("_" + std::to_string(colPos))).c_str());
    return rc;
}
RC IM_Manager::CloseIndex(IM_IdxHandler& handler) {
    return handler.CloseIdx();
}

RC IM_Manager::TraverseLeaf(const char* tblPath, int colPos) {
    IM_IdxHandler iHdl((tblPath + ("_" + std::to_string(colPos))).c_str());
    iHdl.Traverse();
    return iHdl.CloseIdx();
}
