#include <unistd.h>
#include <string.h>
#include "RM/RM_TblIterator.h"
#include "RM/RM_Rid.h"
#include "FM/FM_Bid.h"
#include "MM/MM_PageHandler.h"
#include "FM/FM_FileHandler.h"
#include "RM_RecHeader.h"
#include "RM/RM_Record.h"
#include "MM/MM_Buffer.h"
#include "utils/DB_Option.h"
#include "main.h"


extern MM_Buffer* gBuffer;

RC RM_TblIterator::Reset() {
    this->curPNum = 1;
    this->curSNum = -1;
    return SUCCESS;
}

RM_Record RM_TblIterator::NextRec() {
    RM_Record res;
    res.rid.num = -1;
    //td::cout<<tHandler.fHandler->GetBlockNum()<<std::endl;
    if (tHandler.fHandler->GetBlockNum() <= curPNum)
    {
        return res;
    }
    MM_PageHandler pHdr;
    RM_RecHdr tmpRHdr;
    //std::cout<<tHandler.fHandler->GetBlockNum()<<std::endl;
    while(curPNum < tHandler.fHandler->GetBlockNum()) {
        gBuffer->GetPage(FM_Bid(tHandler.fHandler->GetFd(), curPNum), pHdr);
        curSNum ++;
        //std::cout<<curPNum<<" "<<curSNum<<" "<<pHdr.GetHeader().slotCnt<<std::endl;
        int off = sizeof(MM_PageHdr) + curSNum * sizeof(RM_RecHdr);
        while(curSNum < pHdr.GetHeader().slotCnt) {
            //std::cout<<pHdr.GetHeader().slotCnt<<std::endl;
            memcpy(&tmpRHdr, pHdr.GetPtr(off), sizeof(RM_RecHdr));
            if (!tmpRHdr.isDeleted) {
            
                res.rid.num = curPNum;
                res.rid.slot = curSNum;
                res.len = tmpRHdr.len;
                //std::cout<<res.rid.num<<" "<<res.rid.slot<<" "<<res.len<<std::endl;
                //std::cout<<tmpRHdr.off<<std::endl;
                //return res;
                res.addr = pHdr.GetPtr(tmpRHdr.off);
                //return res;
                //------------------------------
                res.InitPrefix(tHandler.metaData);
                //return res;
                if (res.valid(tHandler.metaData, limits))
                    return res;
                else
                    curSNum++;
                //return res;
            }else {
                curSNum ++;
            }
            off += sizeof(RM_RecHdr);
        }
        curPNum ++;
        curSNum = -1;
    }
    //std::cout<<"end"<<std::endl;
    res.rid.num = -1;
    //std::cout<<"end"<<std::endl;
    return res;
}

RC RM_TblIterator::SetTbl(const char* tblPath) {
    Reset();
    tHandler = RM_TableHandler(tblPath);
    return SUCCESS;
}

RC RM_TblIterator::SetLimits(const std::vector<DB_Opt>& lim) {
    this->limits = lim;
    Reset();
    return SUCCESS;
}

