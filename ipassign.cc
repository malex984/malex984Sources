/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/* $Id: ipassign.cc,v 1.27 1998-04-07 17:52:19 Singular Exp $ */

/*
* ABSTRACT: interpreter:
*           assignment of expressions and lists to objects or lists
*/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "mod2.h"
#include "tok.h"
#include "ipid.h"
#include "intvec.h"
#include "mmemory.h"
#include "febase.h"
#include "polys.h"
#include "ideals.h"
#include "matpol.h"
#include "kstd1.h"
#include "timer.h"
#include "ring.h"
#include "subexpr.h"
#include "numbers.h"
#include "lists.h"
#include "longalg.h"
#include "stairc.h"
#include "maps.h"
#include "syz.h"
#include "weight.h"
#include "ipconv.h"
#include "attrib.h"
#include "silink.h"
#include "ipshell.h"

/*=================== proc =================*/
#ifdef SRING
static BOOLEAN jjALTVARS(leftv res, leftv a)
{
  pAltVars=(int)a->Data();
  pSRING=(pAltVars<=pVariables);
  pSDRING=pSDRING||pSRING;
  currRing->partN=pAltVars;
  return FALSE;
}
#endif
static BOOLEAN jjECHO(leftv res, leftv a)
{
  si_echo=(int)a->Data();
  return FALSE;
}
static BOOLEAN jjPAGELENGTH(leftv res, leftv a)
{
  pagelength=(int)a->Data();
  return FALSE;
}
static BOOLEAN jjPRINTLEVEL(leftv res, leftv a)
{
  printlevel=(int)a->Data();
  return FALSE;
}
static BOOLEAN jjCOLMAX(leftv res, leftv a)
{
  colmax=(int)a->Data();
  return FALSE;
}
static BOOLEAN jjTIMER(leftv res, leftv a)
{
  timerv=(int)a->Data();
  initTimer();
  return FALSE;
}
#ifdef HAVE_RTIMER
static BOOLEAN jjRTIMER(leftv res, leftv a)
{
  rtimerv=(int)a->Data();
  initRTimer();
  return FALSE;
}
#endif
static BOOLEAN jjMAXDEG(leftv res, leftv a)
{
  Kstd1_deg=(int)a->Data();
  if (Kstd1_deg!=0)
    test |=Sy_bit(24);
  else
    test &=(~Sy_bit(24));
  return FALSE;
}
static BOOLEAN jjMAXMULT(leftv res, leftv a)
{
  Kstd1_mu=(int)a->Data();
  if (Kstd1_mu!=0)
    test |=Sy_bit(23);
  else
    test &=(~Sy_bit(23));
  return FALSE;
}
static BOOLEAN jjTRACE(leftv res, leftv a)
{
  traceit=(int)a->Data();
  return FALSE;
}
static BOOLEAN jjSHORTOUT(leftv res, leftv a)
{
#ifdef HAVE_TCL
  if (!tclmode)
#endif
    pShortOut=(int)a->Data();
  return FALSE;
}
static BOOLEAN jjMINPOLY(leftv res, leftv a)
{
  if ((currRing->parameter!=NULL)
  && ((currRing->P>1)
    || (currRing->ch>1/* GF(q) <=> parameter!=NULL && ch=q>1*/)))
  {
    WerrorS("no minpoly allowed");
    return TRUE;
  }
  if (currRing->minpoly!=NULL)
  {
    WerrorS("minpoly already set");
    return TRUE;
  }
  number p=(number)a->CopyD(NUMBER_CMD);
  if (!nIsZero(p))
  {
    nNormalize(p);
    currRing->minpoly=p;
    naMinimalPoly=((lnumber)currRing->minpoly)->z;
  }
  else
  {
    naMinimalPoly=NULL;
  }
  return FALSE;
}
static BOOLEAN jjNOETHER(leftv res, leftv a)
{
  poly p=(poly)a->CopyD(POLY_CMD);
  pDelete(&ppNoether);
  ppNoether=p;
  return FALSE;
}
/*=================== proc =================*/
static void jiAssignAttr(leftv l,leftv r)
{
  // get the attribute of th right side
  // and set it to l
  leftv rv=r->LData();
  if (rv!=NULL)
  {
    if (rv->e==NULL)
    {
      if (rv->attribute!=NULL)
      {
        attr la;
        if (r->rtyp!=IDHDL)
        {
          la=rv->attribute;
          rv->attribute=NULL;
        }
        else
        {
          la=rv->attribute->Copy();
        }
        l->attribute=la;
      }
      l->flag=rv->flag;
    }
  }
  if (l->rtyp==IDHDL)
  {
    idhdl h=(idhdl)l->data;
    IDATTR(h)=l->attribute;
    IDFLAG(h)=l->flag;
  }
}
static BOOLEAN jiA_INT(leftv res, leftv a, Subexpr e)
{
  if (e==NULL)
  {
    res->data=(void *)a->Data();
    jiAssignAttr(res,a);
  }
  else
  {
    int i=e->start-1;
    if (i<0)
    {
      Werror("index[%d] must be positive",i+1);
      return TRUE;
    }
    intvec *iv=(intvec *)res->data;
    if (e->next==NULL)
    {
      if (i>=iv->length())
      {
        intvec *iv1=new intvec(i+1);
        (*iv1)[i]=(int)a->Data();
        intvec *ivn=ivAdd(iv,iv1);
        delete iv;
        delete iv1;
        res->data=(void *)ivn;
      }
      else
        (*iv)[i]=(int)a->Data();
    }
    else
    {
      int c=e->next->start;
      if ((i>=iv->rows())||(c<1)||(c>iv->cols()))
      {
        Werror("wrong range [%d,%d] in intmat (%d,%d)",i+1,c,iv->rows(),iv->cols());
        return TRUE;
      }
      else
        IMATELEM(*iv,i+1,c) = (int)a->Data();
    }
  }
  return FALSE;
}
static BOOLEAN jiA_NUMBER(leftv res, leftv a, Subexpr e)
{
  number p=(number)a->CopyD(NUMBER_CMD);
  if (res->data!=NULL) nDelete((number *)&res->data);
  nNormalize(p);
  res->data=(void *)p;
  jiAssignAttr(res,a);
  return FALSE;
}
static BOOLEAN jiA_LIST_RES(leftv res, leftv a,Subexpr e)
{
  syStrategy r=(syStrategy)a->CopyD();
  if (res->data!=NULL) ((lists)res->data)->Clean();
  res->data=(void *)syConvRes(r);
  //jiAssignAttr(res,a);
  return FALSE;
}
static BOOLEAN jiA_LIST(leftv res, leftv a,Subexpr e)
{
  lists l=(lists)a->CopyD();
  if (res->data!=NULL) ((lists)res->data)->Clean();
  res->data=(void *)l;
  jiAssignAttr(res,a);
  return FALSE;
}
static BOOLEAN jiA_POLY(leftv res, leftv a,Subexpr e)
{
  poly p=(poly)a->CopyD(POLY_CMD);
  pNormalize(p);
  if (e==NULL)
  {
    if (res->data!=NULL) pDelete((poly*)&res->data);
    res->data=(void*)p;
    jiAssignAttr(res,a);
  }
  else
  {
    int i,j;
    matrix m=(matrix)res->data;
    i=e->start;
    if (e->next==NULL)
    {
      j=i; i=1;
      // for ideal/module: check indices
      if (res->rtyp!=MAP_CMD)
      {
        if (j>MATCOLS(m))
        {
          pEnlargeSet(&(m->m),MATCOLS(m),j-MATCOLS(m));
          MATCOLS(m)=j;
        }
        else if (j<=0)
        {
          Werror("index[%d] must be positive",j/*e->start*/);
          return TRUE;
        }
      }
    }
    else
    {
      // for matrices: indices are correct (see ipExprArith3(..,'['..) )
      j=e->next->start;
    }
    pDelete(&MATELEM(m,i,j));
    MATELEM(m,i,j)=p;
    /* for module: update rank */
    if ((p!=NULL) && (pGetComp(p)!=0))
    {
      m->rank=max(m->rank,pMaxComp(p));
    }
  }
  return FALSE;
}
static BOOLEAN jiA_1x1MATRIX(leftv res, leftv a,Subexpr e)
{
  if ((res->rtyp!=MATRIX_CMD) /*|| (e!=NULL) - TRUE because of type poly */)
     return TRUE;
  matrix am=(matrix)a->CopyD(MATRIX_CMD);
  if ((MATROWS(am)!=1) || (MATCOLS(am)!=1))
  {
    idDelete((ideal *)&am);
    return TRUE;
  }
  matrix m=(matrix)res->data;
  // indices are correct (see ipExprArith3(..,'['..) )
  int i=e->start;
  int j=e->next->start;
  pDelete(&MATELEM(m,i,j));
  pNormalize(MATELEM(am,1,1));
  MATELEM(m,i,j)=MATELEM(am,1,1);
  return FALSE;
}
static BOOLEAN jiA_STRING(leftv res, leftv a, Subexpr e)
{
  if (e==NULL)
  {
    FreeL((ADDRESS)res->data);
    res->data=(void *)a->CopyD(STRING_CMD);
    jiAssignAttr(res,a);
  }
  else
  {
    char *s=(char *)res->data;
    if ((e->start>0)&&(e->start<=(int)strlen(s)))
      s[e->start-1]=(char)(*((char *)a->Data()));
    else
    {
      Werror("string index %d out of range 1..%d",e->start,strlen(s));
      return TRUE;
    }
  }
  return FALSE;
}
static BOOLEAN jiA_PROC(leftv res, leftv a, Subexpr e)
{
  extern procinfo *iiInitSingularProcinfo(procinfo *pi, char *libname,
					  char *procname, int line,
					  long pos, BOOLEAN pstatic=FALSE);
  extern void piCleanUp(procinfov pi);

  if(res->data!=NULL) piCleanUp((procinfo *)res->data);
  if(a->rtyp==STRING_CMD)
  {
    res->data = (void *)Alloc0(sizeof(procinfo));
    ((procinfo *)(res->data))->language=LANG_NONE;
    iiInitSingularProcinfo((procinfo *)res->data,"",res->name,0,0);
    ((procinfo *)res->data)->data.s.body=(char *)a->CopyD(STRING_CMD);
  }
  else
    res->data=(void *)a->CopyD(PROC_CMD);
  jiAssignAttr(res,a);
  return FALSE;
}
static BOOLEAN jiA_INTVEC(leftv res, leftv a, Subexpr e)
{
  if (res->data!=NULL) delete ((intvec *)res->data);
  res->data=(void *)a->CopyD(INTVEC_CMD);
  jiAssignAttr(res,a);
  return FALSE;
}
static BOOLEAN jiA_IDEAL(leftv res, leftv a, Subexpr e)
{
  if (res->data!=NULL) idDelete((ideal*)&res->data);
  res->data=(void *)a->CopyD(MATRIX_CMD);
  jiAssignAttr(res,a);
  return FALSE;
}
static BOOLEAN jiA_RESOLUTION(leftv res, leftv a, Subexpr e)
{
  if (res->data!=NULL) syKillComputation((syStrategy)res->data);
  res->data=(void *)a->CopyD(RESOLUTION_CMD);
  jiAssignAttr(res,a);
  return FALSE;
}
static BOOLEAN jiA_MODUL_P(leftv res, leftv a, Subexpr e)
{
  if (res->data!=NULL) idDelete((ideal*)&res->data);
  ideal I=idInit(1,1);
  I->m[0]=(poly)a->CopyD();
  if (I->m[0]!=NULL) pSetComp(I->m[0],1);
  res->data=(void *)I;
  return FALSE;
}
static BOOLEAN jiA_IDEAL_M(leftv res, leftv a, Subexpr e)
{
  if (res->data!=NULL) idDelete((ideal*)&res->data);
  matrix m=(matrix)a->CopyD(MATRIX_CMD);
  IDELEMS((ideal)m)=MATROWS(m)*MATCOLS(m);
  ((ideal)m)->rank=1;
  MATROWS(m)=1;
  res->data=(void *)m;
  return FALSE;
}
static BOOLEAN jiA_LINK(leftv res, leftv a, Subexpr e)
{
  si_link l=(si_link)res->data;

  if (l!=NULL) slCleanUp(l);

  if (a->Typ() == STRING_CMD)
  {
    if (l == NULL)
    {
      l = (si_link) Alloc0(sizeof(sip_link));
      res->data = (void *) l;
    }
    return slInit(l, (char *) a->Data());
  }
  else if (a->Typ() == LINK_CMD)
  {
    if (l != NULL) Free(l, sizeof(sip_link));
    res->data = slCopy((si_link)a->Data());
    return FALSE;
  }
  return TRUE;
}
static BOOLEAN jiA_MAP(leftv res, leftv a, Subexpr e)
{
  if (res->data!=NULL)
  {
    FreeL((ADDRESS)((map)res->data)->preimage);
    ((map)res->data)->preimage=NULL;
    idDelete((ideal*)&res->data);
  }
  res->data=(void *)a->CopyD();
  jiAssignAttr(res,a);
  return FALSE;
}
static BOOLEAN jiA_MAP_ID(leftv res, leftv a, Subexpr e)
{
  map f=(map)res->data;
  char *rn=f->preimage; // save the old/already assigned preimage ring name
  f->preimage=NULL;
  idDelete((ideal *)&f);
  res->data=(void *)a->CopyD(IDEAL_CMD);
  f=(map)res->data;
  f->preimage = rn;
  return FALSE;
}
static BOOLEAN jiA_QRING(leftv res, leftv a,Subexpr e)
{
  // the follwing can only happen, if:
  //   - the left side is of type qring AND not an id
  if ((e!=NULL)||(res->rtyp!=IDHDL))
  {
    WerrorS("qring_id expected");
    return TRUE;
  }
  ring qr;
  int i,j;
  int *pi;

  assumeStdFlag(a);
  qr=(ring)res->Data();
  ring qrr=rCopy(currRing);
  // hier fehlt noch: evtl. vorhandenen alten Ring streichen
  // vorerst nur:
  memcpy4(qr,qrr,sizeof(ip_sring));
  Free(qrr,sizeof(ip_sring));
  if (qr->qideal!=NULL) idDelete(&qr->qideal);
  qr->qideal = (ideal)a->CopyD(IDEAL_CMD);
  currRing=qr;
  currRingHdl=(idhdl)res->data;
  currQuotient=qr->qideal;
  return FALSE;
}
static BOOLEAN jiA_RING(leftv res, leftv a, Subexpr e)
{
  if ((e!=NULL)||(res->rtyp!=IDHDL))
  {
    WerrorS("id expected");
    return TRUE;
  }
  ring r=(ring)a->Data();
  idhdl rl=(idhdl)res->data;
  if (&IDRING(rl)!=NULL) rKill(rl);
  r->ref++;
  IDRING(rl)=r;
  if ((IDLEV((idhdl)a->data)!=myynest) && (r==currRing))
    currRingHdl=(idhdl)res->data;
  return FALSE;
}
/*=================== table =================*/
struct sValAssign dAssign[]=
{
// proc         res             arg
 {jiA_IDEAL,    IDEAL_CMD,      IDEAL_CMD }
,{jiA_IDEAL_M,  IDEAL_CMD,      MATRIX_CMD }
,{jiA_RESOLUTION,RESOLUTION_CMD,RESOLUTION_CMD }
,{jiA_INT,      INT_CMD,        INT_CMD }
,{jiA_IDEAL,    MATRIX_CMD,     MATRIX_CMD }
,{jiA_MAP_ID,   MAP_CMD,        IDEAL_CMD }
,{jiA_MAP,      MAP_CMD,        MAP_CMD }
,{jiA_IDEAL,    MODUL_CMD,      MODUL_CMD }
,{jiA_MODUL_P,  MODUL_CMD,      POLY_CMD }
,{jiA_POLY,     POLY_CMD,       POLY_CMD }
,{jiA_1x1MATRIX,POLY_CMD,       MATRIX_CMD }
,{jiA_QRING,    QRING_CMD,      IDEAL_CMD }
,{jiA_RING,     RING_CMD,       RING_CMD }
,{jiA_RING,     QRING_CMD,      QRING_CMD }
,{jiA_STRING,   STRING_CMD,     STRING_CMD }
,{jiA_PROC,     PROC_CMD,       STRING_CMD }
,{jiA_PROC,     PROC_CMD,       PROC_CMD }
,{jiA_POLY,     VECTOR_CMD,     VECTOR_CMD }
,{jiA_INTVEC,   INTVEC_CMD,     INTVEC_CMD }
,{jiA_INTVEC,   INTMAT_CMD,     INTMAT_CMD }
,{jiA_NUMBER,   NUMBER_CMD,     NUMBER_CMD }
,{jiA_LIST_RES, LIST_CMD,       RESOLUTION_CMD }
,{jiA_LIST,     LIST_CMD,       LIST_CMD }
,{jiA_LINK,     LINK_CMD,       STRING_CMD }
,{jiA_LINK,     LINK_CMD,       LINK_CMD }
,{NULL,         0,              0 }
};
struct sValAssign_sys dAssign_sys[]=
{
// sysvars:
 {jjECHO,       VECHO,          INT_CMD }
#ifdef SRING
,{jjALTVARS,    VALTVARS,       INT_CMD }
#endif
,{jjPAGELENGTH, VPAGELENGTH,    INT_CMD }
,{jjPRINTLEVEL, VPRINTLEVEL,    INT_CMD }
,{jjCOLMAX,     VCOLMAX,        INT_CMD }
,{jjTIMER,      VTIMER,         INT_CMD }
#ifdef HAVE_RTIMER
,{jjRTIMER,     VRTIMER,        INT_CMD }
#endif
,{jjMAXDEG,     VMAXDEG,        INT_CMD }
,{jjMAXMULT,    VMAXMULT,       INT_CMD }
,{jjTRACE,      TRACE,          INT_CMD }
,{jjSHORTOUT,   VSHORTOUT,      INT_CMD }
,{jjMINPOLY,    VMINPOLY,       NUMBER_CMD }
,{jjNOETHER,    VNOETHER,       POLY_CMD }
,{NULL,         0,              0 }
};
/*=================== operations ============================*/
/*2
* assign a = b
*/
static BOOLEAN jiAssign_1(leftv l, leftv r)
{
  int rt=r->Typ();
  if (rt==0)
  {
    if (!errorreported) Werror("`%s` is undefined",r->Name());
    return TRUE;
  }

  int lt=l->Typ();
  if((lt==0)/*&&(l->name!=NULL)*/)
  {
    if (!errorreported) Werror("left side `%s` is undefined",l->Name());
    return TRUE;
  }
  if((rt==DEF_CMD)||(rt==NONE))
  {
    if (!errorreported) WerrorS("right side is not a datum");
    return TRUE;
  }

  int i=0;
  BOOLEAN nok=FALSE;

  if (lt==DEF_CMD)
  {
    if (l->rtyp==IDHDL)
    {
      IDTYP((idhdl)l->data)=rt;
    }
    else if (l->name!=NULL)
    {
      sleftv ll;
      iiDeclCommand(&ll,l,myynest,rt,&idroot);
      memcpy(l,&ll,sizeof(sleftv));
    }
    else
    {
      l->rtyp=rt;
    }
    lt=rt;
  }
  else
  {
    if ((l->data==r->data)&&(l->e==NULL)&&(r->e==NULL))
      return FALSE;
  }
  leftv ld=l;
  if ((l->rtyp==IDHDL)&&(lt!=QRING_CMD)&&(lt!=RING_CMD))
    ld=(leftv)l->data;
  while (((dAssign[i].res!=lt)
      || (dAssign[i].arg!=rt))
    && (dAssign[i].res!=0)) i++;
  if (dAssign[i].res!=0)
  {
    BOOLEAN b;
    b=dAssign[i].p(ld,r,l->e);
    if(l!=ld) /* i.e. l is IDHDL, l->data is ld */
    {
      l->flag=ld->flag;
      l->attribute=ld->attribute;
    }
    return b;
  }
  // implicite type conversion ----------------------------------------------
  if (dAssign[i].res==0)
  {
    int ri;
    leftv rn = (leftv)Alloc0(sizeof(sleftv));
    BOOLEAN failed=FALSE;
    i=0;
    while ((dAssign[i].res!=lt)
      && (dAssign[i].res!=0)) i++;
    while (dAssign[i].res==lt)
    {
      if ((ri=iiTestConvert(rt,dAssign[i].arg))!=0)
      {
        failed= iiConvert(rt,dAssign[i].arg,ri,r,rn);
        if(!failed)
        {
          failed= dAssign[i].p(ld,rn,l->e);
        }
        // everything done, clean up temp. variables
        rn->CleanUp();
        Free((ADDRESS)rn,sizeof(sleftv));
        if (failed)
        {
          // leave loop, goto error handling
          break;
        }
        else
        {
          if(l!=ld) /* i.e. l is IDHDL, l->data is ld */
          {
            l->flag=ld->flag;
            l->attribute=ld->attribute;
          }
          // everything ok, return
          return FALSE;
        }
     }
     i++;
    }
    // error handling ---------------------------------------------------
    if (!errorreported)
    {
      Werror("`%s` = `%s` is not supported"
             ,Tok2Cmdname(lt),Tok2Cmdname(rt));
      if (BVERBOSE(V_SHOW_USE))
      {
        i=0;
        while ((dAssign[i].res!=lt)
          && (dAssign[i].res!=0)) i++;
        while (dAssign[i].res==lt)
        {
          Werror("expected `%s` = `%s`"
              ,Tok2Cmdname(lt),Tok2Cmdname(dAssign[i].arg));
          i++;
        }
      }
    }
  }
  return TRUE;
}
/*2
* assign sys_var = val
*/
static BOOLEAN iiAssign_sys(leftv l, leftv r)
{
  int rt=r->Typ();

  if (rt==0)
  {
    if (!errorreported) Werror("`%s` is undefined",r->Name());
    return TRUE;
  }
  int i=0;
  int lt=l->rtyp;
  while (((dAssign_sys[i].res!=lt)
      || (dAssign_sys[i].arg!=rt))
    && (dAssign_sys[i].res!=0)) i++;
  if (dAssign_sys[i].res!=0)
  {
    if (!dAssign_sys[i].p(l,r))
    {
      // everything ok, clean up
      return FALSE;
    }
  }
  // implicite type conversion ----------------------------------------------
  if (dAssign_sys[i].res==0)
  {
    int ri;
    leftv rn = (leftv)Alloc0(sizeof(sleftv));
    BOOLEAN failed=FALSE;
    i=0;
    while ((dAssign_sys[i].res!=lt)
      && (dAssign_sys[i].res!=0)) i++;
    while (dAssign_sys[i].res==lt)
    {
      if ((ri=iiTestConvert(rt,dAssign_sys[i].arg))!=0)
      {
        failed= ((iiConvert(rt,dAssign_sys[i].arg,ri,r,rn))
            || (dAssign_sys[i].p(l,rn)));
        // everything done, clean up temp. variables
        rn->CleanUp();
        Free((ADDRESS)rn,sizeof(sleftv));
        if (failed)
        {
          // leave loop, goto error handling
          break;
        }
        else
        {
          // everything ok, return
          return FALSE;
        }
     }
     i++;
    }
    // error handling ---------------------------------------------------
    if(!errorreported)
    {
      Werror("`%s` = `%s` is not supported"
             ,Tok2Cmdname(lt),Tok2Cmdname(rt));
      if (BVERBOSE(V_SHOW_USE))
      {
        i=0;
        while ((dAssign_sys[i].res!=lt)
          && (dAssign_sys[i].res!=0)) i++;
        while (dAssign_sys[i].res==lt)
        {
          Werror("expected `%s` = `%s`"
              ,Tok2Cmdname(lt),Tok2Cmdname(dAssign_sys[i].arg));
          i++;
        }
      }
    }
  }
  return TRUE;
}
static BOOLEAN jiA_INTVEC_L(leftv l,leftv r)
{
  /* right side is intvec, left side is list (of int)*/
  BOOLEAN nok;
  int i=0;
  leftv l1=l;
  leftv h;
  sleftv t;
  intvec *iv=(intvec *)r->Data();
  memset(&t,0,sizeof(sleftv));
  t.rtyp=INT_CMD;
  while ((i<iv->length())&&(l!=NULL))
  {
    t.data=(char *)(*iv)[i];
    h=l->next;
    l->next=NULL;
    nok=jiAssign_1(l,&t);
    if (nok) return TRUE;
    i++;
    l=h;
  }
  l1->CleanUp();
  r->CleanUp();
  return FALSE;
}
static BOOLEAN jiA_VECTOR_L(leftv l,leftv r)
{
  /* right side is vector, left side is list (of poly)*/
  BOOLEAN nok;
  leftv l1=l;
  ideal I=idVec2Ideal((poly)r->Data());
  leftv h;
  sleftv t;
  int i=0;
  while (l!=NULL)
  {
    memset(&t,0,sizeof(sleftv));
    t.rtyp=POLY_CMD;
    if (i>=IDELEMS(I))
    {
      t.data=NULL;
    }
    else
    {
      t.data=(char *)I->m[i];
      I->m[i]=NULL;
    }
    h=l->next;
    l->next=NULL;
    nok=jiAssign_1(l,&t);
    t.CleanUp();
    if (nok)
    {
      idDelete(&I);
      return TRUE;
    }
    i++;
    l=h;
  }
  idDelete(&I);
  l1->CleanUp();
  r->CleanUp();
  return FALSE;
}
static BOOLEAN jjA_L_LIST(leftv l, leftv r)
/* left side: list
*  right side: expression list
*/
{
  int sl = r->listLength();
  lists L=(lists)Alloc(sizeof(slists));
  leftv h=NULL,o_r=r;
  int i;
  int rt;

  L->Init(sl);
  for (i=0;i<sl;i++)
  {
    if (h!=NULL) { /* e.g. not in the first step:
                   * h is the pointer to the old sleftv,
                   * r is the pointer to the next sleftv
                   * (in this moment) */
                   h->next=r;
                 }
    h=r;
    r=r->next;
    h->next=NULL;
    rt=h->Typ();
    if ((rt==0)||(rt==NONE))
    {
      L->Clean();
      Werror("`%s` is undefined",h->Name());
      return TRUE;
    }
    //if ((rt==RING_CMD)||(rt==QRING_CMD))
    //{
    //  L->m[i].rtyp=rt;
    //  L->m[i].data=h->Data();
    //  ((ring)L->m[i].data)->ref++;
    //}
    //else
      L->m[i].Copy(h);
  }
  IDLIST((idhdl)l->data)->Clean();
  IDLIST((idhdl)l->data)=L;
  ipMoveId((idhdl)l->data);
  o_r->CleanUp();
  return FALSE;
}
static BOOLEAN jiA_L_LIST(leftv l, leftv r)
/* left side: list
*  right side: expression list
*/
{
  int sl = r->listLength();
  lists L=(lists)Alloc(sizeof(slists));
  leftv h=NULL;
  int i;
  int rt;

  L->Init(sl);
  for (i=0;i<sl;i++)
  {
    if (h!=NULL) { /* e.g. not in the first step:
                   * h is the pointer to the old sleftv,
                   * r is the pointer to the next sleftv
                   * (in this moment) */
                   h->next=r;
                 }
    h=r;
    r=r->next;
    h->next=NULL;
    rt=h->Typ();
    if (rt==0)
    {
      L->Clean();
      Werror("`%s` is undefined",h->Name());
      return TRUE;
    }
    if ((rt==RING_CMD)||(rt==QRING_CMD))
    {
      L->m[i].rtyp=rt;
      L->m[i].data=h->Data();
      ((ring)L->m[i].data)->ref++;
    }
    else
      L->m[i].Copy(h);
  }
  IDLIST((idhdl)l->data)->Clean();
  IDLIST((idhdl)l->data)=L;
  return FALSE;
}
static BOOLEAN jjA_L_INTVEC(leftv l,leftv r,intvec *iv)
{
  /* left side is intvec/intmat, right side is list (of int,intvec,intmat)*/
  leftv hh=r;
  int i = 0;
  while (hh!=NULL)
  {
    if (i>=iv->length()) break;
    if (hh->Typ() == INT_CMD)
    {
      (*iv)[i++] = (int)(hh->Data());
    }
    else if ((hh->Typ() == INTVEC_CMD)
            ||(hh->Typ() == INTMAT_CMD))
    {
      intvec *ivv = (intvec *)(hh->Data());
      int ll = 0,l = min(ivv->length(),iv->length());
      for (; l>0; l--)
      {
        (*iv)[i++] = (*ivv)[ll++];
      }
    }
    else
    {
      delete iv;
      return TRUE;
    }
    hh = hh->next;
  }
  if (IDINTVEC((idhdl)l->data)!=NULL) delete IDINTVEC((idhdl)l->data);
  IDINTVEC((idhdl)l->data)=iv;
  return FALSE;
}
static BOOLEAN jjA_L_STRING(leftv l,leftv r)
{
  /* left side is string, right side is list of string*/
  leftv hh=r;
  int sl = 1;
  char *s;
  char *t;
  int tl;
  /* find the length */
  while (hh!=NULL)
  {
    if (hh->Typ()!= STRING_CMD)
    {
      return TRUE;
    }
    sl += strlen((char *)hh->Data());
    hh = hh->next;
  }
  s = (char * )AllocL(sl);
  sl=0;
  hh = r;
  while (hh!=NULL)
  {
    t=(char *)hh->Data();
    tl=strlen(t);
    memcpy(s+sl,t,tl);
    sl+=tl;
    hh = hh->next;
  }
  s[sl]='\0';
  FreeL((ADDRESS)IDDATA((idhdl)(l->data)));
  IDDATA((idhdl)(l->data))=s;
  return FALSE;
}
static BOOLEAN jjA_LIST_L(leftv l,leftv r)
{
  /*left side are something, right side are lists*/
  /*e.g. a,b,c=l */
  //int ll=l->listLength();
  if (l->listLength()==1) return jiAssign_1(l,r);
  BOOLEAN nok;
  sleftv t;
  leftv h;
  lists L=(lists)r->Data();
  int rl=L->nr;
  int i=0;

  memset(&t,0,sizeof(sleftv));
  while ((i<=rl)&&(l!=NULL))
  {
    memset(&t,0,sizeof(sleftv));
    t.Copy(&L->m[i]);
    h=l->next;
    l->next=NULL;
    nok=jiAssign_1(l,&t);
    if (nok) return TRUE;
    i++;
    l=h;
  }
  r->CleanUp();
  return FALSE;
}
static BOOLEAN jiA_MATRIX_L(leftv l,leftv r)
{
  /* right side is matrix, left side is list (of poly)*/
  BOOLEAN nok=FALSE;
  int i;
  matrix m=(matrix)r->CopyD(MATRIX_CMD);
  leftv h;
  leftv ol=l;
  leftv o_r=r;
  sleftv t;
  memset(&t,0,sizeof(sleftv));
  t.rtyp=POLY_CMD;
  loop
  {
    i=0;
    while ((i<MATROWS(m)*MATCOLS(m))&&(l!=NULL))
    {
      t.data=(char *)m->m[i];
      m->m[i]=NULL;
      h=l->next;
      l->next=NULL;
      nok=jiAssign_1(l,&t);
      l->next=h;
      if (nok)
      {
        idDelete((ideal *)&m);
        goto ende;
      }
      i++;
      l=h;
    }
    idDelete((ideal *)&m);
    h=r;
    r=r->next;
    if (l==NULL)
    {
      if (r!=NULL)
      {
        Warn("list length mismatch in assign (l>r)");
        nok=TRUE;
      }
      break;
    }
    else if (r==NULL)
    {
      Warn("list length mismatch in assign (l<r)");
      nok=TRUE;
      break;
    }
    if ((r->Typ()==IDEAL_CMD)||(r->Typ()==MATRIX_CMD))
      m=(matrix)r->CopyD(MATRIX_CMD);
    else if (r->Typ()==POLY_CMD)
    {
      m=mpNew(1,1);
      MATELEM(m,1,1)=(poly)r->CopyD(POLY_CMD);
    }
    else
    {
      nok=TRUE;
      break;
    }
  }
ende:
  o_r->CleanUp();
  ol->CleanUp();
  return nok;
}
static BOOLEAN jiA_STRING_L(leftv l,leftv r)
{
  /*left side are strings, right side is a string*/
  /*e.g. s[2..3]="12" */
  /*the case s=t[1..4] is handled in iiAssign,
  * the case s[2..3]=t[3..4] is handled in iiAssgn_rec*/
  int ll=l->listLength();
  int rl=r->listLength();
  BOOLEAN nok=FALSE;
  sleftv t;
  leftv h,l1=l;
  int i=0;
  char *ss;
  char *s=(char *)r->Data();
  int sl=strlen(s);

  memset(&t,0,sizeof(sleftv));
  t.rtyp=STRING_CMD;
  while ((i<sl)&&(l!=NULL))
  {
    ss=(char *)AllocL(2);
    ss[1]='\0';
    ss[0]=s[i];
    t.data=ss;
    h=l->next;
    l->next=NULL;
    nok=jiAssign_1(l,&t);
    if (nok)
    {
      break;
    }
    i++;
    l=h;
  }
  r->CleanUp();
  l1->CleanUp();
  return nok;
}
static BOOLEAN jiAssign_list(leftv l, leftv r)
{
  int i=l->e->start-1;
  if (i<0)
  {
    Werror("index[%d] must be positive",i+1);
    return TRUE;
  }
  if(l->attribute!=NULL)
  {
    atKillAll((idhdl)l);
    l->attribute=NULL;
  }
  l->flag=0;
  lists li;
  if (l->rtyp==IDHDL)
  {
    li=IDLIST((idhdl)l->data);
  }
  else
  {
    li=(lists)l->data;
  }
  if (i>li->nr)
  {
    li->m=(leftv)ReAlloc(li->m,(li->nr+1)*sizeof(sleftv),(i+1)*sizeof(sleftv));
    memset(&(li->m[li->nr+1]),0,(i-li->nr)*sizeof(sleftv));
    int j=li->nr+1;
    for(;j<=i;j++)
      li->m[j].rtyp=DEF_CMD;
    li->nr=i;
  }
  leftv ld=&(li->m[i]);
  ld->e=l->e->next;
  BOOLEAN b;
  if ((ld->rtyp!=LIST_CMD)
  &&(ld->e==NULL)
  &&(ld->Typ()!=r->Typ()))
  {
    sleftv tmp;
    memset(&tmp,0,sizeof(sleftv));
    tmp.rtyp=DEF_CMD;
    b=iiAssign(&tmp,r);
    ld->CleanUp();
    memcpy(ld,&tmp,sizeof(sleftv));
  }
  else
  {
    b=iiAssign(ld,r);
    l->e->next=ld->e;
  }
  return b;
}
static BOOLEAN jiAssign_rec(leftv l, leftv r)
{
  leftv l1=l;
  leftv r1=r;
  leftv lrest;
  leftv rrest;
  BOOLEAN b;
  do
  {
    lrest=l->next;
    rrest=r->next;
    l->next=NULL;
    r->next=NULL;
    b=iiAssign(l,r);
    l->next=lrest;
    r->next=rrest;
    l=lrest;
    r=rrest;
  } while  ((!b)&&(l!=NULL));
  l1->CleanUp();
  r1->CleanUp();
  return b;
}
BOOLEAN iiAssign(leftv l, leftv r)
{
  int ll=l->listLength();
  int rl;
  int lt=l->Typ();
  int rt=NONE;
  BOOLEAN b;

  if(l->attribute!=NULL)
  {
    if (l->rtyp==IDHDL)
    {
      atKillAll((idhdl)l->data);
      l->attribute=NULL;
    }
    else
      atKillAll((idhdl)l);
  }
  if(l->rtyp==IDHDL)
  {
    IDFLAG((idhdl)l->data)=0;
  }
  l->flag=0;
  if (ll==1)
  {
    /* l[..] = ... */
    if((l->e!=NULL)
    && (((l->rtyp==IDHDL) && (IDTYP((idhdl)l->data)==LIST_CMD))
      || (l->rtyp==LIST_CMD)))
    {
       if(r->next!=NULL)
         b=jiA_L_LIST(l,r);
       else
         b=jiAssign_list(l,r);
       if((l->rtyp==IDHDL) && (l->data!=NULL))
       {
         ipMoveId((idhdl)l->data);
         l->attribute=IDATTR((idhdl)l->data);
         l->flag=IDFLAG((idhdl)l->data);
       }
       r->CleanUp();
       Subexpr h;
       while (l->e!=NULL)
       {
         h=l->e->next;
         Free((ADDRESS)l->e,sizeof(*(l->e)));
         l->e=h;
       }
       return b;
    }
    rl=r->listLength();
    if (rl==1)
    {
      /* system variables = ... */
      if(((l->rtyp>=VECHO)&&(l->rtyp<=VPRINTLEVEL))
      ||((l->rtyp>=VALTVARS)&&(l->rtyp<=VMINPOLY)))
      {
        b=iiAssign_sys(l,r);
        r->CleanUp();
        //l->CleanUp();
        return b;
      }
      rt=r->Typ();
      /* a = ... */
      if ((lt!=MATRIX_CMD)
      &&(lt!=INTMAT_CMD)
      &&((lt==rt)||(lt!=LIST_CMD)))
      {
        b=jiAssign_1(l,r);
        if (l->rtyp==IDHDL)
        {
          if ((lt==DEF_CMD)||(lt=LIST_CMD)) ipMoveId((idhdl)l->data);
          l->attribute=IDATTR((idhdl)l->data);
          l->flag=IDFLAG((idhdl)l->data);
          l->CleanUp();
        }
        r->CleanUp();
        return b;
      }
      if (((lt!=LIST_CMD)
        &&((rt==MATRIX_CMD)
          ||(rt==INTMAT_CMD)
          ||(rt==INTVEC_CMD)
          ||(rt==MODUL_CMD)))
      ||((lt==LIST_CMD)
        &&(rt==RESOLUTION_CMD))
      )
      {
        b=jiAssign_1(l,r);
        if((l->rtyp==IDHDL)&&(l->data!=NULL))
        {
          if (lt==DEF_CMD) ipMoveId((idhdl)l->data);
          l->attribute=IDATTR((idhdl)l->data);
          l->flag=IDFLAG((idhdl)l->data);
        }
        r->CleanUp();
        Subexpr h;
        while (l->e!=NULL)
        {
          h=l->e->next;
          Free((ADDRESS)l->e,sizeof(*(l->e)));
          l->e=h;
        }
        return b;
      }
    }
    if (rt==NONE) rt=r->Typ();
  }
  else if (ll==(rl=r->listLength()))
  {
    b=jiAssign_rec(l,r);
    return b;
  }
  else
  {
    if (rt==NONE) rt=r->Typ();
    if (rt==INTVEC_CMD)
      return jiA_INTVEC_L(l,r);
    else if (rt==VECTOR_CMD)
      return jiA_VECTOR_L(l,r);
    else if ((rt==IDEAL_CMD)||(rt==MATRIX_CMD))
      return jiA_MATRIX_L(l,r);
    else if ((rt==STRING_CMD)&&(rl==1))
      return jiA_STRING_L(l,r);
    Werror("length of lists in assignment does not match (l:%d,r:%d)",
      ll,rl);
    return TRUE;
  }

  leftv hh=r;
  BOOLEAN nok=FALSE;
  BOOLEAN map_assign=FALSE;
  switch (lt)
  {
    case INTVEC_CMD:
      nok=jjA_L_INTVEC(l,r,new intvec(exprlist_length(r)));
      break;
    case INTMAT_CMD:
    {
      nok=jjA_L_INTVEC(l,r,new intvec(IDINTVEC((idhdl)l->data)));
      break;
    }
    case MAP_CMD:
    {
      // first element in the list sl (r) must be a ring
      if (((rt == RING_CMD)||(rt == QRING_CMD))&&(r->e==NULL))
      {
        FreeL((ADDRESS)IDMAP((idhdl)l->data)->preimage);
        IDMAP((idhdl)l->data)->preimage = mstrdup (r->Name());
        /* advance the expressionlist to get the next element after the ring */
        hh = r->next;
        //r=hh;
      }
      else
      {
        WerrorS("expected ring-name");
        nok=TRUE;
        break;
      }
      if (hh==NULL) /* map-assign: map f=r; */
      {
        WerrorS("expected image ideal");
        nok=TRUE;
        break;
      }
      if ((hh->next==NULL)&&(hh->Typ()==IDEAL_CMD))
        return jiAssign_1(l,hh); /* map-assign: map f=r,i; */
      //no break, handle the rest like an ideal:
      map_assign=TRUE;
    }
    case MATRIX_CMD:
    case IDEAL_CMD:
    case MODUL_CMD:
    {
      sleftv t;
      matrix olm = (matrix)l->Data();
      int rk=olm->rank;
      char *pr=((map)olm)->preimage;
      BOOLEAN module_assign=(/*l->Typ()*/ lt==MODUL_CMD);
      matrix lm ;
      matrix rm;
      int  num;
      int j,k;
      int i=0;
      int mtyp=MATRIX_CMD; /*Type of left side object*/
      int etyp=POLY_CMD;   /*Type of elements of left side object*/

      if (lt /*l->Typ()*/==MATRIX_CMD)
      {
        num=olm->cols()*olm->rows();
        lm=mpNew(olm->rows(),olm->cols());
      }
      else /* IDEAL_CMD or MODUL_CMD */
      {
        num=exprlist_length(hh);
        lm=(matrix)idInit(num,1);
        rk=1;
        if (module_assign)
        {
          mtyp=MODUL_CMD;
          etyp=VECTOR_CMD;
        }
      }

      int ht;
      loop
      {
        if (hh==NULL)
          break;
        else
        {
          ht=hh->Typ();
          if ((j=iiTestConvert(ht,etyp))!=0)
          {
            nok=iiConvert(ht,etyp,j,hh,&t);
            hh->next=t.next;
            if (nok) break;
            lm->m[i]=(poly)t.CopyD(etyp);
            if (module_assign) rk=max(rk,pMaxComp(lm->m[i]));
            i++;
          }
          else
          if ((j=iiTestConvert(ht,mtyp))!=0)
          {
            nok=iiConvert(ht,mtyp,j,hh,&t);
            hh->next=t.next;
            if (nok) break;
            rm = (matrix)t.CopyD(mtyp);
            if (module_assign)
            {
              j = min(num,rm->cols());
              rk=max(rk,rm->rank);
            }
            else
              j = min(num-i,rm->rows() * rm->cols());
            for(k=0;k<j;k++,i++)
            {
              lm->m[i]=rm->m[k];
              rm->m[k]=NULL;
            }
            idDelete((ideal *)&rm);
          }
          else
          {
            nok=TRUE;
            break;
          }
          t.next=NULL;t.CleanUp();
          if (i==num) break;
          hh=hh->next;
        }
      }
      if (nok)
        idDelete((ideal *)&lm);
      else
      {
        idDelete((ideal *)&olm);
        if (module_assign)   lm->rank=rk;
        else if (map_assign) ((map)lm)->preimage=pr;
#ifdef DRING
        else if (pDRING)
        {
          int i=IDELEMS(lm)-1;
          while (i>=0)
          {
            pdSetDFlag(lm->m[i],1);
            i--;
          }
        }
#endif
        l=l->LData();
        if (l->rtyp==IDHDL)
          IDMATRIX((idhdl)l->data)=lm;
        else
          l->data=(char *)lm;
      }
      break;
    }
    case STRING_CMD:
      nok=jjA_L_STRING(l,r);
      break;
    case LIST_CMD:
      nok=jjA_L_LIST(l,r);
      break;
    case NONE:
    case 0:
      Werror("cannot assign to %s",l->Name());
      nok=TRUE;
      break;
    default:
      WerrorS("assign not impl.");
      nok=TRUE;
      break;
  } /* end switch: typ */
  if (nok && (!errorreported)) WerrorS("incompatible type in list assignment");
  r->CleanUp();
  return nok;
}
