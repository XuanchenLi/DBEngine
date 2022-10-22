#include "BTreeNode.h"
#include "MM/MM_PageHandler.h"
#include "MM/MM_PageHeader.h"
#include "RM/RM_RecHeader.h"
#include <iostream>
#include <string>
#include <string.h>

RC BTreeNode::InitHead() {
    pHdr.firstHole = 0;
    pHdr.slotCnt = BLOCK_SIZE - sizeof(MM_PageHdr) - sizeof(RM_Rid);
    pHdr.slotCnt /= sizeof(RM_RecHeader) + sizeof(RM_Rid) + attrLen;
    pHdr.nextFreePage = -1;
    return SUCCESS;
}

 BTreeNode::BTreeNode(dbType attrT, int attrL) {
    attrType = attrT;
    attrLen = attrL;
    isChanged = false;
    InitHead();
 }
 BTreeNode::~BTreeNode() {
    for (auto i = keys.begin(); i != keys.end(); ++i) {
        switch(attrType) {
            case DB_INT:
                delete (int*)*i;
                break;
            case DB_DOUBLE:
                delete (double*)*i;
                break;
            case DB_STRING:
                delete[] (char*)*i;
                break;
            case DB_BOOL:
                delete (bool*)*i;
                break;
        }
    }
 }

 BTreeNode& BTreeNode::operator=(const BTreeNode& oth) {
    if (&oth == this)
        return *this;
    for (auto i = keys.begin(); i != keys.end(); ++i) {
        switch(attrType) {
            case DB_INT:
                delete (int*)*i;
                break;
            case DB_DOUBLE:
                delete (double*)*i;
                break;
            case DB_STRING:
                delete[] (char*)*i;
                break;
            case DB_BOOL:
                delete (bool*)*i;
                break;
        }
    }
    keys.clear();
    Ptrs.clear();
    isChanged = oth.isChanged;
    attrType = oth.attrType;
    attrLen = oth.attrLen;
    bid = oth.bid;
    pHdr = oth.pHdr;
    Ptrs = oth.Ptrs;
    for (auto i = oth.keys.begin(); i != oth.keys.end(); ++i) {
        switch(attrType) {
            case DB_INT:
                keys.push_back(new int(*(int*)*i));
                break;
            case DB_DOUBLE:
                keys.push_back(new double(*(double*)*i));
                break;
            case DB_STRING:
            {
                char* ptr = new char[attrLen];
                strcpy(ptr, (char*)*i);
                keys.push_back(ptr);
                break;
            }
            case DB_BOOL:
                keys.push_back(new bool(*(bool*)*i));
                break;
        }
    }
    return *this;
 }

 BTreeNode::BTreeNode(const BTreeNode& oth) {
    keys.clear();
    Ptrs.clear();
    attrType = oth.attrType;
    attrLen = oth.attrLen;
    bid = oth.bid;
    pHdr = oth.pHdr;
    Ptrs = oth.Ptrs;
    isChanged = oth.isChanged;
    for (auto i = oth.keys.begin(); i != oth.keys.end(); ++i) {
        switch(attrType) {
            case DB_INT:
                keys.push_back(new int(*(int*)*i));
                break;
            case DB_DOUBLE:
                keys.push_back(new double(*(double*)*i));
                break;
            case DB_STRING:
            {
                char* ptr = new char[attrLen];
                strcpy(ptr, (char*)*i);
                keys.push_back(ptr);
                break;
            }
            case DB_BOOL:
                keys.push_back(new bool(*(bool*)*i));
                break;
        }
    }
 }
int BTreeNode::GetStartOff() const {
    return sizeof(MM_PageHdr) + pHdr.slotCnt * sizeof(RM_RecHdr);
}
RC BTreeNode::SetData(const MM_PageHandler& pHdl) {
    bid = pHdl.GetBid();
    pHdr = pHdl.GetHeader();
    if (pHdr.firstHole == -1) {
        InitHead();
        isChanged = true;
        return SUCCESS;
    }
    if (pHdr.firstHole == 0)
        return SUCCESS;
    auto ptr = pHdl.GetPtr(GetStartOff());
    RM_Rid rid;
    memcpy(&rid, ptr, sizeof(RM_Rid));
    Ptrs.push_back(rid);
    ptr += sizeof(RM_Rid);
    void* tp;
    for (int i = 0; i < pHdr.firstHole; ++i) {
        switch(attrType) {
            case DB_INT:
                tp = new int;
                memcpy(tp, ptr, sizeof(int));
                keys.push_back(tp);
                break;
            case DB_DOUBLE:
                tp = new double;
                memcpy(tp, ptr, sizeof(double));
                keys.push_back(tp);
                break;
            case DB_STRING:
                tp = new char[attrLen];
                strcpy((char*)tp, (char*)ptr);
                keys.push_back(tp);
                break;
            case DB_BOOL:
                tp = new bool;
                memcpy(tp, ptr, sizeof(bool));
                keys.push_back(tp);
                break;
        }
        ptr += attrLen;
        memcpy(&rid, ptr, sizeof(RM_Rid));
        Ptrs.push_back(rid);
        ptr += sizeof(RM_Rid);
    }
    return SUCCESS;
}


