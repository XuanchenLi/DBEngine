#ifndef STRINGUTIL_H
#define STRINGUTIL_H

#include <string>
#include <utility>
#include <tuple>

std::pair<std::string, std::string> GetDbTblName(const std::string& str);
std::tuple<std::string, std::string, int> GetIdxName(const std::string& str);
std::string GetTblPathFromIdxPath(const std::string& p);
int endswith(std::string s, std::string sub);
int startswith(std::string s, std::string sub);

#endif