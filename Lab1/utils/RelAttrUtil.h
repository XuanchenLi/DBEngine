#ifndef RELATTRUTIL_H
#define RELATTRUTIL_H
#include <vector>
#include "QM/QM_Manager.h"

bool ValidAttr(const std::vector<MRelAttr>& attrs);
bool ValidRel(const std::vector<std::string>& rels);
RC ExpandAttrs(std::vector<MRelAttr>& attrs, const std::vector<std::string>& rels);

#endif