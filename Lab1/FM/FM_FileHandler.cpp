#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include "FM/FM_FileHandler.h"
#include "MM/MM_PageHeader.h"
#include "main.h"

extern unsigned int BLOCK_SIZE;

RC FM_FileHandler::GetBlock(int bNum, char* buf) {
    if (!isOpen)
        return INVALID_OPTR;
    if (bNum + 1 > this->fHdr.blkCnt)
        return OUT_OF_RANGE_ERROR;
    lseek(this->fd, bNum * BLOCK_SIZE, SEEK_SET);
    int res = read(this->fd, buf, BLOCK_SIZE);
    if (res != BLOCK_SIZE) {
        return IO_ERROR;
    }
    return SUCCESS;
}

RC FM_FileHandler::WriteBlock(int bNum, char* buf) {
    if (!isOpen)
        return INVALID_OPTR;
    if (bNum + 1 > this->fHdr.blkCnt + 1) {
        return OUT_OF_RANGE_ERROR;
    }
    /*
    MM_PageHdr* pHdr = (MM_PageHdr*)buf;
    if ((float)(pHdr->freeBtsCnt) / BLOCK_SIZE > (1.0f - BLOCK_LIMIT)) {
        if (pHdr->nextFreePage == -1 && pHdr->preFreePage == -1) {
            pHdr->nextFreePage = this->fHdr.firstFreeHole;
            pHdr->preFreePage = 0;
            this->fHdr.firstFreeHole = bNum;
            this->changed = true;
        }
    }
    else {
        
        
    }
    */
    int offset = BLOCK_SIZE * bNum;
    //std::cout<<"121"<<std::endl;
    lseek(this->fd, offset, SEEK_SET);
    int res = write(fd, buf, BLOCK_SIZE);
    //std::cout<<"121"<<std::endl;
    if (res != BLOCK_SIZE)
        return IO_ERROR;
    return SUCCESS;
}

int FM_FileHandler::GetNextFree() {
    if (!this->isOpen)
        return INVALID_OPTR;
    if (fHdr.firstFreeHole > 0) {
        //std::cout<<fHdr.firstFreeHole<<std::endl;
        return fHdr.firstFreeHole;
    }
    MM_PageHdr newPHdr;
    //std::cout<<newPHdr.freeOff<<std::endl;
    newPHdr.nextFreePage = this->fHdr.firstFreeHole;
    newPHdr.preFreePage = 0;
    newPHdr.freeBtsCnt = BLOCK_SIZE - sizeof(MM_PageHdr);
    //std::cout<<this->fHdr.blkCnt<<" "<<newPHdr.freeBtsCnt<<std::endl;
    this->fHdr.firstFreeHole = this->fHdr.blkCnt;
    int newBlk = this->fHdr.blkCnt;
    this->changed = true;
    lseek(this->fd, BLOCK_SIZE * newBlk, SEEK_SET);
    //std::cout<<newPHdr.freeBtsCnt<<std::endl;
    int res = write(this->fd, &newPHdr, sizeof(MM_PageHdr));

    if (res != sizeof(MM_PageHdr))
        return IO_ERROR;
    this->fHdr.blkCnt ++;
    this->changed = true;
    return newBlk;
}