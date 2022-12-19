#include "DB_Iterator.h"
#include "utils/stringUtil.h"
#include <vector>
#include <algorithm>


RC ProjectMemory(char* content, const RM_TblMeta &meta, const RM_Record& srcRec, const RM_TblMeta &srcMeta) {
    //确保meta初始化正确
    memcpy(content, srcRec.addr, sizeof(bool));
    int dColNum = meta.GetDynamicNum();
    int dCnt = 0, dOff = meta.GetPrefixLen() + meta.GetStaticPartLen();
    memcpy(content + sizeof(bool), &dColNum, sizeof(int));
    for (int i = 0; i < meta.colNum; ++i) {
        if (meta.isDynamic[i]) {
            PosInfo posInfo;
            PosInfo srcPosInfo = srcRec.GetDynamicPosInfo(srcMeta, srcMeta.GetPosByName(meta.colName[i]));
            posInfo.start = dOff;
            posInfo.end = dOff + srcPosInfo.end - srcPosInfo.start;
            //std::cout<<posInfo.start<<" "<<posInfo.end<<std::endl;
            memcpy(content + sizeof(int) + sizeof(bool) + dCnt * sizeof(posInfo), &posInfo, sizeof(posInfo));
            memcpy(content + dOff, srcRec.addr + srcPosInfo.start, srcPosInfo.end - srcPosInfo.start);
            //puts(content + dOff);
            dOff += srcPosInfo.end - srcPosInfo.start;
            dCnt++;
        }else {
            int off = meta.GetPrefixLen() + meta.GetStaticPartOff(meta.colPos[i]);
            int srcOff = srcMeta.GetPrefixLen() + 
                        srcMeta.GetStaticPartOff(
                            srcMeta.colPos[srcMeta.GetIdxByName(meta.colName[i])]
                            );
            switch (meta.type[i]) {
                case DB_INT:
                    memcpy(content + off, srcRec.addr + srcOff, sizeof(int));
                    break;
                case DB_BOOL:
                    memcpy(content + off, srcRec.addr + srcOff, sizeof(bool));
                    break;
                case DB_STRING:
                    memcpy(content + off, srcRec.addr + srcOff, meta.length[i]);
                    break;
                case DB_DOUBLE:
                    memcpy(content + off, srcRec.addr + srcOff, sizeof(double));
                    break;
            }
            //memcpy(content + off, srcRec.addr + srcOff, meta.length[i]);
        }
    }
    return SUCCESS;
}

RC ProjectMemory(char* content, 
                const RM_TblMeta &meta,
                const RM_Record& lRec, const RM_TblMeta &lMeta, std::vector<std::string> lNames, 
                const RM_Record& rRec, const RM_TblMeta &rMeta, std::vector<std::string> rNames) {
    memcpy(content, lRec.addr, sizeof(bool));
    int dColNum = meta.GetDynamicNum();
    int dCnt = 0, dOff = meta.GetPrefixLen() + meta.GetStaticPartLen();
    memcpy(content + sizeof(bool), &dColNum, sizeof(int));
    for (int i = 0; i < meta.colNum; ++i) {
        if (meta.isDynamic[i]) {
            PosInfo posInfo;
            PosInfo srcPosInfo;
            RM_Record srcRec;
            if (std::find(lNames.begin(), lNames.end(), meta.colName[i]) != lNames.end()) {
                std::string name = meta.colName[i];
                if (name.length() > 3 && name[name.length() - 3] == '(') {
                    name = name.substr(0, name.length() - 3);
                }
                srcRec = lRec;
                srcPosInfo = srcRec.GetDynamicPosInfo(lMeta, lMeta.GetPosByName(name));
            }
            else {
                std::string name = meta.colName[i];
                if (name.length() > 3 && name[name.length() - 3] == '(') {
                    name = name.substr(0, name.length() - 3);
                }
                srcRec = rRec;
                srcPosInfo = srcRec.GetDynamicPosInfo(rMeta, rMeta.GetPosByName(name));
            }
            posInfo.start = dOff;
            posInfo.end = dOff + srcPosInfo.end - srcPosInfo.start;
            memcpy(content + sizeof(int) + sizeof(bool) + dCnt * sizeof(posInfo), &posInfo, sizeof(posInfo));
            memcpy(content + dOff, srcRec.addr + srcPosInfo.start, srcPosInfo.end - srcPosInfo.start);
            dOff += srcPosInfo.end - srcPosInfo.start;
            dCnt++;
        }else {
            int off = meta.GetPrefixLen() + meta.GetStaticPartOff(meta.colPos[i]);
            RM_Record srcRec;
            RM_TblMeta srcMeta;
            if (std::find(lNames.begin(), lNames.end(), meta.colName[i]) != lNames.end()) {
                srcRec = lRec;
                srcMeta = lMeta;
            }
            else {
                srcRec = rRec;
                srcMeta = rMeta;
            }
            std::string name = meta.colName[i];
            if (name.length() > 3 && name[name.length() - 3] == '(') {
                name = name.substr(0, name.length() - 3);
            }
            int srcOff = srcMeta.GetPrefixLen() + 
                        srcMeta.GetStaticPartOff(
                            srcMeta.colPos[srcMeta.GetIdxByName(name)]
                            );
            switch (meta.type[i]) {
                case DB_INT:
                    memcpy(content + off, srcRec.addr + srcOff, sizeof(int));
                    break;
                case DB_BOOL:
                    memcpy(content + off, srcRec.addr + srcOff, sizeof(bool));
                    break;
                case DB_STRING:
                    memcpy(content + off, srcRec.addr + srcOff, meta.length[i]);
                    break;
                case DB_DOUBLE:
                    memcpy(content + off, srcRec.addr + srcOff, sizeof(double));
                    break;
            }
            //memcpy(content + off, srcRec.addr + srcOff, meta.length[i]);
        }
    }
    return SUCCESS;
}