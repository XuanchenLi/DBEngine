/*
 * interp.c: interpreter for RQL
 *
 * Authors: Dallan Quass (quass@cs.stanford.edu)
 *          Jan Jannink
 * originally by: Mark McAuliffe, University of Wisconsin - Madison, 1991
 */

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include "main.h"
#include "parser_internal.h"
#include "QM/QM_Manager.h"
#include "utils/DBType.h"
#include "utils/AggFun.h"
#include "utils/Optrs.h"
#include "utils/RC.h"
#include "utils/RelAttrUtil.h"
#include "parser.tab.hpp"




extern QM_Manager *pQm;

#define E_OK                0
#define E_INCOMPATIBLE      -1
#define E_TOOMANY           -2
#define E_NOLENGTH          -3
#define E_INVINTSIZE        -4
#define E_INVREALSIZE       -5
#define E_INVFORMATSTRING   -6
#define E_INVSTRLEN         -7
#define E_DUPLICATEATTR     -8
#define E_TOOLONG           -9
#define E_STRINGTOOLONG     -10

/*
 * file pointer to which error messages are printed
 */
#define ERRFP stderr

/*
 * local functions
 */
static void print_error(char *errmsg, RC errval);
static void echo_query(NODE *n);
static void print_attrtypes(NODE *n);
static void print_op(db_optr op);
static void print_relattr(NODE *n);
static void print_aggrelattr(NODE *n);
static void print_orderattr(NODE *n);
static void print_groupattr(NODE *n);
static void print_value(NODE *n);
static void print_condition(NODE *n);
static void print_relattrs(NODE *n);
static void print_aggrelattrs(NODE *n);
static void print_relations(NODE *n);
static void print_conditions(NODE *n);
static void print_values(NODE *n);

/*
 * interp: interprets parse trees
 *
 */
RC interp(NODE *n)
{
   RC errval = SUCCESS;
   //std::cout<<"Not Implemented"<<std::endl;
   switch(n -> kind) {
      case N_CREATETABLE:
         {
            break;
         }
      case N_CREATEINDEX:
         {
            break;
         }
      case N_DROPINDEX:
         {
            break;
         }
      case N_DROPTABLE:
         {
            break;
         }
      case N_QUERY:
         {
            std::vector<MRelAttr> selAttrs;
            std::vector<std::string> rels;
            std::vector<DB_Cond> conds;
            NODE * listPtr = n->u.QUERY.rellist;
            int cnt = 0;
            //std::cout<<"111\n";
            while(listPtr != NULL) {
               NODE* tmpPtr = listPtr->u.LIST.curr;
               rels.push_back(tmpPtr->u.RELATION.relname);
               listPtr = listPtr->u.LIST.next;
               cnt++;
            }
            listPtr = n->u.QUERY.relattrlist;
            //std::cout<<"112\n";
            while(listPtr != NULL) {
               NODE* tmpPtr = listPtr->u.LIST.curr;
               std::string relN, attrN;
               if (cnt > 1 && tmpPtr->u.AGGRELATTR.relname == NULL) {
                  std::cout<<"ERROR: NAME NOT COMPLETE.\n";
                  return BAD_QUERY;
               }
               relN = tmpPtr->u.AGGRELATTR.relname == NULL ? "" : tmpPtr->u.AGGRELATTR.relname;
               attrN = tmpPtr->u.AGGRELATTR.attrname;
               
               if (attrN == "*") {
                  ExpandAttrs(selAttrs, rels);
                  break;
               }else {
                  selAttrs.push_back(MRelAttr(relN, attrN));
               }

               listPtr = listPtr->u.LIST.next;
            }
            listPtr = n->u.QUERY.conditionlist;
            //std::cout<<"113\n";
            while (listPtr != NULL) {
               NODE* tmpPtr = listPtr->u.LIST.curr;
               DB_Cond cond;
               if (cnt > 1 && tmpPtr->u.CONDITION.lhsRelattr->u.RELATTR.relname == NULL) {
                  std::cout<<"ERROR: NAME NOT COMPLETE.\n";
                  return BAD_QUERY;
               }
               cond.lTblName = tmpPtr->u.CONDITION.lhsRelattr->u.RELATTR.relname == NULL ? "" : tmpPtr->u.CONDITION.lhsRelattr->u.RELATTR.relname;
               cond.lColName = tmpPtr->u.CONDITION.lhsRelattr->u.RELATTR.attrname;
               cond.optr = tmpPtr->u.CONDITION.op;
               if (tmpPtr->u.CONDITION.rhsRelattr == NULL) {
                  cond.isConst = true;
                  cond.type = tmpPtr->u.CONDITION.rhsValue->u.VALUE.type;
                  switch (cond.type) {
                     case DB_INT:
                        cond.data.iData = tmpPtr->u.CONDITION.rhsValue->u.VALUE.ival;
                        break;
                     case DB_DOUBLE:
                        cond.data.lfData = tmpPtr->u.CONDITION.rhsValue->u.VALUE.rval;
                        break;
                     case DB_STRING:
                        strcpy(cond.data.sData, tmpPtr->u.CONDITION.rhsValue->u.VALUE.sval);
                        break;
                  }
                  //tmpPtr->u.CONDITION.rhsValue->u.VALUE
               }else {
                  cond.isConst = false;
                  cond.rColName = tmpPtr->u.CONDITION.rhsRelattr->u.RELATTR.attrname;
                  cond.rTblName = tmpPtr->u.CONDITION.rhsRelattr->u.RELATTR.relname == NULL ? "" : tmpPtr->u.CONDITION.rhsRelattr->u.RELATTR.relname;
               }
               listPtr = listPtr->u.LIST.next;
            }
            //std::cout<<"11\n";
            errval = pQm->Select(selAttrs, rels, conds);
            break;
         }
         

   }
   return errval;
}


