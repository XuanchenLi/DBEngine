#ifndef QM_PLANGENERATOR_H
#define QM_PLANGENERATOR_H
#include "utils/RC.h"
#include "MRelAttr.h"
#include "utils/DB_Option.h"
#include "Common/DB_Iterator.h"
#include <vector>


class QM_PlanGenerator {
public:
    virtual DB_Iterator* generate(const std::vector<MRelAttr>& selAttrs, 
                        const std::vector<std::string>& relations, 
                        const std::vector<DB_Cond>& conditions) = 0;

protected:
    virtual std::vector<DB_Iterator*> generateLeaf(const std::vector<MRelAttr>& selAttrs, 
                        const std::vector<std::string>& relations, 
                        const std::vector<DB_Cond>& conditions) = 0;
    virtual DB_Iterator* generateOne(
        const std::string& relName,
        const std::vector<std::string>& selAttrs,
        const std::vector<DB_Cond>& conds
    ) = 0;
};

#endif