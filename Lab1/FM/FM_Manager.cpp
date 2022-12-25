#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include "FM/FM_Manager.h"
#include "utils/RC.h"
#include "FM_FileHeader.h"
#include "FM_FileHandler.h"
#include "MM/MM_Buffer.h"
#include "main.h"

extern unsigned int BLOCK_SIZE;
extern unsigned int PREALLOC_SIZE;
extern MM_Buffer* gBuffer;

bool isPathExist(const char* path) {
    if (access(path, 0) == F_OK) {
        return true;
    }
    return false;
}

bool isFile(const char* path) {
    if (!isPathExist(path)) {
        return false;
    }
    struct stat buffer;
    return (stat(path, &buffer) == 0 && S_ISREG(buffer.st_mode));
}
bool isDir(const char* path) {
    if (!isPathExist(path)) {
        return false;
    }
    struct stat buffer;
    return (stat(path, &buffer) == 0 && S_ISDIR(buffer.st_mode));
}

FM_Manager::~FM_Manager() {
    //std::cout<<fHandleTbl.size()<<std::endl;
    for(auto kv:fHandleTbl){
        if (kv.second->IsOpen()) {
            //std::cout<<"!23"<<std::endl;
            CloseFile(*(kv.second));
        }
        delete kv.second;
    }
}


RC FM_Manager::CreateDir(const char * dirPath) {
    if (isPathExist(dirPath))
        return FILE_EXIST;
    
    if (!mkdir(dirPath, 777))
        return SUCCESS;
    return FAILURE;
}

RC FM_Manager::PreallocBlock(const char* filePath, unsigned int size) {
    
    if (isPathExist(filePath))
        return FILE_EXIST;
    int fd = open(filePath, O_WRONLY | O_CREAT, 777);
    if (fd < 0) {
        return FAILURE;
    }
    if(fallocate(fd, 0, 0, size)) {
        close(fd);
        remove(filePath);
        return FAILURE;
    }
    close(fd);
    return SUCCESS;

}

RC FM_Manager::DeleteFile(const char* filePath) {
    if (!isPathExist(filePath))
        return NOT_EXIST;
    if (!isFile(filePath))
        return TYPE_ERROR;
    if (!remove(filePath))
        return SUCCESS;
    return FAILURE;
}

RC FM_Manager::DeleteDir(const char* dirPath) {
    char dir_name[512];
    DIR *dirp;
    struct dirent *dp;
    struct stat dir_stat;

    // 参数传递进来的目录不存在，直接返回
    if ( 0 != access(dirPath, F_OK) ) {
        return NOT_EXIST;
    }

    // 获取目录属性失败，返回错误
    if ( 0 > stat(dirPath, &dir_stat) ) {
        return FAILURE;
    }

    if ( S_ISREG(dir_stat.st_mode) ) {  // 普通文件直接删除
        remove(dirPath);
    } else if ( S_ISDIR(dir_stat.st_mode) ) {   // 目录文件，递归删除目录中内容
        dirp = opendir(dirPath);
        while ( (dp=readdir(dirp)) != NULL ) {
            // 忽略 . 和 ..
            if ( (0 == strcmp(".", dp->d_name)) || (0 == strcmp("..", dp->d_name)) ) {
                continue;
            }
            sprintf(dir_name, "%s/%s", dirPath, dp->d_name);
            DeleteDir(dir_name);   // 递归调用
        }
        closedir(dirp);

        rmdir(dirPath);     // 删除空目录
    } else {
        return FAILURE;    
    }

    return SUCCESS;
}


RC FM_Manager::CreateTblFile(const char* filePath) {
    int status = 0;
    if (status = PreallocBlock(filePath, PREALLOC_SIZE))
        return status;
    int fd = open(filePath, O_RDWR);
    if (fd == -1)
        return FAILURE;
    FM_FileHeader fHdr;
    //fHdr.maxBlkNum = (PREALLOC_SIZE<<10) / BLOCK_SIZE;
    lseek(fd, 0, SEEK_SET);
    int res = write(fd, (char*)&fHdr, sizeof(FM_FileHdr));

    close(fd);
    if (res != sizeof(FM_FileHdr))
        return IO_ERROR;
    return SUCCESS;
    
}

RC FM_Manager::CreateEmptyFile(const char* filePath) {
    int fd = open(filePath, O_RDWR | O_CREAT, 777);
    if (fd == -1)
        return FAILURE;
    close(fd);
    return SUCCESS;
}

RC FM_Manager::OpenFile(const char* filePath, FM_FileHandler& fHandle) {
    //std::cout<<"!23"<<std::endl;
    if (!isPathExist(filePath))
        return NOT_EXIST;
    if (!isFile(filePath))
        return TYPE_ERROR;
    int fd = open(filePath, O_RDWR);
    if (fd < 0)
        return FAILURE;
    FM_FileHdr fHdr;
    lseek(fd, 0, SEEK_SET);
    int n_read = read(fd, &fHdr, sizeof(FM_FileHdr));
    if (n_read != sizeof(FM_FileHdr))
        return IO_ERROR;
    fHandle.SetFileHdr(fHdr);
    //std::cout<<fHdr.firstFreeHole<<std::endl;
    fHandle.SetOpen(true);
    fHandle.SetChanged(false);
    fHandle.SetFd(fd);
    this->fHandleTbl.insert(std::make_pair(fd, &fHandle));
    //std::cout<<fHandleTbl.size()<<std::endl;
    return  SUCCESS;
}

RC FM_Manager::CloseFile(FM_FileHandler& fHandle) {
    //std::cout<<"!23"<<std::endl;
    if (!fHandle.IsOpen())
        return SUCCESS;
    if (fHandle.IsChanged()) {
        lseek(fHandle.GetFd(), 0, SEEK_SET);
        FM_FileHdr tHdr = fHandle.GetFileHdr();
        //std::cout<<tHdr.blkCnt<<std::endl;
        int res = write(fHandle.GetFd(), &tHdr, sizeof(FM_FileHdr));
        fHandle.SetChanged(false);
    }
    int fd = fHandle.GetFd();
    //gBuffer->ForcePage(fd);
    gBuffer->Clear(fd);
    close(fd);
    fHandle.SetFd(-1);
    fHandle.SetOpen(false);
    this->fHandleTbl.erase(fd);
    
    return SUCCESS;
}

RC FM_Manager::GetFileHandle(int fd, FM_FileHandler** hdl) {
    auto handle = fHandleTbl.find(fd);
    if (handle == fHandleTbl.end())
        return NOT_EXIST;
    //std::cout<<handle->second<<std::endl;
    *hdl = handle->second;
    return SUCCESS;
}