/*
 * print_error: prints an error message corresponding to errval
 */
static void print_error(char *errmsg, RC errval)
{
   if(errmsg != NULL)
      fprintf(stderr, "%s: ", errmsg);
   switch(errval){
      case E_OK:
         fprintf(ERRFP, "no error\n");
         break;
      case E_INCOMPATIBLE:
         fprintf(ERRFP, "attributes must be from selected relation(s)\n");
         break;
      case E_TOOMANY:
         fprintf(ERRFP, "too many elements\n");
         break;
      case E_NOLENGTH:
         fprintf(ERRFP, "length must be specified for STRING attribute\n");
         break;
      case E_INVINTSIZE:
         fprintf(ERRFP, "invalid size for INTEGER attribute (should be %d)\n",
               (int)sizeof(int));
         break;
      case E_INVREALSIZE:
         fprintf(ERRFP, "invalid size for REAL attribute (should be %d)\n",
               (int)sizeof(real));
         break;
      case E_INVFORMATSTRING:
         fprintf(ERRFP, "invalid format string\n");
         break;
      case E_INVSTRLEN:
         fprintf(ERRFP, "invalid length for string attribute\n");
         break;
      case E_DUPLICATEATTR:
         fprintf(ERRFP, "duplicated attribute name\n");
         break;
      case E_TOOLONG:
         fprintf(stderr, "relation name or attribute name too long\n");
         break;
      case E_STRINGTOOLONG:
         fprintf(stderr, "string attribute too long\n");
         break;
      default:
         fprintf(ERRFP, "unrecognized errval: %d\n", errval);
   }
}

static void echo_query(NODE *n)
{
   switch(n -> kind){
      case N_CREATETABLE:            /* for CreateTable() */
         printf("create table %s (", n -> u.CREATETABLE.relname);
         print_attrtypes(n -> u.CREATETABLE.attrlist);
         printf(")");
         printf(";\n");
         break;
      case N_CREATEINDEX:            /* for CreateIndex() */
         printf("create index %s(%s);\n", n -> u.CREATEINDEX.relname,
               n -> u.CREATEINDEX.attrname);
         break;
      case N_DROPINDEX:            /* for DropIndex() */
         printf("drop index %s(%s);\n", n -> u.DROPINDEX.relname,
               n -> u.DROPINDEX.attrname);
         break;
      case N_DROPTABLE:            /* for DropTable() */
         printf("drop table %s;\n", n -> u.DROPTABLE.relname);
         break;
      case N_LOAD:            /* for Load() */
         printf("load %s(\"%s\");\n",
               n -> u.LOAD.relname, n -> u.LOAD.filename);
         break;
      case N_HELP:            /* for Help() */
         printf("help");
         if(n -> u.HELP.relname != NULL)
            printf(" %s", n -> u.HELP.relname);
         printf(";\n");
         break;
      case N_PRINT:            /* for Print() */
         printf("print %s;\n", n -> u.PRINT.relname);
         break;
      case N_SET:                                 /* for Set() */
         printf("set %s = \"%s\";\n", n->u.SET.paramName, n->u.SET.string);
         break;
      case N_QUERY:            /* for Query() */
         printf("select ");
         print_aggrelattrs(n -> u.QUERY.relattrlist);
         printf("\n from ");
         print_relations(n -> u.QUERY.rellist);
         if (n->u.QUERY.conditionlist) {
            printf("\n");
            printf("where ");
            print_conditions(n->u.QUERY.conditionlist);
         }
         if (n->u.QUERY.orderrelattr) {
           print_orderattr(n->u.QUERY.orderrelattr);
         }
         if (n->u.QUERY.grouprelattr) {
           print_groupattr(n->u.QUERY.grouprelattr);
         }

         printf(";\n");
         break;
      case N_INSERT:            /* for Insert() */
         printf("insert into %s values ( ",n->u.INSERT.relname);
         print_values(n -> u.INSERT.valuelist);
         printf(");\n");
         break;
      case N_DELETE:            /* for Delete() */
         printf("delete %s ",n->u.DELETE.relname);
         if (n->u.DELETE.conditionlist) {
            printf("where ");
            print_conditions(n->u.DELETE.conditionlist);
         }
         printf(";\n");
         break;
      case N_UPDATE:            /* for Update() */
         {
            printf("update %s set ",n->u.UPDATE.relname);
            print_relattr(n->u.UPDATE.relattr);
            printf(" = ");
            struct node *rhs = n->u.UPDATE.relorvalue;

            /* The RHS can be either a relation.attribute or a value */
            if (rhs->u.RELATTR_OR_VALUE.relattr) {
               /* Print out the relation.attribute */
               print_relattr(rhs->u.RELATTR_OR_VALUE.relattr);
            } else {
               /* Print out the value */
               print_value(rhs->u.RELATTR_OR_VALUE.value);
            }
            if (n->u.UPDATE.conditionlist) {
               printf("where ");
               print_conditions(n->u.UPDATE.conditionlist);
            }
            printf(";\n");
            break;
         }
      default:   // should never get here
         break;
   }
   fflush(stdout);
}

