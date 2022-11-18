#include "DBType.h"


bool comp(dbType type, db_optr optr,const void* lhs,const void*rhs) {
    int re = true;
    if (lhs == nullptr || rhs == nullptr) {
        return false;
    }
    switch(optr) {
                case EQUAL:
                    {
                        switch(type) {
                        case DB_BOOL:
                            if (*(bool*)lhs != *(bool*)rhs)
                                re=false;
                            break;
                        case DB_DOUBLE:
                            if (*(double*)lhs != *(double*)rhs)
                                re=false;
                            break;
                        case DB_INT:
                            if (*(int*)lhs != *(int*)rhs)
                                re=false;
                            break;
                        case DB_STRING:
                            if (strcmp((char*)lhs, (char*)rhs )!=0)
                                re = false;
                            break;
                        }
                    }
                    
                    break;

                case NOT_EQUAL:
                    {
                        switch(type) {
                        case DB_BOOL:
                            if (*(bool*)lhs == *(bool*)rhs)
                                re=false;
                            break;
                        case DB_DOUBLE:
                            if (*(double*)lhs == *(double*)rhs)
                                re=false;
                            break;
                        case DB_INT:
                            if (*(int*)lhs == *(int*)rhs)
                                re=false;
                            break;
                        case DB_STRING:
                            if (strcmp((char*)lhs, (char*)rhs )==0)
                                re = false;
                            break;
                        }
                    }
                    break;

                case LESS:
                    {
                        switch(type) {
                        case DB_BOOL:
                            if (*(bool*)lhs >= *(bool*)rhs)
                                re=false;
                            break;
                        case DB_DOUBLE:
                            if (*(double*)lhs >= *(double*)rhs)
                                re=false;
                            break;
                        case DB_INT:
                            if (*(int*)lhs >= *(int*)rhs)
                                re=false;
                            break;
                        case DB_STRING:
                            if (strcmp((char*)lhs, (char*)rhs )>=0)
                                re = false;
                            break;
                        }
                    }
                    break;

                case GREATER:
                    {
                        switch(type) {
                        case DB_BOOL:
                            if (*(bool*)lhs <= *(bool*)rhs)
                                re=false;
                            break;
                        case DB_DOUBLE:
                            if (*(double*)lhs <= *(double*)rhs)
                                re=false;
                            break;
                        case DB_INT:
                            if (*(int*)lhs <= *(int*)rhs)
                                re=false;
                            break;
                        case DB_STRING:
                            if (strcmp((char*)lhs, (char*)rhs )<=0)
                                re = false;
                            break;
                        }
                    }
                    break;

                case NOT_LESS:
                    {
                        switch(type) {
                        case DB_BOOL:
                            if (*(bool*)lhs < *(bool*)rhs)
                                re=false;
                            break;
                        case DB_DOUBLE:
                            if (*(double*)lhs < *(double*)rhs)
                                re=false;
                            break;
                        case DB_INT:
                            if (*(int*)lhs < *(int*)rhs)
                                re=false;
                            break;
                        case DB_STRING:
                            if (strcmp((char*)lhs, (char*)rhs )<0)
                                re = false;
                            break;
                        }
                    }
                    break;

                case NOT_GREATER:
                    {
                        switch(type) {
                        case DB_BOOL:
                            if (*(bool*)lhs > *(bool*)rhs)
                                re=false;
                            break;
                        case DB_DOUBLE:
                            if (*(double*)lhs > *(double*)rhs)
                                re=false;
                            break;
                        case DB_INT:
                            if (*(int*)lhs > *(int*)rhs)
                                re=false;
                            break;
                        case DB_STRING:
                            if (strcmp((char*)lhs, (char*)rhs )>0)
                                re = false;
                            break;
                        }
                    }
                    break;

            }
    return re;
}