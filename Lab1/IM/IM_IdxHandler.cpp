#include "IM_IdxHandler.h"
#include "FM/FM_FileHandler.h"
#include "utils/stringUtil.h"
#include "FM/FM_Manager.h"
#include "FM/FM_FileHeader.h"
#include "RM/RM_TableHandler.h"
#include "RM/RM_TblIterator.h"
#include "MM/MM_PageHeader.h"
#include "BTreeNode.h"
#include "main.h"
#include <tuple>
#include <math.h>
#include <string>

extern MM_Buffer* gBuffer;
extern FM_Manager* fM_Manager;

IM_IdxHandler::~IM_IdxHandler() {
    if (!(fHandler == nullptr)) {
        //std::cout<<123<<std::endl;
        //CloseTbl();
        delete fHandler;
    }
    //delete fHandler;
}
RC IM_IdxHandler::OpenIdx(const char* idxPath) {
    if (fHandler == nullptr) {
        fHandler = new FM_FileHandler;
    }
    isChanged = false;
    
    int status = fM_Manager->OpenFile(idxPath, *fHandler);
    auto names = GetIdxName(idxPath);
    //获取索引字段元信息
    RM_TableHandler tblHdl((DBT_DIR + COL_DIC_NAME).c_str());
    RM_TblIterator iter;
    std::vector<DB_Opt> lims;
    DB_Opt opt;
    opt.optr = EQUAL;
    opt.colName = "dbName";
    opt.type = DB_STRING;
    strcpy(opt.data.sData, std::get<0>(names).c_str());
    lims.push_back(opt);
    opt.optr = EQUAL;
    opt.colName = "tblName";
    opt.type = DB_STRING;
    strcpy(opt.data.sData, std::get<1>(names).c_str());
    lims.push_back(opt);
    opt.optr = EQUAL;
    opt.colName = "colPos";
    opt.type = DB_INT;
    opt.data.iData = std::get<2>(names);
    lims.push_back(opt);
    tblHdl.GetIter(iter);
    RM_Record rec = iter.NextRec();
    if (rec.rid.num == -1) {
        std::cout<<"Index Not Exist\n";
        return NOT_EXIST;
    }
    rec.GetColData(tblHdl.GetMeta(), 3, &attrType);
    switch(attrType) {
        case DB_INT:
            attrLen = sizeof(int);
            break;
        case DB_DOUBLE:
            attrLen = sizeof(double);
            break;
        case DB_BOOL:
            attrLen = sizeof(bool);
            break;
        case DB_STRING:
            rec.GetColData(tblHdl.GetMeta(), 4, &attrLen);
            break;
    }
    tblHdl.CloseTbl();

    return status;
}
RC IM_IdxHandler::CloseIdx() {
    isChanged = false;
    return fM_Manager->CloseFile(*fHandler);
}

IM_IdxHandler::IM_IdxHandler(const char* idxPath) {
    fHandler = new FM_FileHandler;
    OpenIdx(idxPath);
    //std::cout<<status<<std::endl;
}

RC IM_IdxHandler::FindInsertPos(void* pData, BTreeNode& L) {
    FM_FileHdr fHdr = fHandler->GetFileHdr();
    MM_PageHandler pHdl;
    gBuffer->GetPage(FM_Bid(fHandler->GetFd(), fHdr.preF), pHdl);
    L.SetData(pHdl);
    RM_Rid nex = L.GetSon(pData);
    while (nex.num != -1) {
        gBuffer->GetPage(FM_Bid(fHandler->GetFd(), nex.num), pHdl);
        L.SetData(pHdl);
        nex = L.GetSon(pData);
    }
    //std::cout<<"ggg"<<std::endl;
    return SUCCESS;
}

RC IM_IdxHandler::FindLeaf(void* pData, const RM_Rid &rid, BTreeNode& L) {
    FM_FileHdr fHdr = fHandler->GetFileHdr();
    MM_PageHandler pHdl;
    gBuffer->GetPage(FM_Bid(fHandler->GetFd(), fHdr.preF), pHdl);
    L.SetData(pHdl);
    RM_Rid nex = L.GetFirstSon(pData);
    while (nex.num != -1) {
        gBuffer->GetPage(FM_Bid(fHandler->GetFd(), nex.num), pHdl);
        L.SetData(pHdl);
        nex = L.GetFirstSon(pData);
    }

    while(true) {
        if (L.Contain(pData, rid)) return SUCCESS;
        if (L.Ptrs.size()%2 == 0) break;
        gBuffer->GetPage(FM_Bid(fHandler->GetFd(), L.Ptrs.back().num), pHdl);
        L.SetData(pHdl);
    }
    //std::cout<<"ggg"<<std::endl;
    return NOT_EXIST;
}



