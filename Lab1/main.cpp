#include <unistd.h>
#include <iostream>
#include <string.h>
#include <sys/stat.h>
#include <tuple>
#include "FM/FM_Manager.h"
#include "RM/RM_TblIterator.h"
#include "main.h"
#include "IM/IM_Manager.h"
#include "MM/MM_Buffer.h"
#include "RM/RM_TblMeta.h"
#include "RM/RM_TableHandler.h"
#include "MM/MM_StrategyLRU.h"
#include "RM/RM_Record.h"

using namespace std;

unsigned int BLOCK_SIZE; //KB
unsigned int PREALLOC_SIZE; //MB
unsigned int BUFFER_SIZE;
FM_Manager* fM_Manager;
MM_Buffer* gBuffer;
std::string DBT_DIR = "./DB/db_schema/";
std::string WORK_DIR = "./DB/";
const char DIC_DB_NAME[] = "db_schema";
const char TBL_DIC_NAME[] = "tables"; 
const char COL_DIC_NAME[] = "columns"; 
const char IDX_DIC_NAME[] = "indexes";
RM_TblMeta TBL_DIC_META;
RM_TblMeta COL_DIC_META;

void dbClear();
void dbInit(int argc, char* argv[]);
void task1();
void task3();


int main(int argc, char* argv[]) {
    if (argc != 5) {
        cout<<"usage:./main [DBname] [preallocate size](MB) [buffer block number] [buffer block size](KB)"<<endl;
        exit(0);
    }
    dbInit(argc, argv);
    //----------------------------------
    task1();
    task3();
    //----------------------------------
    dbClear();

}

void dbClear() {
    //----------------------------------
    gBuffer->Clear();
    delete fM_Manager;
    delete gBuffer;
}

