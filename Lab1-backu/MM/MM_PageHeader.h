#ifndef MM_PAGEHEADER_H
#define MM_PAGEHEADER_H
#include "main.h"

extern unsigned int BLOCK_SIZE;

struct MM_PageHeader {

    MM_PageHeader(): slotCnt(0), freeOff(BLOCK_SIZE - 1), firstHole(-1), preFreePage(-1), nextFreePage(-1){}

    int slotCnt;  //用于索引时表示最大键数；用于数据表表示所用槽数
    int freeOff;    //用于数据表代表末尾偏移
    int freeBtsCnt;     //用于索引时表示当前已有指针数；用于数据表代表空闲字节数
    int firstHole;     //用于索引时表示当前已有键数；用于数据表代表第一个空闲槽
    int preFreePage;    //用于索引时表示父节点，0表示根；用于数据表代表空闲链表中下一个页
    int nextFreePage;   //用于索引时-1表示叶节点 1表示非叶节点；用于数据表代表空闲链表中上一个页
};

typedef struct MM_PageHeader MM_PageHdr;

#endif