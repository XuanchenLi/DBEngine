#include <unistd.h>
#include <iostream>
#include "FM/FM_FileHandler.h"
#include "FM/FM_Manager.h"
#include "FM/FM_Bid.h"
#include "MM_Buffer.h"
#include "MM_BufferUnit.h"
#include "MM/MM_PageHandler.h"
#include "main.h"

extern unsigned int BUFFER_SIZE;
extern FM_Manager* fM_Manager;

MM_Buffer::MM_Buffer(unsigned int s, Strategy str) {
    this->strategy = str;
    this->hashTbl.clear();
    this->victimList.clear();
    this->freeList.clear();
    this->units = new MM_BufferUnit[s];
    for (int i = 0; i < s; ++i) {
        this->freeList.push_back(i);
    }
    
}

MM_Buffer::~MM_Buffer() {
    delete[] this->units;
}
RC MM_Buffer::Clear() {
    for (int i = 0; i < BUFFER_SIZE; ++i) {
        //std::cout<<i<<std::endl;
        if (units[i].dirty) {
            FM_FileHandler* hdl = nullptr;
            if (fM_Manager->GetFileHandle(units[i].bid.fd, &hdl) == SUCCESS) {
                //std::cout<<"1231"<<std::endl;
                int st = hdl->WriteBlock(units[i].bid.num, units[i].content);
                //std::cout<<st<<std::endl;
            }else {
                return FAILURE;
            }
        }
        units[i].clear();
        this->freeList.push_back(i);
    }
    hashTbl.clear();
    victimList.clear();
    //std::cout<<"121"<<std::endl;
    return SUCCESS;
}

RC MM_Buffer::Clear(int fd) {
    for (int i = 0; i < BUFFER_SIZE; ++i) {
        //std::cout<<i<<std::endl;
        if (units[i].bid.fd == fd) {
            if (units[i].dirty) {
               FM_FileHandler* hdl = nullptr;
               if (fM_Manager->GetFileHandle(units[i].bid.fd, &hdl) == SUCCESS) {
                   //std::cout<<"1231"<<std::endl;
                   int st = hdl->WriteBlock(units[i].bid.num, units[i].content);
                   //std::cout<<st<<std::endl;
               }else {
                   return FAILURE;
               }
            }
            units[i].clear();
            this->freeList.push_back(i);
        }
    }
    auto it = hashTbl.begin();
    while(it != hashTbl.end()){
      if(it->first.fd == fd) {
        victimList.remove(it->second);
        hashTbl.erase(it++);
      }
      else
        it++;
    }
    //std::cout<<"121"<<std::endl;
    return SUCCESS;
}

RC MM_Buffer::ForcePage(FM_Bid bid) {
    auto pr = hashTbl.find(bid);
    if (pr == hashTbl.end())
        return NOT_EXIST;
    int idx = pr->second;
    if (!units[idx].dirty) 
        return SUCCESS;
    FM_FileHandler* hdl = nullptr;
    if (fM_Manager->GetFileHandle(units[idx].bid.fd, &hdl) == SUCCESS) {
        return hdl->WriteBlock(units[idx].bid.num, units[idx].content);
    }
    return FAILURE;
}

RC MM_Buffer::ForcePage(int fd) {
    for (auto p:hashTbl) {
        if (p.first.fd == fd) {
            int idx = p.second;
            if (!units[idx].dirty) 
                continue;
            FM_FileHandler* hdl = nullptr;
            if (fM_Manager->GetFileHandle(units[idx].bid.fd, &hdl) == SUCCESS) {
                hdl->WriteBlock(units[idx].bid.num, units[idx].content);
            }
        }
    }
    return SUCCESS;
}

RC MM_Buffer::Pin(FM_Bid bid) {
    auto pr = hashTbl.find(bid);
    if (pr == hashTbl.end())
        return NOT_EXIST;
    int idx = pr->second;
    if (units[idx].refCount == 0) {
        victimList.remove(idx);
    }
    units[idx].refCount++;
    units[idx].usedCount++;
    return SUCCESS;
}

RC MM_Buffer::Unpin(FM_Bid bid) {
    auto pr = hashTbl.find(bid);
    if (pr == hashTbl.end())
        return NOT_EXIST;
    int idx = pr->second;
    if (units[idx].refCount <= 0) return FAILURE;
    units[idx].refCount--;
    if (units[idx].refCount == 0) {
        victimList.push_back(idx);
    }
    return SUCCESS;
}

RC MM_Buffer::SetDirty(FM_Bid bid) {
    auto pr = hashTbl.find(bid);
    if (pr == hashTbl.end())
        return NOT_EXIST;
    int idx = pr->second;
    //std::cout<<idx<<std::endl;
    units[idx].dirty = true;
    return SUCCESS;
}

RC MM_Buffer::GetPage(FM_Bid bid, MM_PageHandler& hdl) {
    auto pr = hashTbl.find(bid);
    if (pr != hashTbl.end()) {
        //Pin(bid);
        int idx = pr->second;
        hdl.SetPage(&units[idx]);
        return SUCCESS;
    }

    int slot = GetEmpty();
    if (slot == FAILURE) {
        std::cout<<"!!!BUFFER USED OUT!!!"<<std::endl;
        return FAILURE;
    }
    FM_FileHandler* fHdl = nullptr;
    if (fM_Manager->GetFileHandle(bid.fd, &fHdl) != SUCCESS) {
        return NOT_EXIST;
    }
    //std::cout<<fHdl<<std::endl;
    
    if (fHdl->GetBlock(bid.num, units[slot].content) != SUCCESS) {
        return FAILURE;
    }
    
    units[slot].bid = bid;
    units[slot].dirty = false;
    hashTbl.insert(std::make_pair(bid, slot));
    //Pin(bid);
    hdl.SetPage(&units[slot]);
    return SUCCESS;
}
int MM_Buffer::GetEmpty() {
    if (!this->freeList.empty()) {
        int res = freeList.front();
        freeList.pop_front();
        return res;
    }
    if (!victimList.empty()) {
        int res = 0;
        if (this->strategy == LRU) {
            res = victimList.front();
            victimList.pop_front();
        }else if (this->strategy == CLOCK_SWEEP) {
            auto it = this->victimList.begin();
            while(true) {
                if (this->units[*it].usedCount == 0) {
                    res = *it;
                    this->victimList.remove(*it);
                    break;
                } else {
                   this->units[*it].usedCount--; 
                   it++;
                   if (it == this->victimList.end())
                        it = this->victimList.begin();
                }
            }   
        }
        ForcePage(units[res].bid);
        hashTbl.erase(units[res].bid);
        units[res].clear();
        return res;
    }
    return FAILURE;
}