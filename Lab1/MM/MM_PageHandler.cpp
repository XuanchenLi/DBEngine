#include <fcntl.h>
#include <iostream>
#include "MM_PageHandler.h"
#include "MM_BufferUnit.h"
#include "MM_Buffer.h"
#include "main.h"

extern MM_Buffer* gBuffer;

MM_PageHandler::~MM_PageHandler() {
        if (buf != nullptr) {
            Unpin();
        }
    }

MM_PageHandler::MM_PageHandler(const MM_PageHandler & rhs) {
    buf = rhs.buf;
    pHdr = rhs.pHdr;
    if (buf != nullptr) Pin();
}

MM_PageHandler& MM_PageHandler::operator= (const MM_PageHandler & rhs) {
        if (this == &rhs)
            return *this;
        if (buf != nullptr) Unpin();
        buf = rhs.buf;
        pHdr = rhs.pHdr;
        Pin();
        return *this;
    }


RC MM_PageHandler::SetPage(MM_BufferUnit* unit) {
    if (buf != nullptr)
        Unpin();
    buf = unit;
    pHdr = *((MM_PageHdr*) buf->GetPtr());
    //memcpy(&pHdr, buf->GetPtr(), sizeof(MM_PageHdr));
    //std::cout<<"casac"<<pHdr.freeBtsCnt<<std::endl;
    this->Pin();
    return SUCCESS;
}

RC MM_PageHandler::Pin() {
    return gBuffer->Pin(buf->GetBid());
}

RC MM_PageHandler::Unpin() {
    return gBuffer->Unpin(buf->GetBid());
}

RC MM_PageHandler::Force() {
    return gBuffer->ForcePage(buf->GetBid());
}

RC MM_PageHandler::SetDirty() {
    //std::cout<<buf->GetBid().num<<std::endl;
    return gBuffer->SetDirty(buf->GetBid());
}

RC MM_PageHandler::SetHeader(const MM_PageHdr& nHdr) {
    this->pHdr = nHdr;
    char* ptr = this->buf->GetPtr();
    memcpy(ptr, &nHdr, sizeof(MM_PageHdr));
    return SUCCESS;
}

char* MM_PageHandler::GetPtr(int off) {
    if (buf == nullptr)
        return nullptr;
    return buf->GetPtr() + off;
}