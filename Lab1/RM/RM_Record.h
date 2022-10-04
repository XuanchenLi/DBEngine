#ifndef RM_RECORD_H
#define RM_RECORD_H

#include <tuple>
#include <iostream>
#include "RM_Rid.h"
#include "utils/RC.h"
#include "RM_TblMeta.h"
#include "RM_RecAux.h"
#include "RM_RecPrefix.h"

struct RM_Record {

    RM_Rid rid;
    RM_RecPrefix prefix;
    char* addr;
    int len;
    
    template <typename T>
    RC SetContent(T t) {
        if (addr == nullptr)
            return INVALID_OPTR;
        memcpy(addr, (char*)&t, sizeof(t));
        len = sizeof(t);
        return SUCCESS;
    }

    // 读取现有内容的前缀信息，而非新建
    RC InitPrefix(const RM_TblMeta& meta) {
        if (addr == nullptr)
            return FAILURE;
        if (prefix.posInfo != nullptr)
            delete[] prefix.posInfo;
        int pLen = meta.GetPrefixLen();
        int num = (pLen - sizeof(bool) - sizeof(int)) / sizeof(PosInfo);
        prefix.posInfo = new PosInfo[num];
        memcpy(&prefix, addr, pLen);
        return SUCCESS;
    }

    //
    RC SerializeData(const RM_TblMeta& meta, RM_RecAux& data) {
        if(addr != nullptr) {
            //!!需要防止addr此时指向缓冲区内部
            //避免指向缓冲区的对象和用于插入的对象的混用
            delete[] addr;
        }
        int totNum = data.iValue.size() + data.lfValue.size() + data.strValue.size();
        if (totNum > meta.colNum) {
            std::cout<<"LESS COL NUM THAN PROVIDED\n"<<std::endl;
            return FAILURE;
        }

        //获取数据总长度（包括前缀）来分配内存空间
        int totLen = meta.GetStaticPartLen() + meta.GetPrefixLen();
        for (int i = 0; i < data.strValue.size(); ++i) {
            int idx = 0;
            if ((idx = meta.GetIdxByName(data.strValue[i].first)) != NOT_EXIST) {
                if (meta.isDynamic[idx]) {
                    totLen += data.strValue[i].second.length();
                }
            }else {
                std::cout<<"COL NOT EXIST\n"<<std::endl;
                return FAILURE;
            }
        }
        addr = new char[totLen];
        bool tmpB = true;
        int tmpI = meta.GetDynamicNum();
        memcpy(addr, &tmpB, sizeof(bool));
        memcpy(addr + sizeof(bool), &tmpI, sizeof(int));
        int sStart = meta.GetPrefixLen();
        int dStart = meta.GetPrefixLen() + meta.GetStaticPartLen();
        int dCnt = 0;
        for (int i = 0; i < meta.colNum; ++i) {
            std::string tar = meta.GetNameByPos(i);
            std::pair<int, int> vPos = data.GetPosByKey(tar);
            if (vPos.first == NOT_EXIST) {
                delete[] addr;
                std::cout<<"COL NOT EXIST\n"<<std::endl;
                return FAILURE;
            }
            if (vPos.first == 0) {
                int idx = meta.GetIdxByName(tar);
                if (meta.isDynamic[idx]) {
                    PosInfo pos;
                    pos.start = dStart;
                    strcpy(addr + dStart, data.strValue[vPos.second].second.c_str());
                    dStart += strlen(data.strValue[vPos.second].second.c_str()) + 1;
                    pos.end = dStart;
                    memcpy(addr + sizeof(bool) + sizeof(int) + dCnt * sizeof(PosInfo), &pos, sizeof(PosInfo));
                    dCnt++;
                }else {
                    strcpy(addr + sStart, data.strValue[vPos.second].second.c_str());
                    sStart += strlen(data.strValue[vPos.second].second.c_str()) + 1;  //加上'\0'
                }
            }else if (vPos.first == 1) {
                memcpy(addr + sStart, &data.lfValue[vPos.second].second, sizeof(double));
                sStart += sizeof(double);
            }else if (vPos.first == 2) {
                memcpy(addr + sStart, &data.iValue[vPos.second].second, sizeof(int));
                sStart += sizeof(int);
            }
        }
        return SUCCESS;

    }

