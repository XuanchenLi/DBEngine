#ifndef STRINGUTIL_H
#define STRINGUTIL_H

#include <string>
#include <utility>
#include <tuple>

std::pair<std::string, std::string> GetDbTblName(const std::string& str);
std::tuple<std::string, std::string, int> GetIdxName(const std::string& str);



#endif