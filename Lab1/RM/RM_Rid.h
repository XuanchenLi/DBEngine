#ifndef RM_RID_H
#define RM_RID_H

struct RM_Rid {
    RM_Rid(): num(-1), slot(-1){}
    RM_Rid(int n, int s) {num = n; slot = s;}
    int num;
    int slot;
};



#endif