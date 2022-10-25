#ifndef FM_FILEHANDLER_H
#define FM_FILEHANDLER_H

#include "utils/RC.h"
#include "FM_FileHeader.h"


class FM_FileHandler {
public:
    FM_FileHandler():fd(-1), isOpen(false), changed(false){}
    ~FM_FileHandler() {}
    FM_FileHandler(const FM_FileHandler &) = default;
    FM_FileHandler& operator= (const FM_FileHandler &) = default;

    RC GetBlock(int bNum, char* buf);
    RC WriteBlock(int bNum, char* buf);
    int GetNextFree();
    int GetNextFree(int len);  // len包括记录前缀长不包括头部长
    int GetNextWhole();
    RC ReturnWhole(int num);
    

    int GetBlockNum() const  {return fHdr.blkCnt;}
    void SetFileHdr(const FM_FileHdr& rhs) {
        this->fHdr = rhs;
    };
    void SetFd(const int fd) {this->fd = fd;}
    void SetOpen(const bool s) {isOpen = s;}
    void SetChanged(const bool s) {this->changed = s;}
    int GetFd()const {return this->fd;}
    bool IsOpen()const {return isOpen;}
    bool IsChanged()const {return changed;}
    FM_FileHdr GetFileHdr()const {return fHdr;}

private:
    int fd;
    bool isOpen;
    bool changed;
    FM_FileHdr fHdr;

};

#endif