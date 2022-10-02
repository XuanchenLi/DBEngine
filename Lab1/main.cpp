#include <unistd.h>
#include <iostream>
#include <string.h>
#include <sys/stat.h>
#include <tuple>
#include "FM/FM_Manager.h"
#include "RM/RM_TblIterator.h"
#include "main.h"
#include "MM/MM_Buffer.h"
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

int main(int argc, char* argv[]) {
    if (argc != 5) {
        cout<<"usage:./main [DBname] [preallocate size](MB) [buffer block number] [buffer block size](KB)"<<endl;
        exit(0);
    }
    //--------创建数据库目录----------
    BLOCK_SIZE = (unsigned int)atoi(argv[4]) << 10;
    BUFFER_SIZE = (unsigned int)atoi(argv[3]);
    PREALLOC_SIZE = (unsigned int)atoi(argv[2])<< 20;
    gBuffer = new MM_Buffer(BUFFER_SIZE);
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

    //--------更新数据字典------------
    
    RM_TableHandler tblHandler((DBT_DIR + "tables").c_str());
    SString<64> DBName;
    SString<64> TblName;

    strcpy(DBName.msg, argv[1]);
    strcpy(TblName.msg, "banking");
    tblHandler.Insert<TBL_DIC_ROW>(TBL_DIC_ROW(
        DBName,
        TblName,
        'S',
        0
    ));

    //
    strcpy(DBName.msg, "db_schema");
    strcpy(TblName.msg, "columns");
    tblHandler.Insert<TBL_DIC_ROW>(TBL_DIC_ROW(
        DBName,
        TblName,
        'S',
        0
    ));
    //
    strcpy(DBName.msg, "db_schema");
    strcpy(TblName.msg, "tables");
    tblHandler.Insert<TBL_DIC_ROW>(TBL_DIC_ROW(
        DBName,
        TblName,
        'S',
        0
    ));

    tblHandler.Insert<TBL_DIC_ROW>(
        TBL_DIC_ROW(
        DBName,
        TblName,
        'S',
        0
    )
    );

    tblHandler.CloseTbl();

    //--------插入记录----------------
    
    SString<10> accNum;
    SString<30> branchName;
    char bN[5][30] = {"Downtown", "Perryridge", "Redwood", "Mianus", "Brighton"};
    double balance;

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
    //----------------------------------
    gBuffer->Clear();
    delete fM_Manager;
    delete gBuffer;

}