#ifndef QM_COMMONGENERATOR_H
#define QM_COMMONGENERATOR_H
#include "QM_PlanGenerator.h"

class QM_CommonGenerator : public QM_PlanGenerator {
public:
    DB_Iterator* generate(const std::vector<MRelAttr>& selAttrs, 
                        const std::vector<std::string>& relations, 
                        const std::vector<DB_Cond>& conditions);
private:
    std::vector<DB_Iterator*> generateLeaf(const std::vector<MRelAttr>& selAttrs, 
                        const std::vector<std::string>& relations, 
                        const std::vector<DB_Cond>& conditions);
    DB_Iterator* generateOne(
        const std::string& relName,
        const std::vector<std::string>& selAttrs,
        const std::vector<DB_Cond>& conditions
    );
};


#endif