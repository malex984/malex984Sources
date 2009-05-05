/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/* $Id: attrib.cc,v 1.34 2009-05-05 09:54:38 Singular Exp $ */

/*
* ABSTRACT: attributes to leftv and idhdl
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include "mod2.h"
#include "omalloc.h"
#include "tok.h"
#include "ipid.h"
#include "intvec.h"
#include "polys.h"
#include "ideals.h"
#include "matpol.h"
#include "ipshell.h"
#include "attrib.h"

static omBin sattr_bin = omGetSpecBin(sizeof(sattr));

void sattr::Print()
{
  omCheckAddrSize(this,sizeof(sattr));
  ::Print("attr:%s, type %s \n",name,Tok2Cmdname(atyp));
  if (next!=NULL) next->Print();
}

attr sattr::Copy()
{
  omCheckAddrSize(this,sizeof(sattr));
  attr n=(attr)omAlloc0Bin(sattr_bin);
  n->atyp=atyp;
  if (name!=NULL) n->name=omStrDup(name);
  n->data=CopyA();
  if (next!=NULL)
  {
    n->next=next->Copy();
  }
  return n;
}

// in subexr.cc:
//void * sattr::CopyA()
//{
//  omCheckAddrSize(this,sizeof(sattr));
//  return s_internalCopy(atyp,data);
//}

attr sattr::set(const char * s, void * data, int t)
{
  attr h = get(s);
  if (h!=NULL)
  {
    switch (h->atyp)
    {
    case INTVEC_CMD:
      delete (intvec *)h->data;
      break;
    case IDEAL_CMD:
    case MODUL_CMD:
    case MATRIX_CMD:
      idDelete((ideal *)&h->data);
      break;
    case POLY_CMD:
    case VECTOR_CMD:
      pDelete((poly *)&h->data);
      break;
    case INT_CMD:
      break;
    case STRING_CMD:
      omFree((ADDRESS)h->data);
      break;
#ifdef TEST
    default:
      ::Print("at-set: unknown type(%d)\n",atyp);  /* DEBUG */
#endif
    } /* end switch: (atyp) */
    omFree((ADDRESS)s);
  }
  else
  {
     h = (attr)omAlloc0Bin(sattr_bin);
     h->name = s;
     h->next = this;
     h->data = data;
     h->atyp = t;
     return  h;
  }
  //::Print("set attr >>%s<< of type %d\n",h->name,t);
  h->data = data;
  h->atyp = t;
  return  this;
}

attr sattr::get(const char * s)
{
  attr h = this;
  while (h!=NULL)
  {
    if (0 == strcmp(s,h->name)) return h;
    h = h->next;
  }
  return NULL;
}

void * atGet(idhdl root,const char * name)
{
  attr temp = root->attribute->get(name);
  if (temp!=NULL)
    return temp->data;
  else
    return NULL;
}

void * atGet(leftv root,const char * name)
{
  attr temp;
  if (root->e==NULL)
    temp = root->attribute->get(name);
  else
    temp = (root->LData())->attribute->get(name);
  if ((temp==NULL) && (root->rtyp==IDHDL))
  {
    idhdl h=(idhdl)root->data;
    temp=h->attribute->get(name);
  }
  if (temp!=NULL)
    return temp->data;
  else
    return NULL;
}

void * atGet(idhdl root,const char * name, int t)
{
  attr temp = root->attribute->get(name);
  if ((temp!=NULL) && (temp->atyp==t))
    return temp->data;
  else
    return NULL;
}

void * atGet(leftv root,const char * name, int t)
{
  attr temp = root->attribute->get(name);
  if ((temp==NULL) && (root->rtyp==IDHDL))
  {
    idhdl h=(idhdl)root->data;
    temp=h->attribute->get(name);
  }
  if ((temp!=NULL) && (temp->atyp==t))
    return temp->data;
  else
    return NULL;
}

