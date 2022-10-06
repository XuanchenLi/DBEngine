#ifndef RM_RECAUX_H
#define RM_RECAUX_H

#include <vector>
#include <string>
#include "utils/RC.h"

template<class vertex_t, class value_t>
class isEqualALL {
public:
    explicit isEqualALL(vertex_t node) : node(node) {}
    bool operator() (const std::pair<vertex_t, value_t>& element) const {
        return element.first == node;
    }
private:
    const vertex_t node;
};

typedef struct RM_RecAux {
    std::vector<std::pair<std::string, std::string> > strValue;
    std::vector<std::pair<std::string, double> > lfValue;
    std::vector<std::pair<std::string, int> > iValue;
    std::vector<std::pair<std::string, bool> > bValue;

    std::pair<int, int> GetPosByKey(const std::string& key) const {
       for (int i = 0; i < strValue.size(); ++i) {
        if (strValue[i].first == key) {
            return std::make_pair(0, i);
        }
       } 
       for (int i = 0; i < lfValue.size(); ++i) {
        if (lfValue[i].first == key) {
            return std::make_pair(1, i);
        }
       }
       for (int i = 0; i < iValue.size(); ++i) {
        if (iValue[i].first == key) {
            return std::make_pair(2, i);
        }
       }
       for (int i = 0; i < bValue.size(); ++i) {
        if (bValue[i].first == key) {
            return std::make_pair(3, i);
        }
       }
       return std::make_pair(NOT_EXIST, 0);
    }

    void Clear() {
        strValue.clear();
        lfValue.clear();
        iValue.clear();
        bValue.clear();
    }
}RM_RecAux;

#endif