    int GetOffset(const RM_TblMeta& meta, const int pos) const {
        if (pos >= meta.colNum)
            return OUT_OF_RANGE_ERROR;
        int idx = 0;
        int preDNum = 0;
        int preSNum = 0;
        int res = 0;
        for (int i = 0; i < meta.colNum; ++i) {
            if (meta.colPos[i] == pos)
                idx = i;
            if (meta.colPos[i] < pos) {
                if (meta.isDynamic[i])
                    preDNum++;
                else
                    preSNum++;
            }
        }
        if (meta.isDynamic[idx]) {
            //res = meta.GetStaticPartLen();
            PosInfo posI = prefix.posInfo[preDNum];
            res = posI.start; 
        }else {
            res = meta.GetStaticPartOff(pos);
            res += meta.GetPrefixLen();
        }
        return res;
    }
    PosInfo GetDynamicPosInfo(const RM_TblMeta& meta, int pos) {
        int idx = 0;
        int preDNum = 0;
        for (int i = 0; i < meta.colNum; ++i) {
            if (meta.colPos[i] == pos)
                idx = i;
            if (meta.colPos[i] < pos) {
                if (meta.isDynamic[i])
                    preDNum++;
            }
        }
        return prefix.posInfo[preDNum];
    }
    
    
    RC GetColData(const RM_TblMeta& meta, int num, void* p) {
        if (num >= meta.colNum)
            return OUT_OF_RANGE_ERROR;
        if (p==nullptr)
            return INVALID_OPTR;
        int idx = 0;
        int preDNum = 0;
        int preSNum = 0;
        for (int i = 0; i < meta.colNum; ++i) {
            if (meta.colPos[i] == num) {
                idx = i;
                break;
            }
        }
        int off = GetOffset(meta, num);
        if (meta.isDynamic[idx]) {
            PosInfo info = GetDynamicPosInfo(meta, num);
            memcpy(p, addr + info.start, info.end - info.start);  //end为下一个字段的开始
        }else {
            switch(meta.type[idx]) {
                case DB_BOOL:
                    *(bool*)p = *(bool*)(addr + off);
                    break;
                case DB_DOUBLE:
                    *(double*)p = *(double*)(addr + off);
                    break;
                case DB_INT:
                    *(int*)p = *(int*)(addr + off);
                    break;
                case DB_STRING:
                    memcpy(p, addr + off, meta.length[idx]);
                    break;
            }
        }
        return SUCCESS;

    }

