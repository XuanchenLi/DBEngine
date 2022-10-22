#ifndef IM_MANAGER_H
#define IM_MANAGER_H

#include "utils/RC.h"
#include "IM_IdxHandler.h"


class IM_Manager {
public:
    RC CreateIndex(const char* tblPath, int colPos);
	RC DestroyIndex(const char* tblPath, int colPos);
	RC FindIndex(const char* tblPath, int colPos) const;
	RC OpenIndex(const char* tblPath, int colPos,  IM_IdxHandler& handler);
	RC CloseIndex(IM_IdxHandler& handler);

private:

};



#endif