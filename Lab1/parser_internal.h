/*
 * parser_internal.h: internal declarations for the REDBASE parser
 *
 * Authors: Dallan Quass
 *          Jan Jannink
 *
 * originally by: Mark McAuliffe, University of Wisconsin - Madison, 1991
 */

#ifndef PARSER_INTERNAL_H
#define PARSER_INTERNAL_H

#include "parser.h"

/*
 * use double for real
 */
typedef double real;

/*
 * the prompt
 */
#define PROMPT	"\nDBSystem >> "

/*
 * REL_ATTR: describes a qualified attribute (relName.attrName)
 */
typedef struct{
    char *relName;      /* relation name        */
    char *attrName;     /* attribute name       */
} REL_ATTR;

/*
 * ATTR_VAL: <attribute, value> pair
 */
typedef struct{
    char *attrName;     /* attribute name       */
    dbType valType;   /* type of value        */
    int valLength;      /* length if type = STRING */
    void *value;        /* value for attribute  */
} ATTR_VAL;

/*
 * all the available kinds of nodes
 */
typedef enum{
    N_CREATETABLE,
    N_CREATEINDEX,
    N_DROPTABLE,
    N_DROPINDEX,
    N_LOAD,
    N_SET,
    N_HELP,
    N_PRINT,
    N_QUERY,
    N_INSERT,
    N_DELETE,
    N_UPDATE,
    N_RELATTR,
    N_ORDERATTR,
    N_AGGRELATTR,
    N_CONDITION,
    N_RELATTR_OR_VALUE,
    N_ATTRTYPE,
    N_VALUE,
    N_RELATION,
    N_STATISTICS,
    N_LIST
} NODEKIND;

/*
 * structure of parse tree nodes
 */
typedef struct node{
   NODEKIND kind;

   union{
      /* SM component nodes */
      /* create table node */
      struct{
         char *relname;
         struct node *attrlist;
      } CREATETABLE;

      /* create index node */
      struct{
         char *relname;
         char *attrname;
      } CREATEINDEX;

      /* drop index node */
      struct{
         char *relname;
         char *attrname;
      } DROPINDEX;

      /* drop table node */
      struct{
         char *relname;
      } DROPTABLE;

      /* load node */
      struct{
         char *relname;
         char *filename;
      } LOAD;

      /* set node */
      struct{
         char *paramName;
         char *string;
      } SET;

      /* help node */
      struct{
         char *relname;
      } HELP;

      /* print node */
      struct{
         char *relname;
      } PRINT;

      /* QL component nodes */
      /* query node */
      struct{
         struct node *relattrlist;
         struct node *rellist;
         struct node *conditionlist;
         struct node *orderrelattr; 
         struct node *grouprelattr; 
      } QUERY;

      /* insert node */
      struct{
         char *relname;
         struct node *valuelist;
      } INSERT;

      /* delete node */
      struct{
         char *relname;
         struct node *conditionlist;
      } DELETE;

      /* update node */
      struct{
         char *relname;
         struct node *relattr;
         struct node *relorvalue;
         struct node *conditionlist;
      } UPDATE;

      /* command support nodes */
      /* relation attribute node */
      struct{
         char *relname;
         char *attrname;
      } RELATTR;

      /* order + relation attribute node */
      struct{
         int order;
         struct node *relattr;
      } ORDERATTR;

      /* agg func + relation attribute node */
      struct{
         AggFun func;
         char *relname;
         char *attrname;
      } AGGRELATTR;

      /* condition node */
      struct{
         struct node *lhsRelattr;
         db_optr op;
         struct node *rhsRelattr;
         struct node *rhsValue;
      } CONDITION;

      /* relation-attribute or value */
      struct{
         struct node *relattr;
         struct node *value;
      } RELATTR_OR_VALUE;

      /* <attribute, type> pair */
      struct{
         char *attrname;
         char *type;
      } ATTRTYPE;

      /* <value, type> pair */
      struct{
         dbType type;
         int  ival;
         real rval;
         char *sval;
      } VALUE;

      /* relation node */
      struct{
         char *relname;
      } RELATION;

      /* list node */
      struct{
         struct node *curr;
         struct node *next;
      } LIST;
   } u;
} NODE;


/*
 * function prototypes
 */
NODE *newnode(NODEKIND kind);
NODE *create_table_node(char *relname, NODE *attrlist);
NODE *create_index_node(char *relname, char *attrname);
NODE *drop_index_node(char *relname, char *attrname);
NODE *drop_table_node(char *relname);
NODE *load_node(char *relname, char *filename);
NODE *set_node(char *paramName, char *string);
NODE *help_node(char *relname);
NODE *print_node(char *relname);
NODE *query_node(NODE *relattrlist, NODE *rellist, NODE *conditionlist, 
                 NODE *order_relattr, NODE *group_relattr);
NODE *insert_node(char *relname, NODE *valuelist);
NODE *delete_node(char *relname, NODE *conditionlist);
NODE *update_node(char *relname, NODE *relattr, NODE *value,
                  NODE *conditionlist);
NODE *relattr_node(char *relname, char *attrname);
NODE *orderattr_node(int order, NODE *relattr);
NODE *aggrelattr_node(AggFun a, char *relname, char *attrname);
NODE *condition_node(NODE *lhsRelattr, db_optr op, NODE *rhsRelattrOrValue);
NODE *value_node(dbType type, void *value);
NODE *relattr_or_value_node(NODE *relattr, NODE *value);
NODE *attrtype_node(char *attrname, char *type);
NODE *relation_node(char *relname);
NODE *list_node(NODE *n);
NODE *prepend(NODE *n, NODE *list);

void reset_scanner(void);
void reset_charptr(void);
void new_query(void);
RC   interp(NODE *n);
extern "C" int yylex();
//int  yylex(void);
int  yyparse(void);
//void yyerror(char const *s);

#endif



