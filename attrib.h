#ifndef ATTRIB_H
#define ATTRIB_H
/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/* $Id: attrib.h,v 1.9 2009-05-05 09:54:37 Singular Exp $ */
/*
* ABSTRACT: attributes to leftv and idhdl
*/
#include <string.h>
#include "structs.h"

class sattr;
typedef sattr * attr;
class sattr
{
  public:
    void Init() { memset(this,0,sizeof(*this)); }
    const char *  name;
    void *  data;
    attr    next;
    int     atyp; /* the type of the attribut, describes the data field
                  */

    void Print();
    attr Copy(); /* copy all arguments */
    void * CopyA(); /* copy the data of this attribute */
    attr set(const char * s, void * data, int t);
    attr get(const char * s);
    void kill(const ring r);
    void killAll(const ring r);
};

void * atGet(idhdl root,const char * name);
void * atGet(leftv root,const char * name);
void * atGet(idhdl root,const char * name, int t);
void * atGet(leftv root,const char * name, int t);
void atSet(idhdl root,const char * name,void * data,int typ);
void atSet(leftv root,const char * name,void * data,int typ);
void at_KillAll(idhdl root,const ring r);
#define atKillAll(H) at_KillAll(H,currRing)
void at_Kill(idhdl root,const char * name,const ring r);
#define atKill(H,A) at_Kill(H,A,currRing)

BOOLEAN atATTRIB1(leftv res,leftv a);
BOOLEAN atATTRIB2(leftv res,leftv a,leftv b);
BOOLEAN atATTRIB3(leftv res,leftv a,leftv b,leftv c);
BOOLEAN atKILLATTR1(leftv res,leftv a);
BOOLEAN atKILLATTR2(leftv res,leftv a,leftv b);
#endif
