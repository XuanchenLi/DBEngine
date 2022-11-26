#ifndef RM_RID_H
#define RM_RID_H

struct RM_Rid {
    RM_Rid(): num(-1), slot(-1){}
    RM_Rid(int n, int s) {num = n; slot = s;}
    bool operator==(const RM_Rid& rhs) const {
        return num == rhs.num && slot == rhs.slot;
    }
    int num;
    int slot;
};



#endif