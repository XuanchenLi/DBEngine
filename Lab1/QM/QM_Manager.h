#ifndef QM_MANAGER_H
#define QM_MANAGER_H

#include <vector>
#include "utils/RC.h"
#include "utils/DB_Option.h"
#include "QM_PlanGenerator.h"
#include "QM_CommonGenerator.h"
#include "MRelAttr.h"


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
public:
    //RC Select();
    QM_Manager(QM_PlanGenerator* g = new QM_CommonGenerator()) {generator = g;}
    ~QM_Manager() {delete generator;}
    QM_Manager(const QM_Manager &)=delete;
    QM_Manager& operator= (const QM_Manager &)=delete;
    RC Select(std::vector<MRelAttr>& selAttrs, std::vector<std::string>& relations, std::vector<DB_Cond>& conditions);
    //bool ValidAttr(const std::vector<MRelAttr>&);
    //bool ValidRel(const std::vector<std::string>&);
private:
    QM_PlanGenerator* generator;
};

#endif