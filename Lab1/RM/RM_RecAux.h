#ifndef RM_RECAUX_H
#define RM_RECAUX_H

#include <vector>
#include <string>

typedef struct RM_RecAux {
    std::vector<std::string> strValue;
    std::vector<double> lfValue;
    std::vector<int> iValue;
}RM_RecAux;

#endif