RC IM_IdxHandler::InsertEntry(void *pData, const RM_Rid &rid) {
    FM_FileHdr fHdr = fHandler->GetFileHdr();
    BTreeNode L(attrType, attrLen);
    //std::cout<<fHdr.blkCnt<<std::endl;
    if (fHdr.blkCnt == 1) {  //树为空
        //创建空叶节点
        //std::cout<<"123"<<std::endl;
        MM_PageHandler pHdl;
        int num = fHandler->GetNextWhole();
        fHdr.preF = num;
        fHandler->SetFileHdr(fHdr);
        fHandler->SetChanged(true);
        gBuffer->GetPage(FM_Bid(fHandler->GetFd(), num), pHdl);
        L.SetData(pHdl);

    }
    else {
        //找到应该包含pData的叶节点
        //std::cout<<"112"<<std::endl;
        FindInsertPos(pData, L);
    }
    if (L.GetKeyNum() < L.GetMaxPNum() - 1) {
        //std::cout<<L.GetKeyNum()<< " "<< L.GetMaxPNum()<< std::endl;
        L.InsertPair(pData, rid);
        MM_PageHandler pHdl;
        gBuffer->GetPage(FM_Bid(fHandler->GetFd(), L.GetBlkNum()), pHdl);
        return L.SetPage(pHdl);
    }
    else {
        //std::cout<<"dsd"<< std::endl;
        //分裂
        //创建新节点
        BTreeNode L2(attrType, attrLen);
        MM_PageHandler pHdl;
        int num = fHandler->GetNextWhole();
        //std::cout<<num<< std::endl;
        gBuffer->GetPage(FM_Bid(fHandler->GetFd(), num), pHdl);
        L2.SetData(pHdl);
        //
        BTreeNode T(L);
        T.Ptrs.pop_back();
        T.InsertPair(pData, rid);
        L2.Ptrs.push_back(L.Ptrs.back());
        L.Ptrs.pop_back(); L.Ptrs.push_back(RM_Rid(L2.bid.num, 0));
        L.EraseNPair(L.GetMaxPNum() - 1);
        int half = ceil(L.GetMaxPNum() / 2.0);
        auto p1 = T.Ptrs.begin();
        auto p2 = T.keys.begin();
        for(int i = 0; i < half; ++i)
        {
            p1++;
            p2++;
        }
        L.Ptrs.splice(L.Ptrs.begin(), T.Ptrs, T.Ptrs.begin(), p1);
        L.keys.splice(L.keys.begin(), T.keys, T.keys.begin(), p2);
        L.pHdr.firstHole = L.keys.size();
        L2.Ptrs.splice(L2.Ptrs.begin(), T.Ptrs, T.Ptrs.begin(), T.Ptrs.end());
        L2.keys.splice(L2.keys.begin(), T.keys, T.keys.begin(), T.keys.end());
        L2.pHdr.firstHole = L2.keys.size();
        void* k2 = nullptr;
        switch(attrType) {
            case DB_INT:
                k2 = new int(*(int*)L2.keys.front());
                break;
            case DB_DOUBLE:
                k2 = new double(*(double*)L2.keys.front());
                break;
            case DB_STRING:
                k2 = new char[attrLen];
                strcpy((char*)k2, (char*)L2.keys.front());
                break;
            case DB_BOOL:
                k2 = new bool(*(bool*)L2.keys.front());
                break;
        }
        //写回缓存
        gBuffer->GetPage(FM_Bid(fHandler->GetFd(), L.GetBlkNum()), pHdl);
        L.SetPage(pHdl);
        gBuffer->GetPage(FM_Bid(fHandler->GetFd(), L2.GetBlkNum()), pHdl);
        L2.SetPage(pHdl);
        //std::cout<<L.bid.num<<L2.bid.num<< std::endl;
        return InsertInParent(L, k2, L2);
    }
    return SUCCESS;

}

