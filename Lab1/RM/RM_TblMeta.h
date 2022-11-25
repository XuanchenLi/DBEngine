#ifndef RM_TBLMETA_H
#define RM_TBLMETA_H

#include <string>
#include <string.h>
#include "RM/RM_RecPrefix.h"
#include "main.h"

struct RM_TblMeta {
    std::string dbName;  //数据库名
    std::string tblName;  //表名
    int colNum;  //记录字段数
    dbType type[MAXCOLNUM];  //字段数据类型
    std::string colName[MAXCOLNUM];  //字段名
    bool isPrimary[MAXCOLNUM];  //是否主键
    int length[MAXCOLNUM];  //字段长度
    int isDynamic[MAXCOLNUM];  //字段是否可变长
    int colPos[MAXCOLNUM];  //字段顺序编号
    int GetDynamicNum() const{
        int dNum = 0;
        for (int i = 0; i < colNum; ++i){
            if (isDynamic[i])
                dNum++;
        }
        return dNum;
    }
    int GetPrefixLen() const {
        int res = sizeof(bool) + sizeof(int) + sizeof(PosInfo) * GetDynamicNum();
        return res;
    }
    int GetPosByName(const std::string& name) const {
        for (int i = 0; i < colNum; ++i) {
            if (colName[i] == name)
                return colPos[i];
        }
        //std::cout<<name<<std::endl;
        return NOT_EXIST;
    }
    int GetIdxByName(const std::string& name) const {
        //std::cout<<name<<std::endl;
        for (int i = 0; i < colNum; ++i) {
            if (colName[i] == name)
                return i;
        }
        //std::cout<<name<<std::endl;
        return NOT_EXIST;
    }
    std::string GetNameByPos(const int pos) const {
        for (int i = 0; i < colNum; ++i) {
            if (colPos[i] == pos) {
                return colName[i];
            }
        }
        //std::cout<< pos <<std::endl;
        return "";
    }
    int GetStaticPartLen() const {
        //不包括前缀
        int res = 0;
        for (int i = 0; i < colNum; ++i) {
            if (!isDynamic[i]) {
                switch(type[i]){
                    case DB_BOOL:
                        res += sizeof(bool);
                        break;
                    case DB_DOUBLE:
                        res += sizeof(double);
                        break;
                    case DB_INT:
                        res += sizeof(int);
                        break;
                    case DB_STRING:
                        res += length[i];
                        break;
                }
            }
        }
        return res;
    }
    int GetStaticPartOff(const int pos) const {
        int res = 0;
        for (int i = 0; i < colNum; ++i) {
            if (!isDynamic[i] && colPos[i] < pos) {
                switch(type[i]){
                    case DB_BOOL:
                        res += sizeof(bool);
                        break;
                    case DB_DOUBLE:
                        res += sizeof(double);
                        break;
                    case DB_INT:
                        res += sizeof(int);
                        break;
                    case DB_STRING:
                        res += length[i];
                        break;
                }
            }
        }
        return res;
    }
};



#endif