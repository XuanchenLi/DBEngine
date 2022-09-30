#ifndef FM_MANAGER_H
#define FM_MANAGER_H

#include <unistd.h>
#include <unordered_map>
#include "utils/RC.h"

class FM_FileHandler;

class FM_Manager {
public:
    ~FM_Manager();
    RC PreallocBlock(const char* dirPath, unsigned int size);
    //RC AllocBlock(const char* destFilePath, unsigned int bNum);
    RC CreateTblFile(const char* filePath);
    RC CreateEmptyFile(const char* filePath);
    RC CreateDir(const char* dirPath);
    RC DeleteFile(const char* filePath);
    RC DeleteDir(const char* dirPath);

    RC OpenFile(const char* filePath, FM_FileHandler&);
    RC CloseFile(FM_FileHandler&);

    RC GetFileHandle(int fd, FM_FileHandler**);

private:
    std::unordered_map<int, FM_FileHandler*> fHandleTbl;
    
};

#endif