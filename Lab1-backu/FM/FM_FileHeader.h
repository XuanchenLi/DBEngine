#ifndef FM_FILEHEADER_H
#define FM_FILEHEADER_H

typedef struct FM_FileHeader {
    int blkCnt;  //所用块总数
    int firstFreeHole;  //空闲块链表头
    int preF;  //若用于索引代表根节点
    int nextF;
    FM_FileHeader():blkCnt(1), firstFreeHole(-1), preF(0), nextF(0) {}
}FM_FileHdr;

#endif