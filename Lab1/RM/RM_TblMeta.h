#ifndef RM_TBLMETA_H
#define RM_TBLMETA_H

#include <string>
#include <string.h>
#include "main.h"

struct RM_TblMeta {
    std::string dbName;
    std::string tblName;
    int colNum;
    int type[MAXCOLNUM];
    std::string colName[MAXCOLNUM];
    bool isPrimary[MAXCOLNUM];
    int length[MAXCOLNUM];
    int isDynamic[MAXCOLNUM];
    int colPos[MAXCOLNUM];
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
        return NOT_EXIST;
    }
    int GetIdxByName(const std::string& name) const {
        for (int i = 0; i < colNum; ++i) {
            if (colName[i] == name)
                return i;
            return NOT_EXIST;
        }
    }
    std::string GetNameByPos(const int pos) const {
        for (int i = 0; i < colNum; ++i) {
            if (colPos[i] == pos) {
                return colName[i];
            }
        }
        return "";
    }
    int GetStaticPartLen() const {
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

    //不包括前缀
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