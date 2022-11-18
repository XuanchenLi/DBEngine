#include "IM_IdxHandler.h"
#include "FM/FM_FileHandler.h"
#include "utils/stringUtil.h"
#include "FM/FM_Manager.h"
#include "FM/FM_FileHeader.h"
#include "RM/RM_TableHandler.h"
#include "RM/RM_TblIterator.h"
#include "MM/MM_PageHeader.h"
#include "IM_IdxIterator.h"
#include "BTreeNode.h"
#include "main.h"
#include <tuple>
#include <math.h>
#include <string>

extern MM_Buffer *gBuffer;
extern FM_Manager *fM_Manager;

IM_IdxHandler::~IM_IdxHandler()
{
    if (!(fHandler == nullptr))
    {
        // std::cout<<123<<std::endl;
        // CloseTbl();
        delete fHandler;
    }
    // delete fHandler;
}
RC IM_IdxHandler::OpenIdx(const char *idxPath)
{
    if (fHandler == nullptr)
    {
        fHandler = new FM_FileHandler;
    }
    isChanged = false;
    this->idxPath = idxPath;
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
    if (rec.rid.num == -1)
    {
        std::cout << "Index Not Exist\n";
        return NOT_EXIST;
    }
    colPos = std::get<2>(names);
    rec.GetColData(tblHdl.GetMeta(), 3, &attrType);
    switch (attrType)
    {
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
RC IM_IdxHandler::CloseIdx()
{
    isChanged = false;
    return fM_Manager->CloseFile(*fHandler);
}

IM_IdxHandler::IM_IdxHandler(const char *idxPath)
{
    fHandler = new FM_FileHandler;
    OpenIdx(idxPath);
    // std::cout<<status<<std::endl;
}

RC IM_IdxHandler::FindInsertPos(void *pData, BTreeNode &L)
{
    //printf("haha %s\n", pData);
    FM_FileHdr fHdr = fHandler->GetFileHdr();
    MM_PageHandler pHdl;
    gBuffer->GetPage(FM_Bid(fHandler->GetFd(), fHdr.preF), pHdl);
    L.SetData(pHdl);
    RM_Rid nex = L.GetSon(pData);
    //std::cout<<nex.num<<std::endl;
    /*
    printf("head %d\n", L.bid.num);
    printf("total blk num %d\n", fHdr.blkCnt);
    for (auto i : L.keys) {
        printf("root %s\n", i);
    }
    */
    int pNum = L.bid.num;
    while (nex.num != -1)
    {
        gBuffer->GetPage(FM_Bid(fHandler->GetFd(), nex.num), pHdl);
        L.SetData(pHdl);
        L.pHdr.preFreePage = pNum;
        L.SetPage(pHdl);
        pNum = L.bid.num;
        nex = L.GetSon(pData);
    }
    // std::cout<<"ggg"<<std::endl;
    return SUCCESS;
}

RC IM_IdxHandler::FindLeaf(void *pData, const RM_Rid &rid, BTreeNode &L)
{
    FM_FileHdr fHdr = fHandler->GetFileHdr();
    MM_PageHandler pHdl;
    gBuffer->GetPage(FM_Bid(fHandler->GetFd(), fHdr.preF), pHdl);
    L.SetData(pHdl);
    
    RM_Rid nex = L.GetFirstSon(pData);
    //std::cout<<L.GetBlkNum()<<std::endl;
    //std::cout<<nex.num<<std::endl;
    int pNum = L.bid.num;
    while (nex.num != -1)
    {
        gBuffer->GetPage(FM_Bid(fHandler->GetFd(), nex.num), pHdl);
        L.SetData(pHdl);
        L.pHdr.preFreePage = pNum;
        L.SetPage(pHdl);
        pNum = L.bid.num;
        nex = L.GetFirstSon(pData);
        //std::cout<< nex.num<<std::endl;
    }
    /*
    if (!L.Contain(pData, rid)) {
        printf("query %s %d %d\n", pData, rid.num, rid.slot);
        auto j = L.Ptrs.begin();
        for (auto i : L.keys) {
            printf("keys %s--------rid %d %d\n", i, (*j).num, (*j).slot);
            j++;
        }
    }
    */
        
    //std::cout<<L.bid.num<<std::endl;
    //return NOT_EXIST;
    //std::cout<<L.GetBlkNum()<<std::endl;
    while (true)
    {

        //printf("query %s\n", pData);
        /*
        for (auto i : L.keys) {
            printf("keys %s\n", i);
        }
        */
        if (L.Contain(pData, rid))
            return SUCCESS;
        if (L.Ptrs.size() - L.keys.size() == 0)
            break;
        if (gBuffer->GetPage(FM_Bid(fHandler->GetFd(), L.Ptrs.back().num), pHdl) != SUCCESS)
            return FAILURE;
        //std::cout<<L.Ptrs.back().num<<std::endl;
        L.SetData(pHdl);
    }
    printf("Leaf That Contain Key %s Ptr %d %d Not Exist\n", pData, rid.num, rid.slot);
    return NOT_EXIST;
}

RC IM_IdxHandler::InsertEntry(void *pData, const RM_Rid &rid)
{
    FM_FileHdr fHdr = fHandler->GetFileHdr();
    BTreeNode L(attrType, attrLen);
    //std::cout<<fHdr.preF<<std::endl;
    if (fHdr.blkCnt == 1)
    { //树为空
        //创建空叶节点
        // std::cout<<"123"<<std::endl;
        MM_PageHandler pHdl;
        int num = fHandler->GetNextWhole();
        //std::cout<<"new block "<<num<<" "<<fHandler->GetFileHdr().blkCnt<< std::endl;
        fHdr = fHandler->GetFileHdr();
        //std::cout<<fHandler->GetFileHdr().blkCnt<<std::endl;
        fHdr.preF = num;
        fHandler->SetFileHdr(fHdr);
        fHandler->SetChanged(true);
        int s = gBuffer->GetPage(FM_Bid(fHandler->GetFd(), num), pHdl);
        //std::cout<<fHandler->GetFileHdr().blkCnt<<std::endl;
        L.SetData(pHdl);
        //std::cout<<"123"<<std::endl;
    }
    else
    {
        //找到应该包含pData的叶节点
        //std::cout<<"112"<<std::endl;
        MM_PageHandler pHdl;
        BTreeNode LL(attrType, attrLen);

        FindInsertPos(pData, L);

        //std::cout<<L.GetBlkNum()<<std::endl;
        
    }
    //std::cout<<"123"<<std::endl;
    if (L.GetKeyNum() < L.GetMaxPNum() - 1)
    {
        // std::cout<<L.GetKeyNum()<< " "<< L.GetMaxPNum()<< std::endl;
        
        //printf("-------------------\n");   
        //printf("%s Insert in Leaf blk %d \n", pData, L.bid.num);
        
        L.InsertPair(pData, rid);
        /*
        for (auto i : L.keys) {
            printf("0 %s\n", i);
        }
        */
        

        MM_PageHandler pHdl;
        gBuffer->GetPage(FM_Bid(fHandler->GetFd(), L.GetBlkNum()), pHdl);
        return L.SetPage(pHdl);
    }
    else
    {
        //printf("%s Insert cause Leaf %d split\n", pData, L.bid.num);
        // std::cout<<"dsd"<< std::endl;
        //分裂
        //创建新节点
        /*
        for (auto i : L.keys) {
        printf("0 %s\n", i);
        }
        */
        
        
        BTreeNode L2(attrType, attrLen);
        MM_PageHandler pHdl;
        int num = fHandler->GetNextWhole();
        //std::cout<<"new block "<<num<<" "<<fHandler->GetFileHdr().blkCnt<< std::endl;
        gBuffer->GetPage(FM_Bid(fHandler->GetFd(), num), pHdl);
        L2.SetData(pHdl);
        //std::cout<<"hhhhhhhhhh"<< std::endl;
        //
        BTreeNode T(L);
        if (T.Ptrs.size() == T.keys.size() + 1)
            T.Ptrs.pop_back();
        T.InsertPair(pData, rid);
        //std::cout<<"T "<<T.Ptrs.size()<<" "<<T.keys.size()<<std::endl;
        //if (L2.bid.num == 140)
        //std::cout<<"L0 "<<L.Ptrs.size()<<" "<<L.keys.size()<<std::endl;
        if (L.Ptrs.size() == L.keys.size() + 1) {
            L2.Ptrs.push_back(L.Ptrs.back());
            L.Ptrs.pop_back();
        }
        L.Ptrs.push_back(RM_Rid(L2.bid.num, 0));
        //std::cout<<L.bid.num<<" next "<<L2.bid.num<<std::endl;
        //if (L2.Ptrs.size()!=L2.keys.size())
        //std::cout<<L2.bid.num<<" next2 "<<L2.Ptrs.back().num<<std::endl;
        //std::cout<<"hhhhhhhhhh"<< std::endl;
        L.EraseNPair(L.GetMaxPNum() - 1);
        //std::cout<<"hhhhhhhhhh"<< std::endl;
        //std::cout<<"L "<<L.Ptrs.size()<<" "<<L.keys.size()<<std::endl;
        int half = ceil(L.GetMaxPNum() / 2.0);
        auto p1 = T.Ptrs.begin();
        auto p2 = T.keys.begin();
        for (int i = 0; i < half; ++i)
        {
            p1++;
            p2++;
        }
        L.Ptrs.splice(L.Ptrs.begin(), T.Ptrs, T.Ptrs.begin(), p1);
        L.keys.splice(L.keys.begin(), T.keys, T.keys.begin(), p2);
        L.pHdr.firstHole = L.keys.size();
        //if (L2.bid.num == 6)
        //std::cout<<"L "<<L.Ptrs.size()<<" "<<L.keys.size()<<std::endl;
        L2.Ptrs.splice(L2.Ptrs.begin(), T.Ptrs, T.Ptrs.begin(), T.Ptrs.end());
        L2.keys.splice(L2.keys.begin(), T.keys, T.keys.begin(), T.keys.end());
        L2.pHdr.firstHole = L2.keys.size();
        L2.pHdr.preFreePage = L.pHdr.preFreePage;
        //if (L2.bid.num == 140)
        //std::cout<<"L2 "<<L2.Ptrs.size()<<" "<<L2.keys.size()<<std::endl;
        //std::cout<<L2.bid.num<<std::endl;
        void *k2 = nullptr;
        switch (attrType)
        {
        case DB_INT:
            k2 = new int(*(int *)L2.keys.front());
            break;
        case DB_DOUBLE:
            k2 = new double(*(double *)L2.keys.front());
            break;
        case DB_STRING:
            k2 = new char[attrLen];
            strcpy((char *)k2, (char *)L2.keys.front());
            break;
        case DB_BOOL:
            k2 = new bool(*(bool *)L2.keys.front());
            break;
        }
        //写回缓存
        //std::cout<<L.bid.num<<L2.bid.num<< std::endl;
        //std::cout<<"hhhhhhhhhh"<< std::endl;
        gBuffer->GetPage(FM_Bid(fHandler->GetFd(), L.GetBlkNum()), pHdl);
        L.SetPage(pHdl);
        gBuffer->GetPage(FM_Bid(fHandler->GetFd(), L2.GetBlkNum()), pHdl);
        L2.SetPage(pHdl);
        //std::cout<<"hhhhhhhhhh"<< std::endl;
        /*
        std::cout<<L.bid.num<<std::endl;
        std::cout<<L.GetKeyNum()<<std::endl;
        for (auto i : L.keys) {
        printf("12 %s\n", i);
        }
        std::cout<<L2.bid.num<<std::endl;
        std::cout<<L2.GetKeyNum()<<std::endl;
        for (auto i : L2.keys) {
        printf("22 %s\n", i);
        }
        */
        //printf("key %s\n", k2);
        
        //std::cout<<L.bid.num<<L2.bid.num<< std::endl;
        return InsertInParent(L, k2, L2);
    }
    return SUCCESS;
}

RC IM_IdxHandler::InsertInParent(BTreeNode &N1, void *key, BTreeNode &N2)
{
    //std::cout<<"123"<<std::endl;
    if (N1.isRoot())
    {
        //创建新根节点
        //std::cout<<"223"<<std::endl;
        BTreeNode R(attrType, attrLen);
        MM_PageHandler pHdl;
        int num = fHandler->GetNextWhole();
        //std::cout<<"new head "<<num<<std::endl;
        gBuffer->GetPage(FM_Bid(fHandler->GetFd(), num), pHdl);
        R.SetData(pHdl);
        R.keys.push_back(key);
        R.Ptrs.push_back(RM_Rid(N1.bid.num, 0));
        R.Ptrs.push_back(RM_Rid(N2.bid.num, 0));
        R.pHdr.firstHole = 1;
        R.pHdr.freeBtsCnt = 2;
        R.pHdr.nextFreePage = 1;
        N1.pHdr.preFreePage = R.bid.num;
        N2.pHdr.preFreePage = R.bid.num;
        R.SetPage(pHdl);
        gBuffer->GetPage(FM_Bid(fHandler->GetFd(), N1.GetBlkNum()), pHdl);
        N1.SetPage(pHdl);
        gBuffer->GetPage(FM_Bid(fHandler->GetFd(), N2.GetBlkNum()), pHdl);
        N2.SetPage(pHdl);
        auto fHdr = fHandler->GetFileHdr();
        fHdr.preF = R.bid.num;
        //std::cout<<fHdr.preF<<std::endl;
        fHandler->SetChanged(true);
        fHandler->SetFileHdr(fHdr);

        return SUCCESS;
    }
    //插入父节点
    BTreeNode P(attrType, attrLen);
    MM_PageHandler pHdl;
    int num = N1.pHdr.preFreePage;
    //std::cout<<N1.bid.num<<" "<<num<<std::endl;
    gBuffer->GetPage(FM_Bid(fHandler->GetFd(), num), pHdl);
    P.SetData(pHdl);
    if (P.Ptrs.size() < P.GetMaxPNum())
    {
        //父节点有空位
        // std::cout<<"111"<<std::endl;
        auto iter = P.Ptrs.begin();
        auto iter2 = P.keys.begin();
        
        while ((*iter).num != N1.bid.num)
            iter++, iter2++;
        //std::cout<<"222"<<std::endl;
        iter++;
        P.Ptrs.insert(iter, RM_Rid(N2.bid.num, 0));
        P.keys.insert(iter2, key);
        P.pHdr.firstHole++;
        P.pHdr.freeBtsCnt++;
        P.SetPage(pHdl);
        N2.pHdr.preFreePage = num;
        gBuffer->GetPage(FM_Bid(fHandler->GetFd(), N2.bid.num), pHdl);
        return N2.SetPage(pHdl);
    }
    else
    {
        //分裂
        //std::cout<<"1232"<<std::endl;
        BTreeNode T(P);
        auto iter = T.Ptrs.begin();
        auto iter2 = T.keys.begin();

        while ((*iter).num != N1.bid.num)
            iter++, iter2++;
        iter++;
        T.Ptrs.insert(iter, RM_Rid(N2.bid.num, 0));
        T.keys.insert(iter2, key);
        T.pHdr.firstHole++;
        T.pHdr.freeBtsCnt++;

        P.EraseNPair(P.keys.size());
        P.Ptrs.clear();
        //std::cout<<P.Ptrs.size()<<P.keys.size()<<std::endl;

        BTreeNode P2(attrType, attrLen);
        MM_PageHandler pHdl;
        int num = fHandler->GetNextWhole();
        gBuffer->GetPage(FM_Bid(fHandler->GetFd(), num), pHdl);
        //std::cout<<"new block "<<num<<std::endl;
        P2.SetData(pHdl);

        int half = ceil(P.GetMaxPNum() / 2.0);
        auto p1 = T.Ptrs.begin();
        auto p2 = T.keys.begin();
        for (int i = 0; i < half; ++i)
        {
            p1++;
            p2++;
        }
        p2--;
        P.Ptrs.splice(P.Ptrs.begin(), T.Ptrs, T.Ptrs.begin(), p1);
        P.keys.splice(P.keys.begin(), T.keys, T.keys.begin(), p2);
        P.pHdr.firstHole = P.keys.size();
        P.pHdr.freeBtsCnt = P.Ptrs.size();

        void *kn = nullptr;
        switch(attrType) {
            case DB_INT:
                kn = new int;
                memcpy(kn, T.keys.front(), sizeof(int));
                break;
            case DB_DOUBLE:
                kn = new double;
                memcpy(kn, T.keys.front(), sizeof(double));
                break;
            case DB_STRING:
                kn = new char[attrLen];
                strcpy((char*)kn, (char*)T.keys.front());
                break;
            case DB_BOOL:
                kn = new bool;
                memcpy(kn, T.keys.front(), sizeof(bool));
                break;
        }
        T.keys.pop_front();

        P2.Ptrs.splice(P2.Ptrs.begin(), T.Ptrs, T.Ptrs.begin(), T.Ptrs.end());
        P2.keys.splice(P2.keys.begin(), T.keys, T.keys.begin(), T.keys.end());
        P2.pHdr.firstHole = P2.keys.size();
        P2.pHdr.freeBtsCnt = P2.Ptrs.size();
        P2.pHdr.nextFreePage = 1;

        //写回缓存
        gBuffer->GetPage(FM_Bid(fHandler->GetFd(), P.GetBlkNum()), pHdl);
        P.SetPage(pHdl);
        gBuffer->GetPage(FM_Bid(fHandler->GetFd(), P2.GetBlkNum()), pHdl);
        P2.SetPage(pHdl);
        

        return InsertInParent(P, kn, P2);
    }
}
RC IM_IdxHandler::DeleteEntry(void *pData, const RM_Rid &rid)
{
    BTreeNode N(attrType, attrLen);
    FindLeaf(pData, rid, N);
    //printf("----%s in leaf %d----\n", pData, N.bid.num);
    /*
    auto j = N.Ptrs.begin();
    printf("----%s in leaf %d----\n", pData, N.bid.num);
    for (auto key: N.keys) {
        printf("key %s ----- ptr %d %d\n", key, (*j).num, (*j).slot);
        j++;
    }
    */
    //return SUCCESS;
    return DeleteEntry(N, pData, rid);
}

RC IM_IdxHandler::DeleteEntry(BTreeNode &N, void *pData, const RM_Rid &rid)
{
    FM_FileHdr fHdr = fHandler->GetFileHdr();
    // TODO 当前只考虑了非重复键
    //std::cout<<"1232"<<std::endl;
    int s = N.DeleteSinglePair(pData, rid);
    //std::cout<<"1233"<<std::endl;
    //std::cout<<N.pHdr.preFreePage<<std::endl;
    if (N.isRoot() && N.Ptrs.empty()) {
        //std::cout<<"1233"<<std::endl;
        fHdr.preF = -1;
        fHdr.firstFreeHole = 0;
        fHdr.blkCnt = 1;
        fHandler->SetChanged(true);
        fHandler->SetFileHdr(fHdr);
        return SUCCESS;
    }
    //std::cout<<"1232"<<std::endl;
    if (N.isRoot() && N.keys.size() == 0 && N.Ptrs.size() == 1)
    {
        fHdr.preF = (*N.Ptrs.begin()).num;
        fHandler->SetChanged(true);
        fHandler->SetFileHdr(fHdr);
        fHandler->ReturnWhole(N.bid.num);
        MM_PageHandler pHdl;
        gBuffer->GetPage(FM_Bid(fHandler->GetFd(), fHdr.preF), pHdl);
        N.SetData(pHdl);
        N.pHdr.preFreePage = 0;
        N.SetPage(pHdl);

        return SUCCESS;
    }
    else if (!N.isRoot() && N.Ptrs.size() < ceil(N.GetMaxPNum() / 2.0))
    {
        //std::cout << "merge\n";
        int pNum = N.GetParent();
        BTreeNode P(attrType, attrLen);
        BTreeNode N2(attrType, attrLen);
        MM_PageHandler pHdl;
        gBuffer->GetPage(FM_Bid(fHandler->GetFd(), pNum), pHdl);
        P.SetData(pHdl);
        auto pair1 = P.GetLeftBro(RM_Rid(N.GetBlkNum(), 0));
        auto pair2 = P.GetRightBro(RM_Rid(N.GetBlkNum(), 0));
        if (pair1.first == nullptr && pair2.first == nullptr)
        {
            std::cout << "Can't find brother.\n";
            return FAILURE;
        }
        bool isLeft = false;
        void *k2;
        //std::cout<<"1232"<<std::endl;
        if (pair1.first != nullptr)
        {
            
            int ss = gBuffer->GetPage(FM_Bid(fHandler->GetFd(), pair1.second.num), pHdl);
            //std::cout<<"Blk Num: "<<pHdl.GetBid().num<<std::endl;
            N2.SetData(pHdl);
            //std::cout<<"1233"<<std::endl;
            k2 = pair1.first;
            isLeft = true;
        }
        if (pair2.first != nullptr)
        {
            //std::cout<<"1232"<<std::endl;
            if (isLeft)
            {
                BTreeNode N3(attrType, attrLen);
                gBuffer->GetPage(FM_Bid(fHandler->GetFd(), pair2.second.num), pHdl);
                N3.SetData(pHdl);
                if (N3.Ptrs.size() < N2.Ptrs.size())
                {
                    N2 = N3;
                    k2 = pair2.first;
                    isLeft = false;
                }
            }
            else
            {
                gBuffer->GetPage(FM_Bid(fHandler->GetFd(), pair2.second.num), pHdl);
                k2 = pair2.first;
                N2.SetData(pHdl);
            }
        }
        int n1s = N.Ptrs.size();
        int n2s = N2.Ptrs.size();
        //std::cout<<"1232"<<std::endl;
        if (N.isLeaf() && N.Ptrs.size() == N.keys.size() + 1 && N2.Ptrs.size() == N2.keys.size() + 1) {
            n1s--;
        }
        if (n1s + n2s <= N.GetMaxPNum())
        {
            //能够合并
            //std::cout<<"can merge"<<std::endl;
            if (!isLeft)
            {
                // N是N'的前一个节点
                BTreeNode TMP(N);
                N = N2;
                N2 = TMP;
            }
            if (!N.isLeaf())
            {
                void *kn = nullptr;
                switch(attrType) {
                case DB_INT:
                    kn = new int;
                    memcpy(kn, k2, sizeof(int));
                break;
                case DB_DOUBLE:
                    kn = new double;
                    memcpy(kn, k2, sizeof(double));
                break;
                case DB_STRING:
                    kn = new char[attrLen];
                    strcpy((char*)kn, (char*)k2);
                    break;
                case DB_BOOL:
                    kn = new bool;
                    memcpy(kn, k2, sizeof(bool));
                break;
                }
                N2.keys.push_back(kn);
            }
            else
            {
                if (N2.Ptrs.size() == N2.keys.size() + 1)
                    N2.Ptrs.pop_back();
            }
            N2.keys.splice(N2.keys.end(), N.keys, N.keys.begin(), N.keys.end());
            N2.Ptrs.splice(N2.Ptrs.end(), N.Ptrs, N.Ptrs.begin(), N.Ptrs.end());
            gBuffer->GetPage(FM_Bid(fHandler->GetFd(), N2.bid.num), pHdl);
            N2.SetPage(pHdl);

            DeleteEntry(P, k2, RM_Rid(N.GetBlkNum(), 0));
            fHandler->ReturnWhole(N.bid.num);
            
            /*
            switch (attrType)
            {
            case DB_INT:
                delete (int *)k2;
                break;
            case DB_DOUBLE:
                delete (double *)k2;
                break;
            case DB_STRING:
                delete[](char *) k2;
                break;
            case DB_BOOL:
                delete (bool *)k2;
                break;
            }
            */
            
            return SUCCESS;
        }
        else
        {
            //重新分布
            //std::cout<<"borrow"<<std::endl;
            if (isLeft) {
                //std::cout<<"borrow from left"<<std::endl;
                if (!N.isLeaf()) {
                    //std::cout<<"borrow from left non-leaf"<<std::endl;
                    RM_Rid pm = N2.Ptrs.back();
                    void* km_1 = N2.keys.back();
                    N2.Ptrs.pop_back();
                    N2.keys.pop_back();
                    void *kn = nullptr;
                    switch(attrType) {
                    case DB_INT:
                        kn = new int;
                        memcpy(kn, k2, sizeof(int));
                        break;
                    case DB_DOUBLE:
                        kn = new double;
                        memcpy(kn, k2, sizeof(double));
                        break;
                    case DB_STRING:
                        kn = new char[attrLen];
                        strcpy((char*)kn, (char*)k2);
                        break;
                    case DB_BOOL:
                        kn = new bool;
                        memcpy(kn, k2, sizeof(bool));
                        break;
                    }
                    N.keys.push_front(kn);
                    N.Ptrs.push_front(pm);
                    P.ReplaceKey(k2, km_1);
                    //TODO delete k2
                }else {
                    auto iter = N2.Ptrs.rbegin();
                    if (N2.Ptrs.size() == N2.keys.size() + 1)
                        iter++;
                    RM_Rid pm = *iter;
                    void* km = N2.keys.back();
                    iter++;
                    N2.Ptrs.erase(iter.base());
                    N2.keys.pop_back();
                    N.keys.push_front(km);
                    N.Ptrs.push_front(pm);
                    void *kt = nullptr;
                    switch(attrType) {
                    case DB_INT:
                        kt = new int;
                        memcpy(kt, km, sizeof(int));
                        break;
                    case DB_DOUBLE:
                        kt = new double;
                        memcpy(kt, km, sizeof(double));
                        break;
                    case DB_STRING:
                        kt = new char[attrLen];
                        strcpy((char*)kt, (char*)km);
                        break;
                    case DB_BOOL:
                        kt = new bool;
                        memcpy(kt, km, sizeof(bool));
                        break;
                    }
                    P.ReplaceKey(k2, kt);
                }
                gBuffer->GetPage(FM_Bid(fHandler->GetFd(), P.GetBlkNum()), pHdl);
                P.SetPage(pHdl);
                gBuffer->GetPage(FM_Bid(fHandler->GetFd(), N.GetBlkNum()), pHdl);
                N.SetPage(pHdl);
                gBuffer->GetPage(FM_Bid(fHandler->GetFd(), N2.GetBlkNum()), pHdl);
                N2.SetPage(pHdl);
                return SUCCESS;
            }else {
               if (!N.isLeaf()) {
                    RM_Rid pm = N2.Ptrs.front();
                    void* km_1 = N2.keys.front();
                    N2.Ptrs.pop_front();
                    N2.keys.pop_front();
                    void* kn = nullptr;
                    switch(attrType) {
                    case DB_INT:
                        kn = new int;
                        memcpy(kn, k2, sizeof(int));
                        break;
                    case DB_DOUBLE:
                        kn = new double;
                        memcpy(kn, k2, sizeof(double));
                        break;
                    case DB_STRING:
                        kn = new char[attrLen];
                        strcpy((char*)kn, (char*)k2);
                        break;
                    case DB_BOOL:
                        kn = new bool;
                        memcpy(kn, k2, sizeof(bool));
                        break;
                    }
                    N.keys.push_back(kn);
                    N.Ptrs.push_back(pm);
                    P.ReplaceKey(k2, km_1);

               }else {
                    RM_Rid pm = N2.Ptrs.front();
                    void* km = N2.keys.front();
                    N2.Ptrs.pop_front();
                    N2.keys.pop_front();
                    if (N.keys.size() == N.Ptrs.size())
                        N.Ptrs.push_back(pm);
                    else {
                        auto tmp = N.Ptrs.back();
                        N.Ptrs.pop_back();
                        N.Ptrs.push_back(pm), N.Ptrs.push_back(tmp);
                    }
                    N.keys.push_back(km);
                    void *kt = nullptr;
                    switch(attrType) {
                    case DB_INT:
                        kt = new int;
                        memcpy(kt, km, sizeof(int));
                        break;
                    case DB_DOUBLE:
                        kt = new double;
                        memcpy(kt, km, sizeof(double));
                        break;
                    case DB_STRING:
                        kt = new char[attrLen];
                        strcpy((char*)kt, (char*)km);
                        break;
                    case DB_BOOL:
                        kt = new bool;
                        memcpy(kt, km, sizeof(bool));
                        break;
                    }
                    P.ReplaceKey(k2, kt);
               } 
                gBuffer->GetPage(FM_Bid(fHandler->GetFd(), P.GetBlkNum()), pHdl);
                P.SetPage(pHdl);
                gBuffer->GetPage(FM_Bid(fHandler->GetFd(), N.GetBlkNum()), pHdl);
                N.SetPage(pHdl);
                gBuffer->GetPage(FM_Bid(fHandler->GetFd(), N2.GetBlkNum()), pHdl);
                N2.SetPage(pHdl);
                return SUCCESS;
            }

        }
    }
    MM_PageHandler pHdl;
    gBuffer->GetPage(FM_Bid(fHandler->GetFd(), N.bid.num), pHdl);
    return N.SetPage(pHdl);
}

RC IM_IdxHandler::GetFirstLeaf(BTreeNode& L) {
    MM_PageHandler pHdl;
    auto fHdr = fHandler->GetFileHdr();
    if (fHdr.preF == -1) {
        std::cout<<"Idx Is Empty"<<std::endl;
        return NOT_EXIST;
    }
    gBuffer->GetPage(FM_Bid(fHandler->GetFd(), fHdr.preF), pHdl);
    L.SetData(pHdl);
    while (!L.isLeaf()) {
        //printf("arrive %d\n", L.bid.num);
        gBuffer->GetPage(FM_Bid(fHandler->GetFd(), L.Ptrs.front().num), pHdl);
        L.SetData(pHdl);
    }
    return SUCCESS;

}
RC IM_IdxHandler::GetLastLeaf(BTreeNode& L) {
    MM_PageHandler pHdl;
    auto fHdr = fHandler->GetFileHdr();
    gBuffer->GetPage(FM_Bid(fHandler->GetFd(), fHdr.preF), pHdl);
    L.SetData(pHdl);
    while (!L.isLeaf()) {
        //printf("arrive %d\n", L.bid.num);
        gBuffer->GetPage(FM_Bid(fHandler->GetFd(), L.Ptrs.back().num), pHdl);
        L.SetData(pHdl);
    }
    return SUCCESS;
}

RC IM_IdxHandler::Traverse() {
    BTreeNode L(attrType, attrLen);
    int s = GetFirstLeaf(L);
    if (s != SUCCESS)
        return s;
    //printf("First Leaf %d-----%d %d\n", L.bid.num, L.Ptrs.size(), L.keys.size());
    //return SUCCESS;
    MM_PageHandler pHdl;
    int i = 0;
    while (true) {
        
        printf("leaf %d BlockNum %d---------\n", i, L.bid.num);
        i++;
        //std::cout<<L.Ptrs.size()<<" "<<L.keys.size()<<std::endl;
        //std::cout<<L.Ptrs.size()<<" "<<L.keys.size()<<std::endl;
        auto j = L.Ptrs.begin();
        for (auto key: L.keys) {
            printf("key %s ----- ptr %d %d\n", key, (*j).num, (*j).slot);
            j++;
        }
        
        if (L.keys.size() == L.Ptrs.size()) {
            break;
        }
        //printf("next leaf %d\n", L.Ptrs.back().num);
        if (gBuffer->GetPage(FM_Bid(fHandler->GetFd(), L.Ptrs.back().num), pHdl) != SUCCESS) {
            return FAILURE;
        }
        L.SetData(pHdl);
    }
    return SUCCESS;
}

RC IM_IdxHandler::UpdateEntry(void *pData, const RM_Rid &rid, void*nData, const RM_Rid & nRid) {
    int st = 0;
    if (st = DeleteEntry(pData, rid) != SUCCESS) {
        return st;
    }
    return InsertEntry(nData, nRid);
}

RC IM_IdxHandler::GetQueryLeaf(void* pData, BTreeNode& L) {
    FM_FileHdr fHdr = fHandler->GetFileHdr();
    MM_PageHandler pHdl;
    gBuffer->GetPage(FM_Bid(fHandler->GetFd(), fHdr.preF), pHdl);
    L.SetData(pHdl);
    RM_Rid nex = L.GetFirstSon(pData);
    //std::cout<<nex.num<<std::endl;
    /*
    printf("head %d\n", L.bid.num);
    printf("total blk num %d\n", fHdr.blkCnt);
    for (auto i : L.keys) {
        printf("root %s\n", i);
    }
    */
    int pNum = L.bid.num;
    while (nex.num != -1)
    {
        gBuffer->GetPage(FM_Bid(fHandler->GetFd(), nex.num), pHdl);
        L.SetData(pHdl);
        L.pHdr.preFreePage = pNum;
        L.SetPage(pHdl);
        pNum = L.bid.num;
        nex = L.GetFirstSon(pData);
    }
    // std::cout<<"ggg"<<std::endl;
    return SUCCESS;
}

RC IM_IdxHandler::GetNextLeaf(BTreeNode& L) {
    if (L.GetKeyNum() == L.GetPtrNum())
        return SUCCESS;
    FM_FileHdr fHdr = fHandler->GetFileHdr();
    MM_PageHandler pHdl;
    gBuffer->GetPage(FM_Bid(fHandler->GetFd(), L.Ptrs.back().num), pHdl);
    return L.SetData(pHdl);
}

RC IM_IdxHandler::GetIter(IM_IdxIterator& iter) {
    iter.SetTypeLen(attrType, attrLen);
    iter.SetIdxHandler(*this);
    iter.Reset();
    return SUCCESS;
}

