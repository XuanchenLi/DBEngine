#ifndef MAIN_H
#define MAIN_H

#define BLOCK_LIMIT 0.8
#define MAXCOLNUM 30
#include <string>
#include <tuple>
#include "utils/DBType.h"

class FM_Manager;
class MM_Buffer;
class RM_TblMeta;

extern unsigned int BLOCK_SIZE; //KB
extern unsigned int PREALLOC_SIZE; //MB
extern unsigned int BUFFER_SIZE;
extern FM_Manager* fM_Manager;
extern MM_Buffer* gBuffer;
extern std::string DBT_DIR;
extern std::string WORK_DIR;
extern const char TBL_DIC_NAME[];
extern const char COL_DIC_NAME[];
extern RM_TblMeta TBL_DIC_META;
extern RM_TblMeta COL_DIC_META;


//==================DBName=======TblName====colNum==rowNum=======
typedef std::tuple<SString<64>, SString<64>, int, int> TBL_DIC_ROW;
//=================DBName====TblName===ColName==Datatype=length=isPrimary=isDynamic=colPos=
typedef std::tuple<char [64], char [64], char [64], int,  int, bool, bool, int> COL_DIC_ROW;


#endif
