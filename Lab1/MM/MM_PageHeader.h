#ifndef MM_PAGEHEADER_H
#define MM_PAGEHEADER_H
#include "main.h"

extern unsigned int BLOCK_SIZE;

struct MM_PageHeader {

    MM_PageHeader(): slotCnt(0), freeOff(BLOCK_SIZE - 1), firstHole(-1), preFreePage(-1), nextFreePage(-1){}

    int slotCnt;
    int freeOff;
    int freeBtsCnt;
    int firstHole;
    int preFreePage;
    int nextFreePage;
};

typedef struct MM_PageHeader MM_PageHdr;

#endif