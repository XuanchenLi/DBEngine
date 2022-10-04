#include <unistd.h>
#include <iostream>
#include <string.h>
#include <sys/stat.h>
#include <tuple>
#include "FM/FM_Manager.h"
#include "RM/RM_TblIterator.h"
#include "main.h"
#include "MM/MM_Buffer.h"
#include "RM/RM_TblMeta.h"
#include "RM/RM_TableHandler.h"
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
RM_TblMeta TBL_DIC_META;
RM_TblMeta COL_DIC_META;

void dbClear();
void dbInit(int argc, char* argv[]);
void task1();


int main(int argc, char* argv[]) {
    if (argc != 5) {
        cout<<"usage:./main [DBname] [preallocate size](MB) [buffer block number] [buffer block size](KB)"<<endl;
        exit(0);
    }
    dbInit(argc, argv);
    //----------------------------------

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
    gBuffer = new MM_Buffer(BUFFER_SIZE, MM_Buffer::CLOCK_SWEEP);
    fM_Manager = new FM_Manager();
    fM_Manager->CreateDir("./DB");
    fM_Manager->CreateDir(DBT_DIR.c_str());
    fM_Manager->CreateDir((WORK_DIR + argv[1]).c_str());

    //--------创建数据字典------------
    int status = fM_Manager->CreateTblFile((DBT_DIR + "tables").c_str());
    fM_Manager->CreateTblFile((DBT_DIR + "columns").c_str());
    //std::cout<<status<<std::endl;

    //--------创建表文件--------------
    WORK_DIR += argv[1];
    WORK_DIR += "/";
    fM_Manager->CreateTblFile((WORK_DIR + "banking").c_str());


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
        TBL_DIC_META.colPos[i] = i;


    //--------更新数据字典------------
    
    RM_TableHandler tblHandler((DBT_DIR + "tables").c_str());
    SString<64> DBName;
    SString<64> TblName;

    strcpy(DBName.msg, argv[1]);
    strcpy(TblName.msg, "banking");
    tblHandler.Insert<TBL_DIC_ROW>(TBL_DIC_ROW(
        DBName,
        TblName,
        3,
        0
    ));

    //
    strcpy(DBName.msg, "db_schema");
    strcpy(TblName.msg, "columns");
    tblHandler.Insert<TBL_DIC_ROW>(TBL_DIC_ROW(
        DBName,
        TblName,
        8,
        0
    ));
    //
    strcpy(DBName.msg, "db_schema");
    strcpy(TblName.msg, "tables");
    tblHandler.Insert<TBL_DIC_ROW>(TBL_DIC_ROW(
        DBName,
        TblName,
        4,
        0
    ));


    tblHandler.CloseTbl();
}

void task1() {
    //--------插入记录----------------
    
    SString<10> accNum;
    SString<30> branchName;
    char bN[5][30] = {"Downtown", "Perryridge", "Redwood", "Mianus", "Brighton"};
    double balance;
    RM_TableHandler tblHandler;
    tblHandler.OpenTbl((WORK_DIR + "banking").c_str());
    for (int i = 0; i < 10000; ++i) {
        strcpy(accNum.msg, ("A-" + to_string(i)).c_str());
        strcpy(branchName.msg, bN[i%5]);
        balance = rand() % 100000;
        tblHandler.Insert<tuple<SString<10>, SString<30>, double>> (
            make_tuple(accNum, branchName, balance)
        );
    }
    
    //--------读出记录----------------
    RM_TblIterator iter;
    strcpy(accNum.msg, "accNum");
    strcpy(branchName.msg, "branchName");
    printf("%s %s %s\n", accNum.msg, branchName.msg, "balance");
    tuple<SString<10>, SString<30>, double> item;

    RM_Record rec;
    tblHandler.GetIter(iter);
    do {
        rec = iter.NextRec();
        if (rec.rid.num == -1)
            break;
        memcpy(&item, rec.addr, rec.len);
        //std::cout<<rec.rid.num<<" "<<rec.rid.slot<<std::endl;
        printf("%s %s %lf\n", std::get<0>(item).msg, std::get<1>(item).msg, std::get<2>(item));
    }while(true);
    
    tblHandler.CloseTbl();
}