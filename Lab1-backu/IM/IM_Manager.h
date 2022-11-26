#ifndef IM_MANAGER_H
#define IM_MANAGER_H

#include <vector>
#include <string>
#include "utils/RC.h"
#include "IM_IdxHandler.h"


class IM_Manager {
public:
    RC CreateIndex(const char* tblPath, int colPos);
	RC DestroyIndex(const char* tblPath, int colPos);
	RC GetAvailIndex(std::string tblName, std::vector<int>& colPoses);
	RC FindIndex(const char* tblPath, int colPos) const;
	RC OpenIndex(const char* tblPath, int colPos,  IM_IdxHandler& handler);
	RC CloseIndex(IM_IdxHandler& handler);
	RC ClearIndex(const char* tblPath, int colPos);
	RC TraverseLeaf(const char* tblPath, int colPos);
private:

};



#endif