void atSet(idhdl root,const char * name,void * data,int typ)
{
  if (root!=NULL)
  {
    root->attribute=root->attribute->set(name,data,typ);
  }
}

void atSet(leftv root,const char * name,void * data,int typ)
{
  if (root!=NULL)
  {
    if (root->e!=NULL)
    {
      Werror("object must have a name for attrib %s",name);
    }
    else
    {
      if (root->rtyp==IDHDL)
      {
        idhdl h=(idhdl)root->data;
        h->attribute=h->attribute->set(name,data,typ);
        root->attribute=h->attribute;
      }
      else
      {
        root->attribute=root->attribute->set(name,data,typ);
      }
    }
  }
}

void sattr::kill(const ring r)
{
  omFree((ADDRESS)name);
  name=NULL;
  switch (atyp)
  {
  case INTVEC_CMD:
    delete (intvec *)data;
    break;
  case IDEAL_CMD:
  case MODUL_CMD:
  case MATRIX_CMD:
    id_Delete((ideal *)&data,r);
    break;
  case POLY_CMD:
  case VECTOR_CMD:
    p_Delete((poly *)&data,r);
    break;
  case INT_CMD:
    break;
  case STRING_CMD:
    omFree((ADDRESS)data);
    break;
#ifdef TEST
  default:
    ::Print("atKill: unknown type(%d)\n",atyp);  /* DEBUG */
#endif
  } /* end switch: (atyp) */
  data=NULL;
  omFreeBin((ADDRESS)this, sattr_bin);
}

void sattr::killAll(const ring r)
{
  attr temp = this,temp1;

  while (temp!=NULL)
  {
    temp1 = temp->next;
    temp->kill(r);
    temp = temp1;
  }
}

void at_Kill(idhdl root,const char * name, const ring r)
{
  attr temp = root->attribute->get(name);
  if (temp!=NULL)
  {
    attr N = temp->next;
    attr temp1 = root->attribute;
    if (temp1==temp)
    {
      root->attribute = N;
    }
    else
    {
      while (temp1->next!=temp) temp1 = temp1->next;
      temp1->next = N;
    }
    temp->kill(r);
  }
}

void at_KillAll(idhdl root, const ring r)
{
  root->attribute->killAll(r);
  root->attribute = NULL;
}

