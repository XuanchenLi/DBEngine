#ifndef MRELATTR_H
#define MRELATTR_H
#include <string>

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

#endif