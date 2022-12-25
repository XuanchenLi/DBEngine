#include <vector>
#include "RM/RM_TableHandler.h"
#include "RM/RM_TblIterator.h"
#include "QM/QM_Manager.h"
#include "RelAttrUtil.h"
#include "main.h"



bool ValidAttr(const std::vector<MRelAttr>& attrs) {
    RM_TableHandler tHdl((DBT_DIR + COL_DIC_NAME).c_str());
    RM_TblIterator iter;

    std::string dbName = WORK_DIR;
    dbName = dbName.substr(0, dbName.length() - 1);
    dbName = dbName.substr(dbName.find_last_of('/') + 1);

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
        tHdl.GetIter(iter);
        iter.SetLimits(lims);
        //rec = iter.NextRec();
        if (!iter.HasNext()) {
            printf("Attribute Not Found.\n");
            return false;
        }
        lims.pop_back();
        lims.pop_back();
    }
    return true;

}


bool ValidRel(const std::vector<std::string>& rels) {
    RM_TableHandler tHdl((DBT_DIR + TBL_DIC_NAME).c_str());
    RM_TblIterator iter;

    std::string dbName = WORK_DIR;
    dbName = dbName.substr(0, dbName.length() - 1);
    dbName = dbName.substr(dbName.find_last_of('/') + 1);

    RM_Record rec;
    std::vector<DB_Opt> lims;
    DB_Opt opt;
    opt.colName = "dbName";
    opt.optr = EQUAL;
    opt.type = DB_STRING;
    strcpy(opt.data.sData, dbName.c_str());
    lims.push_back(opt);
    for (auto item : rels) {
        opt.colName = "tblName";
        strcpy(opt.data.sData, item.c_str());
        lims.push_back(opt);
        tHdl.GetIter(iter);
        iter.SetLimits(lims);
        //rec = iter.NextRec();
        if (!iter.HasNext()) {
            printf("Relation Not Found.\n");
            return false;
        }
        lims.pop_back();
    }
    return true;

}

RC ExpandAttrs(std::vector<MRelAttr>& attrs, const std::vector<std::string>& rels) {

    RM_TableHandler tHdl((DBT_DIR + COL_DIC_NAME).c_str());
    RM_TblIterator iter;
    RM_TblMeta meta = tHdl.GetMeta();

    std::string dbName = WORK_DIR;
    dbName = dbName.substr(0, dbName.length() - 1);
    dbName = dbName.substr(dbName.find_last_of('/') + 1);
    //std::cout<<dbName<<std::endl;
    RM_Record rec;
    std::vector<DB_Opt> lims;
    DB_Opt opt;
    opt.colName = "dbName";
    opt.optr = EQUAL;
    opt.type = DB_STRING;
    strcpy(opt.data.sData, dbName.c_str());
    lims.push_back(opt);
    char attrName[64];
    std::string tmpN;
    for (auto item : rels) {
        //std::cout<<item<<std::endl;
        opt.colName = "tblName";
        strcpy(opt.data.sData, item.c_str());
        lims.push_back(opt);
        tHdl.GetIter(iter);
        iter.SetLimits(lims);
        if (!iter.HasNext()) {
            std::cout<<"Table "<<item<<" Not Exist.\n";
            return FAILURE;
        }
        while (iter.HasNext()) {
            rec = iter.NextRec();
            rec.GetColData(meta, meta.GetPosByName("colName"), attrName);
            tmpN = attrName;
            attrs.push_back(
                MRelAttr(item, tmpN)
            );
        }
        lims.pop_back();
    }
    //std::cout<<"hhh\n";
    return SUCCESS;
}