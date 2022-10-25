#ifndef FM_FILEHEADER_H
#define FM_FILEHEADER_H

typedef struct FM_FileHeader {
    //int maxBlkNum;
    int blkCnt;
    int firstFreeHole;
    int preF;  //用于索引代表根
    int nextF;
    FM_FileHeader():blkCnt(1), firstFreeHole(-1), preF(0), nextF(0) {}
}FM_FileHdr;

#endif