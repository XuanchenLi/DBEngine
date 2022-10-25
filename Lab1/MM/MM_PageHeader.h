#ifndef MM_PAGEHEADER_H
#define MM_PAGEHEADER_H
#include "main.h"

extern unsigned int BLOCK_SIZE;

struct MM_PageHeader {

    MM_PageHeader(): slotCnt(0), freeOff(BLOCK_SIZE - 1), firstHole(-1), preFreePage(-1), nextFreePage(-1){}

    int slotCnt;  //用于索引时表示最大键数
    int freeOff;
    int freeBtsCnt;
    int firstHole;     //用于索引时表示当前已有键数
    int preFreePage;    //用于索引时表示父节点，0表示根
    int nextFreePage;   //用于索引时-1表示叶节点 1表示非叶节点
};

typedef struct MM_PageHeader MM_PageHdr;

#endif