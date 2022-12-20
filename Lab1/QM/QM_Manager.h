#ifndef QM_MANAGER_H
#define QM_MANAGER_H

#include <vector>
#include "utils/RC.h"
#include "utils/DB_Option.h"


typedef struct MRelAttr {
    MRelAttr() {}
    MRelAttr(std::string& t, std::string& c) {
        tblName = t;
        colName = c;
    }
    std::string tblName;
    std::string colName;
public:
    bool operator<(const MRelAttr& rhs) const {
        if (tblName == rhs.tblName)
            return tblName < rhs.tblName;
        return colName < rhs.colName;
    }
    bool operator==(const MRelAttr& rhs) const {
        return tblName == rhs.tblName && colName == rhs.colName;
    }
}MRelAttr;

typedef struct RelNumAttr {
    std::string tblName;
    int colPos;
    dbType type;
    int len;
    union {
        char sData[64];
        int iData;
        double lfData;
        bool bData;
    }data;
}RelNumAttr;



class QM_Manager {
    RC Select();
    RC Select(std::vector<MRelAttr>& selAttrs, std::vector<std::string>& relations, std::vector<DB_Cond>& conditions);
    bool ValidAttr(const std::vector<MRelAttr>&);
    bool ValidRel(const std::vector<std::string>&);
};

#endif