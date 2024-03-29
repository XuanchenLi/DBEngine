#ifndef MM_BUFFERUNIT_H
#define MM_BUFFERUNIT_H

#include <unistd.h>
#include <memory.h>
#include "utils/RC.h"
#include "MM_Buffer.h"
#include "FM/FM_Bid.h"
#include "main.h"

extern unsigned int BLOCK_SIZE;

class MM_BufferUnit {

friend class MM_Buffer;
public:
    MM_BufferUnit(): dirty(false), refCount(0), usedCount(0) {
        bid.fd = -1;
        bid.num = -1;
        content = (char*)malloc(BLOCK_SIZE);
        memset(content, 0, sizeof(content));
    }
    ~MM_BufferUnit() {
        free(content);
    }

    bool IsDirty() {return dirty;}
    FM_Bid GetBid() {return bid;}
    char* GetPtr() {return content;}
    int GetUsedCnt()const {return usedCount;}
    void SetUsedCnt(int cnt) {usedCount = cnt;}
private:
    void clear() {
        dirty = false;
        bid.fd = -1;
        bid.num = -1;
        refCount = 0;
        usedCount = 0;
        memset(content, 0, BLOCK_SIZE);
    }

    bool dirty;  //标记脏
    FM_Bid bid;  //当前缓存块对应物理块
    int refCount;  //引用计数
    int usedCount;  //近期使用次数
    char* content;  //缓存内容
};

#endif