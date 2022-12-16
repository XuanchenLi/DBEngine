#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
#include "utils/Optrs.h"
#include "utils/DB_Option.h"
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
    //std::cout<<pHdr.firstHole<<std::endl;
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

RC RM_TableHandler::InsertRec(const RM_RecAux& rAux) {
    RM_Record rec;
    int status = rec.SerializeData(metaData, rAux);
    
    if (status != SUCCESS)
        return status;
    if ((status = GetNextFreeSlot(rec.rid, rec.len)) != SUCCESS)
        return status;
    //std::cout<<rec.rid.num<<" "<< rec.rid.slot<<" "<<rec.len<<std::endl;
    return InsertRec(rec);
    
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
        //std::cout<<pageHead.slotCnt<<std::endl;
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
        //std::cout<<pageHead.slotCnt<<std::endl;
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
    //std::cout<<pageHead.slotCnt<<std::endl;
    pHdl.SetHeader(pageHead);

    //std::cout<<pHdl.GetHeader().slotCnt<<std::endl;
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
    if (fHandler == nullptr) {
        fHandler = new FM_FileHandler;
    }

    isChanged = false;
    metaData = RM_TblMeta();
    std::string tmp = path;
    if (tmp[tmp.length() - 1] == '/') {
        tmp = tmp.substr(0, tmp.length() - 1);
    }  
    
    //std::cout<<tmp<<std::endl;
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
    //std::cout<<metaData.dbName<<std::endl;
    //std::cout<<metaData.tblName<<std::endl;
    
    int status;
    
    if (status = fM_Manager->OpenFile(path, *fHandler))
        return status;
    
    //std::cout<<status<<std::endl;
    if (metaData.dbName == DIC_DB_NAME && 
        (metaData.tblName == TBL_DIC_NAME || metaData.tblName == COL_DIC_NAME))
    {
        if (metaData.tblName == TBL_DIC_NAME)
            metaData = TBL_DIC_META;
        else 
            metaData = COL_DIC_META;
        return SUCCESS;
    }
    status = InitTblMeta();
    
    return status;
}


RC RM_TableHandler::GetIter(RM_TblIterator& iter) {
    iter.tHandler = *this;
    iter.SetMeta(metaData);
    iter.Reset();
    return SUCCESS;
}

//TODO
RC RM_TableHandler::InitTblMeta() {
    RM_TableHandler tHandle((DBT_DIR + COL_DIC_NAME).c_str());
    RM_TblIterator iter;
    tHandle.GetIter(iter);
    //设置查询条件
    
    std::vector<DB_Opt> lims;
    DB_Opt opt;
    opt.colName = "dbName";
    opt.optr = EQUAL;
    opt.type = DB_STRING;
    strcpy(opt.data.sData, metaData.dbName.c_str());
    lims.push_back(opt);
    opt.colName = "tblName";
    opt.optr = EQUAL;
    opt.type = DB_STRING;
    strcpy(opt.data.sData, metaData.tblName.c_str());
    lims.push_back(opt);
    iter.SetLimits(lims);
    RM_Record rec;
    
    rec = iter.NextRec();
    //std::cout<<rec.rid.num<<" "<<rec.rid.slot<<" "<<rec.len<<std::endl;
    char tmp[64];
    while(rec.rid.num != -1) {
        //std::cout<<"1231"<<std::endl;
        rec.GetColData(tHandle.metaData, 2, tmp);
        metaData.colName[metaData.colNum] = tmp;
        rec.GetColData(tHandle.metaData, 3, &metaData.type[metaData.colNum]);
        rec.GetColData(tHandle.metaData, 4, &metaData.length[metaData.colNum]);
        rec.GetColData(tHandle.metaData, 5, &metaData.isPrimary[metaData.colNum]);
        rec.GetColData(tHandle.metaData, 6, &metaData.isDynamic[metaData.colNum]);
        rec.GetColData(tHandle.metaData, 7, &metaData.colPos[metaData.colNum]);
        metaData.colNum ++;
        //std::cout<<rec.rid.num<<" "<<rec.rid.slot<<" "<<rec.len<<std::endl;
        rec = iter.NextRec();
        //std::cout<<rec.rid.num<<" "<<rec.rid.slot<<" "<<rec.len<<std::endl;
    }
    //std::cout<<"1231"<<std::endl;
    tHandle.CloseTbl();
    return SUCCESS;
}
