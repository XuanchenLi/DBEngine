#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
#include "utils/Optrs.h"
#include "RM_TableHandler.h"
#include "RM_RecHeader.h"
#include "FM/FM_Manager.h"
#include "MM/MM_PageHandler.h"
#include "MM/MM_PageHeader.h"
#include "MM/MM_Buffer.h"
#include "FM/FM_Bid.h"
#include "RM/RM_Record.h"
#include "RM/RM_Rid.h"
#include "RM/RM_TblIterator.h"
#include "main.h"


extern MM_Buffer* gBuffer;
extern FM_Manager* fM_Manager;

RC RM_TableHandler::GetNextFreeSlot(RM_Rid& res) {
    res.num = -1;
    int bNum = this->fHandler->GetNextFree();  //
    //std::cout<<bNum<<std::endl;
    res.num = bNum;
    MM_PageHandler pHdl;
    int status;
    if((status = gBuffer->GetPage(FM_Bid(this->fHandler->GetFd(), bNum), pHdl)) != SUCCESS) {
        return status;
    }
    MM_PageHdr pHdr = pHdl.GetHeader();
    if (pHdr.firstHole != -1) {
        res.slot = pHdr.firstHole;
    }else {
        res.slot = pHdr.slotCnt;
    }
    return SUCCESS;
}

RC RM_TableHandler::GetNextFreeSlot(RM_Rid& res, int len) {
    res.num = -1;
    int bNum = this->fHandler->GetNextFree(len);  //
    //std::cout<<bNum<<std::endl;
    res.num = bNum;
    MM_PageHandler pHdl;
    int status;
    if((status = gBuffer->GetPage(FM_Bid(this->fHandler->GetFd(), bNum), pHdl)) != SUCCESS) {
        return status;
    }
    MM_PageHdr pHdr = pHdl.GetHeader();
    if (pHdr.firstHole == -1)
        res.slot = pHdr.slotCnt;
    else {
        res.slot = pHdr.firstHole;
        RM_RecHdr trHdr;
        while(res.slot >= 0) {
            memcpy(&trHdr, pHdl.GetPtr(sizeof(MM_PageHdr) + res.slot*sizeof(RM_RecHdr)), sizeof(RM_RecHdr));
            if (trHdr.len >= len) {
                return SUCCESS;
            }
            memcpy(&res.slot, pHdl.GetPtr(trHdr.off), sizeof(RM_RecHdr));
        }
        if (res.slot == -1) {
            res.slot = pHdr.slotCnt;
        }
    }
    return SUCCESS;
}

RC RM_TableHandler::GetRec(const RM_Rid& rid, RM_Record& rec) {
    MM_PageHandler pHdl;
    int status;
    if(status = gBuffer->GetPage(FM_Bid(this->fHandler->GetFd(), rid.num), pHdl) != SUCCESS) {
        return status;
    }
    rec.rid = rid;
    if (rid.slot >= pHdl.GetHeader().slotCnt)
        return INVALID_OPTR;
    int off = sizeof(MM_PageHdr) + rid.slot * sizeof(RM_RecHeader);
    RM_RecHdr rHdr;
    memcpy(&rHdr, pHdl.GetPtr(off), sizeof(RM_RecHdr));
    rec.len = rHdr.len;
    rec.addr = pHdl.GetPtr(rHdr.off);
    return SUCCESS;
}