RC BTreeNode::SetPage(MM_PageHandler& pHdl) {
    // 假设pHdl已经指向一个缓冲区
    if (pHdl.GetPtr(0) == nullptr)
        return INVALID_OPTR;
    pHdl.SetHeader(pHdr);
    if (isChanged) {
        isChanged = false;
    }
    pHdl.SetDirty();
    if (pHdr.firstHole == 0)
        return SUCCESS;
    int preLen = GetStartOff();
    auto ptr1 = pHdl.GetPtr(sizeof(MM_PageHdr));
    auto ptr2 = pHdl.GetPtr(preLen);

    memcpy(ptr2, &Ptrs.front(), sizeof(RM_Rid));
    ptr2 += sizeof(RM_Rid);
    RM_RecHdr rHdr;
    rHdr.isDeleted = false;
    rHdr.len = sizeof(RM_Rid) + attrLen;
    rHdr.off = preLen + sizeof(RM_Rid);
    auto ip = keys.begin();
    auto jp = Ptrs.begin(); jp ++;
    for (int i = 0; i < keys.size(); ++i) {
        memcpy(ptr1, &rHdr, sizeof(RM_RecHdr));
        ptr1 += sizeof(RM_RecHdr);
        rHdr.off += rHdr.len;

        memcpy(ptr2, *ip, attrLen);
        ptr2 += attrLen;
        memcpy(ptr2, &(*jp), sizeof(RM_Rid));
        ptr2 += sizeof(RM_Rid);
    }
    return SUCCESS;

}

RM_Rid BTreeNode::GetSon(void* pData) {
    RM_Rid res;   
    if (pHdr.nextFreePage == -1)
        return res;
    auto j = Ptrs.begin();
    for (auto i = keys.begin(); i != keys.end(); ++i, j++) {
        if (Greater(*i, pData)) {
            res = *j;
            return res;
        }
    }
    return res;
}

bool BTreeNode::Less(const void*p1, const void*p2) const{
    switch (attrType)
        {
        case DB_INT:
            if (*(int*)p1 < *(int*)p2) {
                return true;
            }           
            break;
        case DB_BOOL:
            if (*(bool*)p1 < *(bool*)p2) {
                return true;
            }   
            break;
        case DB_DOUBLE:
            if (*(double*)p1 < *(double*)p2) {
                return true;
            }   
            break;
        case DB_STRING:
            if (strcmp((char*)p1, (char*)p2) < 0) {
                return true;
            }   
            break;
        default:
            std::cout<<"Unknown type\n";
            break;
        }
    return false;
}

bool BTreeNode::Greater(const void*p1, const void*p2) const{
    switch (attrType)
        {
        case DB_INT:
            if (*(int*)p1 > *(int*)p2) {
                return true;
            }           
            break;
        case DB_BOOL:
            if (*(bool*)p1 > *(bool*)p2) {
                return true;
            }   
            break;
        case DB_DOUBLE:
            if (*(double*)p1 > *(double*)p2) {
                return true;
            }   
            break;
        case DB_STRING:
            if (strcmp((char*)p1, (char*)p2) > 0) {
                return true;
            }   
            break;
        default:
            std::cout<<"Unknown type\n";
            break;
        }
    return false;
}


RC BTreeNode::InsertPair(void* key, const RM_Rid& ptr) {
    pHdr.firstHole ++;
    isChanged = true;
    if (Less(key, keys.front())) {
        keys.push_front(key);
        Ptrs.push_front(ptr);
        return SUCCESS;
    }else {
        auto pp = Ptrs.begin();
        for (auto iter = keys.begin(); iter!=keys.end(); ++iter, ++pp) {
            if (Greater(*iter, key)) {
                keys.insert(iter, key);
                Ptrs.insert(pp, ptr);
                return SUCCESS;
            }
        }
        keys.push_back(key);
        Ptrs.push_back(ptr);
        return SUCCESS;
    }
    return SUCCESS;
}

RC BTreeNode::EraseNPair(int n) {
    if (n == 0) return SUCCESS;
    if (n > keys.size()) {
        std::cout<<"too many pair to delete\n";
        return FAILURE;
    }
    for (int i = 0; i < n; ++i) {
        Ptrs.pop_front();
        switch(attrType) {
            case DB_INT:
                delete (int*)keys.front();
                keys.pop_front();
                break;
            case DB_DOUBLE:
                delete (double*)keys.front();
                keys.pop_front();
                break;
            case DB_STRING:
                delete[] (char*)keys.front();
                keys.pop_front();
                break;
            case DB_BOOL:
                delete (bool*)keys.front();
                keys.pop_front();
                break;
        }
    }
    pHdr.firstHole-=n;
    isChanged = true;
    return SUCCESS;
}