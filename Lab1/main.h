#ifndef MAIN_H
#define MAIN_H

#define BLOCK_LIMIT 0.8
#include <string>
#include <tuple>
#include "utils/DBType.h"

class FM_Manager;
class MM_Buffer;

extern unsigned int BLOCK_SIZE; //KB
extern unsigned int PREALLOC_SIZE; //MB
extern unsigned int BUFFER_SIZE;
extern FM_Manager* fM_Manager;
extern MM_Buffer* gBuffer;
extern std::string DBT_DIR;
extern std::string WORK_DIR;


//==================DBName=======TblName=====type==rowNum=========
typedef std::tuple<SString<64>, SString<64>, char, int> TBL_DIC_ROW;
//==================DBName====TblName=====ColName==Datatype===nullAble====
typedef std::tuple<char [64], char [64], char [64], char [32], bool> COL_DIC_ROW;


#endif