BOOLEAN atATTRIB1(leftv res,leftv a)
{
  leftv v=a;
  int t;
  if (a->e!=NULL)
  {
    v=a->LData();
    if (v==NULL) return TRUE;
  }
  attr at=v->attribute;
  if (hasFlag(v,FLAG_STD))
  {
    PrintS("attr:isSB, type int\n");
    if (at!=NULL) at->Print();
  }
  else if (((t=v->Typ())==RING_CMD)||(t==QRING_CMD))
  {
    PrintS("attr:global, type int\n");
    if (at!=NULL) at->Print();
  }
  else
  {
    if (at!=NULL) at->Print();
    else          PrintS("no attributes\n");
  }
  return FALSE;
}
BOOLEAN atATTRIB2(leftv res,leftv a,leftv b)
{
  char *name=(char *)b->Data();
  int t;
  leftv v=a;
  if (a->e!=NULL)
  {
    v=a->LData();
    if (v==NULL) return TRUE;
  }
  if (strcmp(name,"isSB")==0)
  {
    res->rtyp=INT_CMD;
    res->data=(void *)(long)hasFlag(v,FLAG_STD);
  }
  else if ((strcmp(name,"rank")==0)&&(v->Typ()==MODUL_CMD))
  {
    res->rtyp=INT_CMD;
    res->data=(void *)(((ideal)v->Data())->rank);
  }
  else if ((strcmp(name,"global")==0)
  &&(((t=v->Typ())==RING_CMD)||(t==QRING_CMD)))
  {
    res->rtyp=INT_CMD;
    res->data=(void *)(((ring)v->Data())->OrdSgn==1);
  }
#ifdef HAVE_SHIFTBBA
  else if ((strcmp(name,"isLPring")==0)
  &&(((t=v->Typ())==RING_CMD)||(t==QRING_CMD)))
  {
    res->rtyp=INT_CMD;
    res->data=(void *)(long)(((ring)v->Data())->isLPring);
  }
#endif
  else
  {
    attr at=v->attribute->get(name);
    if (at!=NULL)
    {
      res->rtyp=at->atyp;
      res->data=at->CopyA();
    }
    else
    {
      res->rtyp=STRING_CMD;
      res->data=omStrDup("");
    }
  }
  return FALSE;
}
BOOLEAN atATTRIB3(leftv res,leftv a,leftv b,leftv c)
{
  idhdl h=(idhdl)a->data;
  int t;
  leftv v=a;
  if (a->e!=NULL)
  {
    v=a->LData();
    if (v==NULL) return TRUE;
    h=NULL;
  }
  attr *at=&(v->attribute);
  char *name=(char *)b->Data();
  if (strcmp(name,"isSB")==0)
  {
    if (c->Typ()!=INT_CMD)
    {
      WerrorS("attribute isSB must be int");
      return TRUE;
    }
    if (((long)c->Data())!=0L)
    {
      if (h!=NULL) setFlag(h,FLAG_STD);
      setFlag(v,FLAG_STD);
    }
    else
    {
      if (h!=NULL) resetFlag(h,FLAG_STD);
      resetFlag(v,FLAG_STD);
    }
  }
  else if ((strcmp(name,"rank")==0)&&(v->Typ()==MODUL_CMD))
  {
    if (c->Typ()!=INT_CMD)
    {
      WerrorS("attribute `rank` must be int");
      return TRUE;
    }
    ideal I=(ideal)v->Data();
    I->rank=si_max((int)I->rank,(int)((long)c->Data()));
  }
  else if ((strcmp(name,"global")==0)
  &&(((t=v->Typ())==RING_CMD)||(t==QRING_CMD)))
  {
    WerrorS("can not set attribute `global`");
    return TRUE;
  }
#ifdef HAVE_SHIFTBBA
  else if ((strcmp(name,"isLPring")==0)
  &&(((t=v->Typ())==RING_CMD)||(t==QRING_CMD)))
  {
    if (c->Typ()==INT_CMD)
      ((ring)v->Data())->isLPring=(int)(long)c->Data();
    else
    {
      WerrorS("attribute `isLPring` must be int");
      return TRUE;
    }
  }
#endif
  else
  {
    int typ=c->Typ();
    atSet(v,omStrDup(name),c->CopyD(typ),typ/*c->T(yp()*/);
    if (h!=NULL) IDATTR(h)=v->attribute;
  }
  return FALSE;
}

BOOLEAN atKILLATTR1(leftv res,leftv a)
{
  if ((a->rtyp!=IDHDL)||(a->e!=NULL))
  {
    WerrorS("object must have a name");
    return TRUE;
  }
  resetFlag(a,FLAG_STD);
  resetFlag((idhdl)a->data,FLAG_STD);
  if (a->attribute!=NULL)
  {
    atKillAll((idhdl)a->data);
    a->attribute=NULL;
  }
  return FALSE;
}
BOOLEAN atKILLATTR2(leftv res,leftv a,leftv b)
{
  if ((a->rtyp!=IDHDL)||(a->e!=NULL))
  {
    WerrorS("object must have a name");
    return TRUE;
  }
  char *name=(char *)b->Data();
  if (strcmp(name,"isSB")==0)
  {
    resetFlag(a,FLAG_STD);
    resetFlag((idhdl)a->data,FLAG_STD);
  }
  else if (strcmp(name,"global")==0)
  {
    WerrorS("can not set attribut `global`");
    return TRUE;
  }
  else
  {
    atKill((idhdl)a->data,name);
  }
  return FALSE;
}