static void print_attrtypes(NODE *n)
{
   NODE *attr;

   for(; n != NULL; n = n -> u.LIST.next){
      attr = n -> u.LIST.curr;
      printf("%s = %s", attr -> u.ATTRTYPE.attrname, attr -> u.ATTRTYPE.type);
      if(n -> u.LIST.next != NULL)
         printf(", ");
   }
}

static void print_op(db_optr op)
{
   switch(op){
      case EQUAL:
         printf(" =");
         break;
      case NOT_EQUAL:
         printf(" <>");
         break;
      case LESS:
         printf(" <");
         break;
      case NOT_GREATER:
         printf(" <=");
         break;
      case GREATER:
         printf(" >");
         break;
      case NOT_LESS:
         printf(" >=");
         break;
      case NO_OP:
         printf(" NO_OP");
         break;
   }
}

static void print_orderattr(NODE *n)
{
   if (n->u.ORDERATTR.order != 0) {
     printf("\norder by");
     print_relattr(n->u.ORDERATTR.relattr);
     printf(" %s", (n->u.ORDERATTR.order == 1 ? "ASC" : "DESC"));
   }        
}  

static void print_groupattr(NODE *n)
{
   if (n->u.RELATTR.attrname) {
     printf("\ngroup by");
     print_relattr(n);
   }        
}  

static void print_relattr(NODE *n)
{
   printf(" ");
   if (n->u.RELATTR.relname)
      printf("%s.",n->u.RELATTR.relname);
   printf("%s",n->u.RELATTR.attrname);
}  

static void print_aggrelattr(NODE *n)
{
   printf(" ");
   if (n->u.AGGRELATTR.func != NO_F) {
     char * foo = "NO_F";
     if(n->u.AGGRELATTR.func == MAX_F)
       foo = "MAX";
     else if(n->u.AGGRELATTR.func == MIN_F)
       foo = "MIN";
     else if(n->u.AGGRELATTR.func == COUNT_F)
       foo = "COUNT";
     else if(n->u.AGGRELATTR.func == AVG_F)
       foo = "AVG";
     else if(n->u.AGGRELATTR.func == SUM_F)
       foo = "SUM";
     printf("%s(",foo);
   }
   if (n->u.AGGRELATTR.relname)
      printf("%s.",n->u.AGGRELATTR.relname);
   printf("%s",n->u.AGGRELATTR.attrname);
   if (n->u.AGGRELATTR.func != NO_F)
      printf(")");
}  

static void print_value(NODE *n)
{
   switch(n -> u.VALUE.type){
      case DB_INT:
         printf(" %d", n -> u.VALUE.ival);
         break;
      case DB_DOUBLE:
         printf(" %f", n -> u.VALUE.rval);
         break;
      case DB_STRING:
         printf(" \"%s\"", n -> u.VALUE.sval);
         break;
   }
}

static void print_condition(NODE *n)
{
   print_relattr(n->u.CONDITION.lhsRelattr);
   print_op(n->u.CONDITION.op);
   if (n->u.CONDITION.rhsRelattr)
      print_relattr(n->u.CONDITION.rhsRelattr);
   else
      print_value(n->u.CONDITION.rhsValue);
}

static void print_relattrs(NODE *n)
{
   for(; n != NULL; n = n -> u.LIST.next){
      print_relattr(n->u.LIST.curr);
      if(n -> u.LIST.next != NULL)
         printf(",");
   }
}

static void print_aggrelattrs(NODE *n)
{
   for(; n != NULL; n = n -> u.LIST.next){
      print_aggrelattr(n->u.LIST.curr);
      if(n -> u.LIST.next != NULL)
         printf(",");
   }
}

static void print_relations(NODE *n)
{
   for(; n != NULL; n = n -> u.LIST.next){
      printf(" %s", n->u.LIST.curr->u.RELATION.relname);
      if(n -> u.LIST.next != NULL)
         printf(",");
   }
}

static void print_conditions(NODE *n)
{
   for(; n != NULL; n = n -> u.LIST.next){
      print_condition(n->u.LIST.curr);
      if(n -> u.LIST.next != NULL)
         printf(" and");
   }
}

static void print_values(NODE *n)
{
   for(; n != NULL; n = n -> u.LIST.next){
      print_value(n->u.LIST.curr);
      if(n -> u.LIST.next != NULL)
         printf(",");
   }
}
