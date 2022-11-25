#ifndef RM_RECHEADER_H
#define RM_RECHEADER_H

typedef struct RM_RecHeader {
    bool isDeleted;
    int off;  //块内偏移
    int len;  //数据+前缀所占用长度
}RM_RecHdr;



#endif