    bool valid(const RM_TblMeta& meta, const std::vector<DB_Opt>& lims) {
        if (lims.empty())
            return true;
        for (int i = 0; i < lims.size(); ++i) {
            int pos = meta.GetPosByName(lims[i].colName);
            if (pos == NOT_EXIST) {
                std::cout<<"COL NOT FOUND!\n";
                return false;
            }
            int idx = meta.GetIdxByName(lims[i].colName);

            void* ptr;
            if (meta.type[idx] != lims[i].type)
            {
                std::cout<<"UNCONSIST TYPR\n";
                return false;
            }
            switch(meta.type[idx]) {
                case DB_BOOL:
                    ptr = new bool;
                    break;
                case DB_DOUBLE:
                    ptr = new double;
                    break;
                case DB_INT:
                    ptr = new int;
                    break;
                case DB_STRING:
                    ptr = new char[meta.length[idx]];
                    break;
            }
            int sc = GetColData(meta, pos, ptr);
            if (sc != SUCCESS) {
                switch(meta.type[idx]) {
                case DB_BOOL:
                    delete static_cast<bool*>(ptr);
                    break;
                case DB_DOUBLE:
                    delete static_cast<double*>(ptr);
                    break;
                case DB_INT:
                    delete static_cast<int*>(ptr);
                    break;
                case DB_STRING:
                    delete[] static_cast<char*>(ptr);
                    break;
                }
                std::cout<<"FAIL TO GET COL DATA\n";
                return false;
            }
            bool re = true;
            switch(lims[i].optr) {
                case EQUAL:
                    {
                        switch(meta.type[idx]) {
                        case DB_BOOL:
                            if (*(bool*)ptr != lims[i].data.bData)
                                re=false;
                            break;
                        case DB_DOUBLE:
                            if (*(double*)ptr != lims[i].data.lfData)
                                re=false;
                            break;
                        case DB_INT:
                            if (*(int*)ptr != lims[i].data.iData)
                                re=false;
                            break;
                        case DB_STRING:
                            if (strcmp((char*)ptr, lims[i].data.sData )!=0)
                                re = false;
                            break;
                        }
                    }
                    
                    break;

                case NOT_EQUAL:
                    {
                        switch(meta.type[idx]) {
                        case DB_BOOL:
                            if (*(bool*)ptr == lims[i].data.bData)
                                re=false;
                            break;
                        case DB_DOUBLE:
                            if (*(double*)ptr == lims[i].data.lfData)
                                re=false;
                            break;
                        case DB_INT:
                            if (*(int*)ptr == lims[i].data.iData)
                                re=false;
                            break;
                        case DB_STRING:
                            if (strcmp((char*)ptr, lims[i].data.sData )==0)
                                re = false;
                            break;
                        }
                    }
                    break;

                case LESS:
                    {
                        switch(meta.type[idx]) {
                        case DB_BOOL:
                            if (*(bool*)ptr >= lims[i].data.bData)
                                re=false;
                            break;
                        case DB_DOUBLE:
                            if (*(double*)ptr >= lims[i].data.lfData)
                                re=false;
                            break;
                        case DB_INT:
                            if (*(int*)ptr >= lims[i].data.iData)
                                re=false;
                            break;
                        case DB_STRING:
                            if (strcmp((char*)ptr, lims[i].data.sData )>=0)
                                re = false;
                            break;
                        }
                    }
                    break;

                case GREATER:
                    {
                        switch(meta.type[idx]) {
                        case DB_BOOL:
                            if (*(bool*)ptr <= lims[i].data.bData)
                                re=false;
                            break;
                        case DB_DOUBLE:
                            if (*(double*)ptr <= lims[i].data.lfData)
                                re=false;
                            break;
                        case DB_INT:
                            if (*(int*)ptr <= lims[i].data.iData)
                                re=false;
                            break;
                        case DB_STRING:
                            if (strcmp((char*)ptr, lims[i].data.sData )<=0)
                                re = false;
                            break;
                        }
                    }
                    break;

                case NOT_LESS:
                    {
                        switch(meta.type[idx]) {
                        case DB_BOOL:
                            if (*(bool*)ptr < lims[i].data.bData)
                                re=false;
                            break;
                        case DB_DOUBLE:
                            if (*(double*)ptr < lims[i].data.lfData)
                                re=false;
                            break;
                        case DB_INT:
                            if (*(int*)ptr < lims[i].data.iData)
                                re=false;
                            break;
                        case DB_STRING:
                            if (strcmp((char*)ptr, lims[i].data.sData )<0)
                                re = false;
                            break;
                        }
                    }
                    break;

                case NOT_GREATER:
                    {
                        switch(meta.type[idx]) {
                        case DB_BOOL:
                            if (*(bool*)ptr > lims[i].data.bData)
                                re=false;
                            break;
                        case DB_DOUBLE:
                            if (*(double*)ptr > lims[i].data.lfData)
                                re=false;
                            break;
                        case DB_INT:
                            if (*(int*)ptr > lims[i].data.iData)
                                re=false;
                            break;
                        case DB_STRING:
                            if (strcmp((char*)ptr, lims[i].data.sData )>0)
                                re = false;
                            break;
                        }
                    }
                    break;

            }
            switch(meta.type[idx]) {
                case DB_BOOL:
                    delete static_cast<bool*>(ptr);
                    break;
                case DB_DOUBLE:
                    delete static_cast<double*>(ptr);
                    break;
                case DB_INT:
                    delete static_cast<int*>(ptr);
                    break;
                case DB_STRING:
                    delete[] static_cast<char*>(ptr);
                    break;
            }
            if (!re)
                return false;           
        }
        return true;
    }

};


#endif