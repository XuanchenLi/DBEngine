//
// parser.h
//   Parser Component Interface
//

#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include "main.h"
#include "parser_internal.h"
#include "utils/DBType.h"
#include "utils/Optrs.h"
#include "utils/RC.h"
#include "utils/AggFun.h"

//
// Structure declarations and output functions
//
struct AttrInfo{
    char     *attrName;   /* attribute name       */
    dbType attrType;    /* type of attribute    */
    int      attrLength;  /* length of attribute  */
};

struct RelAttr{
    char     *relName;    // Relation name (may be NULL)
    char     *attrName;   // Attribute name

    // Print function
    friend std::ostream &operator<<(std::ostream &s, const RelAttr &ra);
};

struct AggRelAttr{
    AggFun   func; 
    char     *relName;    // Relation name (may be NULL)
    char     *attrName;   // Attribute name

    // Print function
    friend std::ostream &operator<<(std::ostream &s, const AggRelAttr &ra);
};

struct Value{
    dbType type;         /* type of value               */
    void     *data;        /* value                       */
			   /* print function              */
    friend std::ostream &operator<<(std::ostream &s, const Value &v);
};

struct Condition{
    RelAttr  lhsAttr;    /* left-hand side attribute            */
    db_optr   op;         /* comparison operator                 */
    int      bRhsIsAttr; /* TRUE if the rhs is an attribute,    */
                         /* in which case rhsAttr below is valid;*/
                         /* otherwise, rhsValue below is valid.  */
    RelAttr  rhsAttr;    /* right-hand side attribute            */
    Value    rhsValue;   /* right-hand side value                */
			 /* print function                               */
    friend std::ostream &operator<<(std::ostream &s, const Condition &c);

};

static Condition NULLCONDITION;

std::ostream &operator<<(std::ostream &s, const db_optr &op);
std::ostream &operator<<(std::ostream &s, const AggFun &func);
std::ostream &operator<<(std::ostream &s, const dbType &at);

//
// Parse function
//
class QM_Manager;
class FM_Manager;
class MM_Buffer;

RC RBparse(FM_Manager &pm, QM_Manager &qm, MM_Buffer & bf);

//
// Error printing function; calls component-specific functions
//
void PrintError(RC rc);

// bQueryPlans is allocated by parse.y.  When bQueryPlans is 1 then the
// query plan chosen for the SFW query will be displayed.  When
// bQueryPlans is 0 then no query plan is shown.
extern int bQueryPlans;

#endif