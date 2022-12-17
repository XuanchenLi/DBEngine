
#include <string>
#include <utility>
#include <tuple>
#include "stringUtil.h"

std::pair<std::string, std::string> GetDbTblName(const std::string& str) {
    std::string tmp = str;
    std::pair<std::string, std::string> res;
    if (tmp[tmp.length() - 1] == '/') {
        tmp = tmp.substr(0, tmp.length() - 1);
    }  
    //std::cout<<tmp<<std::endl;
    int idx = tmp.find_last_of('/');
    if (idx != std::string::npos) {
       res.second = tmp.substr(idx + 1);
        tmp = tmp.substr(0, idx);
    }
    idx = tmp.find_last_of('/');
    res.first = tmp;
    if (idx != std::string::npos) {
        res.first = tmp.substr(idx + 1);
    }
    return res;
}

std::tuple<std::string, std::string, int> GetIdxName(const std::string& str) {
    int colPos;
    std::string dbName, tblName;
    std::string tmp = str;
    if (tmp[tmp.length() - 1] == '/') {
        tmp = tmp.substr(0, tmp.length() - 1);
    }
    int idx = tmp.find_last_of('_');
    if (idx != std::string::npos) {
       colPos = atoi(tmp.substr(idx + 1).c_str());
       tmp = tmp.substr(0, idx);
    }
    idx = tmp.find_last_of('/');
    if (idx != std::string::npos) {
       tblName = tmp.substr(idx + 1);
        tmp = tmp.substr(0, idx);
    }
    idx = tmp.find_last_of('/');
    dbName = tmp;
    if (idx != std::string::npos) {
        dbName = tmp.substr(idx + 1);
    }
    return std::make_tuple(dbName, tblName, colPos);
}

std::string GetTblPathFromIdxPath(const std::string& p) {
    std::string tmp = p;
    if (tmp[tmp.length() - 1] == '/') {
        tmp = tmp.substr(0, tmp.length() - 1);
    }
    int idx = tmp.find_last_of('_');
    tmp = tmp.substr(0, idx);
    return tmp;
}

int endswith(std::string s, std::string sub) {
	if (s.rfind(sub) == -1) {//排除出现类似s:23   sub:123的情况.
		return 0;
	}
	else {
		return s.rfind(sub) == (s.length() - sub.length()) ? 1 : 0;
	}

}
int startswith(std::string s, std::string sub) {
	
		return s.find(sub) == 0 ? 1 : 0;
}