RC IM_IdxHandler::InsertInParent(BTreeNode&N1, void*key, BTreeNode& N2) {
    if (N1.isRoot()) {
        //创建新根节点
        //std::cout<<"123"<<std::endl;
        BTreeNode R(attrType, attrLen);
        MM_PageHandler pHdl;
        int num = fHandler->GetNextWhole();
        gBuffer->GetPage(FM_Bid(fHandler->GetFd(), num), pHdl);
        R.SetData(pHdl);
        R.keys.push_back(key);
        R.Ptrs.push_back(RM_Rid(N1.bid.num, 0));
        R.Ptrs.push_back(RM_Rid(N2.bid.num, 0));
        R.pHdr.firstHole = 1;
        R.pHdr.nextFreePage = 1;
        N1.pHdr.preFreePage = R.bid.num;
        N2.pHdr.preFreePage = R.bid.num;
        R.SetPage(pHdl);
        gBuffer->GetPage(FM_Bid(fHandler->GetFd(), N1.GetBlkNum()), pHdl);
        N1.SetPage(pHdl);
        gBuffer->GetPage(FM_Bid(fHandler->GetFd(), N2.GetBlkNum()), pHdl);
        N2.SetPage(pHdl);
        return SUCCESS;
    }
    //插入父节点
    BTreeNode P(attrType, attrLen);
    MM_PageHandler pHdl;
    int num = N1.pHdr.preFreePage;
    //std::cout<<N1.bid.num<<" "<<num<<std::endl;
    gBuffer->GetPage(FM_Bid(fHandler->GetFd(), num), pHdl);
    P.SetData(pHdl);
    if (P.keys.size() < P.GetMaxPNum()) {
        //父节点有空位
        //std::cout<<"111"<<std::endl;
        auto iter = P.Ptrs.begin();
        auto iter2 = P.keys.begin();
        while((*iter).num != N1.bid.num) iter++, iter2++;
        //std::cout<<"222"<<std::endl;
        iter++, iter2++;
        P.Ptrs.insert(iter, RM_Rid(N2.bid.num, 0));
        P.keys.insert(iter2, key);
        P.pHdr.firstHole++;
        P.SetPage(pHdl);
        N2.pHdr.preFreePage = num;
        gBuffer->GetPage(FM_Bid(fHandler->GetFd(), N2.bid.num), pHdl);
        return N2.SetPage(pHdl);
    }else {
        //分裂
        //std::cout<<"1232"<<std::endl;
        BTreeNode T(P);
        auto iter = T.Ptrs.begin();
        auto iter2 = T.keys.begin();
        
        while((*iter).num != N1.bid.num) iter++, iter2++;
        iter++, iter2++;
        T.Ptrs.insert(iter, RM_Rid(N2.bid.num, 0));
        T.keys.insert(iter2, key);
        T.pHdr.firstHole++;

        P.EraseNPair(P.keys.size());
        P.Ptrs.clear();
        
        BTreeNode P2(attrType, attrLen);
        MM_PageHandler pHdl;
        int num = fHandler->GetNextWhole();
        gBuffer->GetPage(FM_Bid(fHandler->GetFd(), num), pHdl);
        P2.SetData(pHdl);

        int half = ceil((P.GetMaxPNum() + 1) / 2.0);
        auto p1 = T.Ptrs.begin();
        auto p2 = T.keys.begin();
        for(int i = 0; i < half; ++i)
        {
            p1++;
            p2++;
        }
        p2--;
        P.Ptrs.splice(P.Ptrs.begin(), T.Ptrs, T.Ptrs.begin(), p1);
        P.keys.splice(P.keys.begin(), T.keys, T.keys.begin(), p2);
        P.pHdr.firstHole = P.keys.size();
        void* kn = T.keys.front();
        T.keys.pop_front();
        P2.Ptrs.splice(P2.Ptrs.begin(), T.Ptrs, T.Ptrs.begin(), T.Ptrs.end());
        P2.keys.splice(P2.keys.begin(), T.keys, T.keys.begin(), T.keys.end());
        P2.pHdr.firstHole = P2.keys.size();

         //写回缓存
        gBuffer->GetPage(FM_Bid(fHandler->GetFd(), P.GetBlkNum()), pHdl);
        P.SetPage(pHdl);
        gBuffer->GetPage(FM_Bid(fHandler->GetFd(), P.GetBlkNum()), pHdl);
        P.SetPage(pHdl);

        return InsertInParent(P, kn, P2);
    }

}
RC IM_IdxHandler::DeleteEntry(void *pData, const RM_Rid &rid) {
    BTreeNode N(attrType, attrLen);
    FindLeaf(pData, rid, N);
    return DeleteEntry(N, pData, rid);
}

RC IM_IdxHandler::DeleteEntry(BTreeNode& N, void *pData, const RM_Rid &rid) {
    FM_FileHdr fHdr = fHandler->GetFileHdr();
    //TODO 当前只考虑了非重复键
    N.DeleteSinglePair(pData, rid);
    if (N.isRoot() && N.Ptrs.size() == 1) {
        fHdr.preF = (*N.Ptrs.begin()).num;
        fHandler->SetChanged(true);
        fHandler->SetFileHdr(fHdr);
        fHandler->ReturnWhole(N.bid.num);
        return SUCCESS;

    }
}