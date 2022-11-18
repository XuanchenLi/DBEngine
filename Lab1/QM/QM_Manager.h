#ifndef QM_MANAGER_H
#define QM_MANAGER_H

#include <vector>
#include "utils/RC.h"
#include "utils/DB_Option.h"

typedef struct RelAttr {
    std::string tblName;
    std::string colName;
}RelAttr;



class QM_Manager {
    RC Select(std::vector<RelAttr> selAttrs, std::vector<std::string> relations, std::vector<DB_Cond> conditions);
};

#endif