void dbInit(int argc, char* argv[]) {
    //--------创建数据库目录----------
    BLOCK_SIZE = (unsigned int)atoi(argv[4]) << 10;
    BUFFER_SIZE = (unsigned int)atoi(argv[3]);
    PREALLOC_SIZE = (unsigned int)atoi(argv[2])<< 20;
    gBuffer = new MM_Buffer(BUFFER_SIZE, new MM_StrategyLRU());
    fM_Manager = new FM_Manager();
    fM_Manager->CreateDir("./DB");
    fM_Manager->CreateDir(DBT_DIR.c_str());
    fM_Manager->CreateDir((WORK_DIR + argv[1]).c_str());

    //--------创建数据字典------------
    int status = fM_Manager->CreateTblFile((DBT_DIR + TBL_DIC_NAME).c_str());
    fM_Manager->CreateTblFile((DBT_DIR + COL_DIC_NAME).c_str());
    fM_Manager->CreateTblFile((DBT_DIR + IDX_DIC_NAME).c_str());

    //--------创建表文件--------------
    WORK_DIR += argv[1];
    WORK_DIR += "/";
    fM_Manager->CreateTblFile((WORK_DIR + "account").c_str());


    //--------初始化数据字典结构-------
    TBL_DIC_META.dbName = "db_schema";
    TBL_DIC_META.tblName = "tables";
    TBL_DIC_META.colNum = 4;
    TBL_DIC_META.type[0] = DB_STRING;
    TBL_DIC_META.type[1] = DB_STRING;
    TBL_DIC_META.type[2] = DB_INT;
    TBL_DIC_META.type[3] = DB_INT;
    TBL_DIC_META.colName[0] = "dbName";
    TBL_DIC_META.colName[1] = "tblName";
    TBL_DIC_META.colName[2] = "colNum";
    TBL_DIC_META.colName[3] = "rowNum";
    TBL_DIC_META.isPrimary[0] = true;
    TBL_DIC_META.isPrimary[1] = true;
    TBL_DIC_META.length[0] = 64;
    TBL_DIC_META.length[1] = 64;
    TBL_DIC_META.isDynamic[0] = true;
    TBL_DIC_META.isDynamic[1] = true;
    for (int i = 0; i < 4; ++i) 
        TBL_DIC_META.colPos[i] = i;
    
    COL_DIC_META.dbName = "db_schema";
    COL_DIC_META.tblName = "columns";
    COL_DIC_META.colNum = 8;
    COL_DIC_META.type[0] = DB_STRING;
    COL_DIC_META.type[1] = DB_STRING;
    COL_DIC_META.type[2] = DB_STRING;
    COL_DIC_META.type[3] = DB_INT;
    COL_DIC_META.type[4] = DB_INT;
    COL_DIC_META.type[5] = DB_BOOL;
    COL_DIC_META.type[6] = DB_BOOL;
    COL_DIC_META.type[7] = DB_INT;
    COL_DIC_META.colName[0] = "dbName";
    COL_DIC_META.colName[1] = "tblName";
    COL_DIC_META.colName[2] = "colName";
    COL_DIC_META.colName[3] = "dataType";
    COL_DIC_META.colName[4] = "length";
    COL_DIC_META.colName[5] = "isPrimary";
    COL_DIC_META.colName[6] = "isDynamic";
    COL_DIC_META.colName[7] = "colPos";
    COL_DIC_META.isPrimary[0] = true;
    COL_DIC_META.isPrimary[1] = true;
    COL_DIC_META.isPrimary[2] = true;
    COL_DIC_META.length[0] = 64;
    COL_DIC_META.length[1] = 64;
    COL_DIC_META.length[2] = 64;
    COL_DIC_META.isDynamic[0] = true;
    COL_DIC_META.isDynamic[1] = true;
    COL_DIC_META.isDynamic[2] = true;
    for (int i = 0; i < 8; ++i) 
        COL_DIC_META.colPos[i] = i;


    //--------更新数据字典------------
    
    RM_TableHandler tblHandler((DBT_DIR + TBL_DIC_NAME).c_str());
    RM_RecAux rAux;
    rAux.strValue.push_back(
        std::make_pair<std::string, std::string>("dbName", argv[1])
    );
    rAux.strValue.push_back(
        std::make_pair<std::string, std::string>("tblName", "account")
    );
    rAux.iValue.push_back(
        std::make_pair<std::string, int>("colNum", 3)
    );
    rAux.iValue.push_back(
        std::make_pair<std::string, int>("rowNum", 0)
    );
    tblHandler.InsertRec(rAux);
    rAux.Clear();
    //
    
    rAux.strValue.push_back(
        std::make_pair<std::string, std::string>("dbName", "db_schema")
    );
    rAux.strValue.push_back(
        std::make_pair<std::string, std::string>("tblName", "tables")
    );
    rAux.iValue.push_back(
        std::make_pair<std::string, int>("colNum", 4)
    );
    rAux.iValue.push_back(
        std::make_pair<std::string, int>("rowNum", 0)
    );
    tblHandler.InsertRec(rAux);
    rAux.Clear();
    //
    rAux.strValue.push_back(
        std::make_pair<std::string, std::string>("dbName", "db_schema")
    );
    rAux.strValue.push_back(
        std::make_pair<std::string, std::string>("tblName", "columns")
    );
    rAux.iValue.push_back(
        std::make_pair<std::string, int>("colNum", 8)
    );
    rAux.iValue.push_back(
        std::make_pair<std::string, int>("rowNum", 0)
    );
    tblHandler.InsertRec(rAux);
    rAux.Clear();
    //
    rAux.strValue.push_back(
        std::make_pair<std::string, std::string>("dbName", "db_schema")
    );
    rAux.strValue.push_back(
        std::make_pair<std::string, std::string>("tblName", "indexes")
    );
    rAux.iValue.push_back(
        std::make_pair<std::string, int>("colNum", 3)
    );
    rAux.iValue.push_back(
        std::make_pair<std::string, int>("rowNum", 0)
    );
    tblHandler.InsertRec(rAux);
    rAux.Clear();

    tblHandler.CloseTbl();

    
    //-------------------------------插入用户表字段描述--------------------------------------
    //RM_TableHandler tblHandler2((DBT_DIR + COL_DIC_NAME).c_str());
    tblHandler.OpenTbl((DBT_DIR + COL_DIC_NAME).c_str());
    rAux.strValue.push_back(
        std::make_pair<std::string, std::string>("dbName", argv[1])
    );
    rAux.strValue.push_back(
        std::make_pair<std::string, std::string>("tblName", "account")
    );
    rAux.strValue.push_back(
        std::make_pair<std::string, std::string>("colName", "account_number")
    );
    rAux.iValue.push_back(
        std::make_pair<std::string, int>("dataType", DB_STRING)
    );
    rAux.iValue.push_back(
        std::make_pair<std::string, int>("length", 10)
    );
    rAux.iValue.push_back(
        std::make_pair<std::string, int>("colPos", 0)
    );
    rAux.bValue.push_back(
        std::make_pair<std::string, bool>("isPrimary", true)
    );
    rAux.bValue.push_back(
        std::make_pair<std::string, bool>("isDynamic", false)
    );
    tblHandler.InsertRec(rAux);
    //h2.InsertRec(rAux);
    rAux.Clear();
    //
    rAux.strValue.push_back(
        std::make_pair<std::string, std::string>("dbName", argv[1])
    );
    rAux.strValue.push_back(
        std::make_pair<std::string, std::string>("tblName", "account")
    );
    rAux.strValue.push_back(
        std::make_pair<std::string, std::string>("colName", "branch_name")
    );
    rAux.iValue.push_back(
        std::make_pair<std::string, int>("dataType", DB_STRING)
    );
    rAux.iValue.push_back(
        std::make_pair<std::string, int>("length", 30)
    );
    rAux.iValue.push_back(
        std::make_pair<std::string, int>("colPos", 1)
    );
    rAux.bValue.push_back(
        std::make_pair<std::string, bool>("isPrimary", false)
    );
    rAux.bValue.push_back(
        std::make_pair<std::string, bool>("isDynamic", true)
    );
    tblHandler.InsertRec(rAux);
    rAux.Clear();
    //
    rAux.strValue.push_back(
        std::make_pair<std::string, std::string>("dbName", argv[1])
    );
    rAux.strValue.push_back(
        std::make_pair<std::string, std::string>("tblName", "account")
    );
    rAux.strValue.push_back(
        std::make_pair<std::string, std::string>("colName", "balance")
    );
    rAux.iValue.push_back(
        std::make_pair<std::string, int>("dataType", DB_DOUBLE)
    );
    rAux.iValue.push_back(
        std::make_pair<std::string, int>("length", 0)
    );
    rAux.iValue.push_back(
        std::make_pair<std::string, int>("colPos", 2)
    );
    rAux.bValue.push_back(
        std::make_pair<std::string, bool>("isPrimary", false)
    );
    rAux.bValue.push_back(
        std::make_pair<std::string, bool>("isDynamic", false)
    );
    tblHandler.InsertRec(rAux);
    rAux.Clear();


    //-------------------------------插入索引系统表字段描述--------------------------------------
    rAux.strValue.push_back(
        std::make_pair<std::string, std::string>("dbName", "db_schema")
    );
    rAux.strValue.push_back(
        std::make_pair<std::string, std::string>("tblName", "indexes")
    );
    rAux.strValue.push_back(
        std::make_pair<std::string, std::string>("colName", "dbName")
    );
    rAux.iValue.push_back(
        std::make_pair<std::string, int>("dataType", DB_STRING)
    );
    rAux.iValue.push_back(
        std::make_pair<std::string, int>("length", 64)
    );
    rAux.iValue.push_back(
        std::make_pair<std::string, int>("colPos", 0)
    );
    rAux.bValue.push_back(
        std::make_pair<std::string, bool>("isPrimary", true)
    );
    rAux.bValue.push_back(
        std::make_pair<std::string, bool>("isDynamic", true)
    );
    tblHandler.InsertRec(rAux);
    rAux.Clear();
    //
    rAux.strValue.push_back(
        std::make_pair<std::string, std::string>("dbName", "db_schema")
    );
    rAux.strValue.push_back(
        std::make_pair<std::string, std::string>("tblName", "indexes")
    );
    rAux.strValue.push_back(
        std::make_pair<std::string, std::string>("colName", "tblName")
    );
    rAux.iValue.push_back(
        std::make_pair<std::string, int>("dataType", DB_STRING)
    );
    rAux.iValue.push_back(
        std::make_pair<std::string, int>("length", 64)
    );
    rAux.iValue.push_back(
        std::make_pair<std::string, int>("colPos", 1)
    );
    rAux.bValue.push_back(
        std::make_pair<std::string, bool>("isPrimary", true)
    );
    rAux.bValue.push_back(
        std::make_pair<std::string, bool>("isDynamic", true)
    );
    tblHandler.InsertRec(rAux);
    rAux.Clear();
    //
    rAux.strValue.push_back(
        std::make_pair<std::string, std::string>("dbName", "db_schema")
    );
    rAux.strValue.push_back(
        std::make_pair<std::string, std::string>("tblName", "indexes")
    );
    rAux.strValue.push_back(
        std::make_pair<std::string, std::string>("colName", "colPos")
    );
    rAux.iValue.push_back(
        std::make_pair<std::string, int>("dataType", DB_INT)
    );
    rAux.iValue.push_back(
        std::make_pair<std::string, int>("length", 0)
    );
    rAux.iValue.push_back(
        std::make_pair<std::string, int>("colPos", 2)
    );
    rAux.bValue.push_back(
        std::make_pair<std::string, bool>("isPrimary", true)
    );
    rAux.bValue.push_back(
        std::make_pair<std::string, bool>("isDynamic", false)
    );
    tblHandler.InsertRec(rAux);
    rAux.Clear();

    //h2.CloseTbl();
    tblHandler.CloseTbl();
    
}