RC RM_TableHandler::InsertRec(const RM_Record& rec) {
    MM_PageHandler pHdl;
    int status;
    //std::cout<<this->fHandler->GetFd()<<std::endl;
    if(status = gBuffer->GetPage(FM_Bid(this->fHandler->GetFd(), rec.rid.num), pHdl) != SUCCESS) {
        return status;
    }
    if (rec.rid.slot < 0) return INVALID_OPTR;
    RM_RecHdr rHdr;
    rHdr.isDeleted = false;
    rHdr.len = rec.len;
    MM_PageHdr pageHead = pHdl.GetHeader();
    int off = sizeof(MM_PageHdr) + rec.rid.slot * sizeof(RM_RecHeader);
    if (rec.rid.slot == pageHead.slotCnt) {
        //std::cout<<pageHead.freeBtsCnt<<std::endl;
        rHdr.off = pageHead.freeOff - rHdr.len;
        pageHead.freeOff -= rHdr.len;
        pageHead.freeBtsCnt -= rHdr.len + sizeof(RM_RecHdr);
        //std::cout<<pageHead.freeBtsCnt<<std::endl;
        pageHead.slotCnt += 1;
        //std::cout<<pageHead.slotCnt<<std::endl;
        memcpy(pHdl.GetPtr(off), &rHdr, sizeof(RM_RecHdr));
        //std::cout<<off<<std::endl;
        memcpy(pHdl.GetPtr(rHdr.off), rec.addr, rec.len);
        //std::cout<<rHdr.off<<std::endl;

    }else {
        memcpy(&rHdr, pHdl.GetPtr(off), sizeof(RM_RecHdr));
        int nex = 0;
        memcpy(&nex, pHdl.GetPtr(rHdr.off), sizeof(int));
        pageHead.firstHole = nex;
        rHdr.isDeleted = false;
        rHdr.len = rec.len;
        //pageHead.freeBtsCnt -= rHdr.len;
        memcpy(pHdl.GetPtr(off), &rHdr, sizeof(RM_RecHdr));
        memcpy(pHdl.GetPtr(rHdr.off), rec.addr, rec.len);

    }

    if ((float)pageHead.freeBtsCnt / BLOCK_SIZE <= (1.0f - BLOCK_LIMIT)) {
        //不空闲
        //std::cout<<pageHead.freeBtsCnt<<std::endl;
        if (pageHead.nextFreePage != -1 || pageHead.preFreePage != -1) {
            int pNum = pageHead.preFreePage;
            int nNum = pageHead.nextFreePage;
            pageHead.preFreePage = -1;
            pageHead.nextFreePage = -1;
            
            MM_PageHandler tmpHdl;
            if (pNum != -1) {
                if (pNum != 0) {
                    gBuffer->GetPage(FM_Bid(this->fHandler->GetFd(), pNum), tmpHdl);
                    MM_PageHdr tmpHdr = tmpHdl.GetHeader();
                    tmpHdr.nextFreePage = nNum;
                    tmpHdl.SetHeader(tmpHdr);
                    tmpHdl.SetDirty();
                } else{
                    FM_FileHdr fHdr = this->fHandler->GetFileHdr();
                    fHdr.firstFreeHole = nNum;
                    this->fHandler->SetFileHdr(fHdr);
                    this->fHandler->SetChanged(true);
                }
            }

            if (nNum != -1) {
                gBuffer->GetPage(FM_Bid(this->fHandler->GetFd(), nNum), tmpHdl);
                MM_PageHdr tmpHdr = tmpHdl.GetHeader();
                tmpHdr.preFreePage = pNum;
                tmpHdl.SetHeader(tmpHdr);
                tmpHdl.SetDirty();
            }
        }
    }

    pHdl.SetHeader(pageHead);
    pHdl.SetDirty();
    return SUCCESS;
}

RM_TableHandler::RM_TableHandler(const char* tblPath) {
    fHandler = new FM_FileHandler;
    OpenTbl(tblPath);
    //std::cout<<status<<std::endl;
}
RM_TableHandler::~RM_TableHandler() {
    if (!(fHandler == nullptr)) {
        //std::cout<<123<<std::endl;
        //CloseTbl();
        delete fHandler;
    }
    //delete fHandler;
}

RC RM_TableHandler::CloseTbl() {
    isChanged = false;
    metaData.tblName = ""; 
    return fM_Manager->CloseFile(*fHandler);
}

RC RM_TableHandler::OpenTbl(const char* path) {

    isChanged = false;
    std::string tmp = path;
    int idx = tmp.find_last_of('/');
    if (idx != std::string::npos) {
        metaData.tblName = tmp.substr(idx + 1);
        tmp = tmp.substr(0, idx);
    }

    idx = tmp.find_last_of('/');
    metaData.dbName = tmp;
    if (idx != std::string::npos) {
        metaData.dbName = tmp.substr(idx + 1);
    }

    int status;
    if (status = fM_Manager->OpenFile(path, *fHandler))
        return status;
    //std::cout<<status<<std::endl;
    status = InitTblMeta();
    return status;
}


RC RM_TableHandler::GetIter(RM_TblIterator& iter) {
    iter.Reset();
    iter.tHandler = *this;
    return SUCCESS;
}

//TODO
RC RM_TableHandler::InitTblMeta() {
    RM_TableHandler tHandle((DBT_DIR + COL_DIC_NAME).c_str());
    RM_TblIterator iter;
    tHandle.GetIter(iter);

}