void task1() {

    //--------插入记录----------------
    //std::cout<<WORK_DIR<<std::endl;
    RM_TableHandler tblHandler((WORK_DIR + "account").c_str());
    auto meta = tblHandler.GetMeta();
    
    SString<10> accNum;
    SString<30> branchName;
    char bN[5][30] = {"Downtown", "Perryridge", "Redwood", "Mianus", "Brighton"};
    double balance;
    RM_RecAux rAux;
    for (int i = 0; i < 10000; ++i) {
        rAux.Clear();
        rAux.strValue.push_back(
            std::make_pair<std::string, std::string>("account_number", "A-" + to_string(i))
        );
        rAux.strValue.push_back(
            std::make_pair<std::string, std::string>("branch_name", bN[i%5])
        );
        rAux.lfValue.push_back(
            std::make_pair<std::string, double>("balance", rand() % 100000)
        );
        tblHandler.InsertRec(rAux);
    }
    
    
    //--------读出记录----------------
    RM_TblIterator iter;
    strcpy(accNum.msg, "accNum");
    strcpy(branchName.msg, "branchName");
    printf("%s %s %s\n", accNum.msg, branchName.msg, "balance");
    tuple<SString<10>, SString<30>, double> item;

    RM_Record rec;
    tblHandler.GetIter(iter);
    char tStr1[64];
    char tStr2[64];
    double tLf;
    do {
        rec = iter.NextRec();
        if (rec.rid.num == -1)
            break;
        rec.GetColData(tblHandler.GetMeta(), 0, tStr1);
        rec.GetColData(tblHandler.GetMeta(), 1, tStr2);
        rec.GetColData(tblHandler.GetMeta(), 2, &tLf);
        printf("%s %s %lf\n", tStr1, tStr2, tLf);
    }while(true);
    
    tblHandler.CloseTbl();
}

void task3() {
    IM_Manager iManager;
    iManager.CreateIndex((WORK_DIR + "account").c_str(), 0);

}

