/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
static char rcsid[] = "$Header: /exports/cvsroot-2/cvsroot/Singular/ipid.cc,v 1.1.1.1 1997-03-19 13:18:42 obachman Exp $";
/* $Log: not supported by cvs2svn $
*/

/*
* ABSTRACT: identfier handling
*/

#include <string.h>

#include "mod2.h"
#include "tok.h"
#include "ipshell.h"
#include "intvec.h"
#include "febase.h"
#include "mmemory.h"
#include "numbers.h"
#include "polys.h"
#include "ring.h"
#include "ideals.h"
#include "matpol.h"
#include "lists.h"
#include "attrib.h"
#include "silink.h"
#include "ipid.h"

idhdl idroot = NULL;
idhdl currRingHdl = NULL;
ring  currRing = NULL;
ideal currQuotient = NULL;
char* iiNoName="_";

/*0 implementation*/

idhdl idrec::get(const char * s, int lev)
{
  idhdl h = this;
  idhdl found=NULL;
  int l;
  while (h!=NULL)
  {
    l=IDLEV(h);
    mmTestLP(IDID(h));
    if ((l==lev) && (0 == strcmp(s,IDID(h)))) return h;
    if ((l==0) && (found==NULL) && (0 == strcmp(s,IDID(h))))
    {
      found=h;
    }
    h = IDNEXT(h);
  }
  return found;
}

//idrec::~idrec()
//{
//  if (id!=NULL)
//  {
//    FreeL((ADDRESS)id);
//    id=NULL;
//  }
//  /* much more !! */
//}

idhdl idrec::set(char * s, int lev, idtyp t, BOOLEAN init)
{
  //printf("define %s, %x, lev: %d, typ: %d\n", s,s,lev,t);
  idhdl h = (idrec *)Alloc0(sizeof(idrec));
  int   len = 0;
  IDID(h)   = s;
  IDTYP(h)  = t;
  IDLEV(h)  = lev;
  IDNEXT(h) = this;
  if (init)
  {
    switch (t)
    {
    //the type with init routines:
      case INTVEC_CMD:
      case INTMAT_CMD:
        IDINTVEC(h) = new intvec();
        break;
      case NUMBER_CMD:
        IDNUMBER(h) = nInit(0);
        break;
      case IDEAL_CMD:
      case MODUL_CMD:
        IDFLAG(h) = Sy_bit(FLAG_STD);
      case MATRIX_CMD:
        IDIDEAL(h) = idInit(1,1);
        break;
      case MAP_CMD:
        IDIDEAL(h) = idInit(1,1);
        IDMAP(h)->preimage = mstrdup(IDID(currRingHdl));
        break;
      case STRING_CMD:
      #ifdef HAVE_DLD
      case BINARY_CMD:
      #endif
        IDSTRING(h) = mstrdup("");
        break;
      case LIST_CMD:
        IDLIST(h)=(lists)Alloc(sizeof(slists));
        IDLIST(h)->Init();
        break;
    //the types with the standard init: set the struct to zero
      case LINK_CMD:
        len=sizeof(ip_link);
        break;
      case RING_CMD:
      case QRING_CMD:
        len = sizeof(ip_sring);
        break;
      case PACKAGE_CMD:
        len = sizeof(ip_package);
        break;
    //other types: without init (int,proc,script,poly,def,package)
    }
    if (len!=0)
    {
      IDSTRING(h) = (char *)Alloc0(len);
    }
  }
  return  h;
}

idhdl enterid(char * s, int lev, idtyp t, idhdl* root, BOOLEAN init)
{
  idhdl h;
  // is it the currRing - id ?
  if ((currRingHdl!=NULL)
  &&(IDLEV(currRingHdl)!=lev)
  &&(s==IDID(currRingHdl)))
  {
    s=mstrdup(s);
  }
  // is it already defined in root ?
  else if ((h=(*root)->get(s,lev))!=NULL)
  {
    if (IDLEV(h)!=lev)
    {
      s=mstrdup(s);
    }
    else if ((IDTYP(h) == t)||(t==DEF_CMD))
    {
      if (BVERBOSE(V_REDEFINE))
        Warn("redefining %s **",s);
      if (s==IDID(h))
        IDID(h)=NULL;
      killhdl(h,root);
    }
    else
      goto errlabel;
  }
  // is it already defined in idroot ?
  else if (*root != idroot)
  {
    if ((h=idroot->get(s,lev))!=NULL)
    {
      if (IDLEV(h)!=lev)
      {
        s=mstrdup(s);
      }
      else if ((IDTYP(h) == t)||(t==DEF_CMD))
      {
        if (BVERBOSE(V_REDEFINE))
          Warn("redefining %s **",s);
        IDID(h)=NULL;
        killhdl(h,&idroot);
      }
      else
      {
        goto errlabel;
      }
    }
  }
  // is it already defined in currRing->idroot ?
  else if ((currRing!=NULL)&&((*root) != currRing->idroot))
  {
    if ((h=currRing->idroot->get(s,lev))!=NULL)
    {
      if (IDLEV(h)!=lev)
      {
        s=mstrdup(s);
      }
      else if ((IDTYP(h) == t)||(t==DEF_CMD))
      {
        if (BVERBOSE(V_REDEFINE))
          Warn("redefining %s **",s);
        IDID(h)=NULL;
        killhdl(h,&currRing->idroot);
      }
      else
      {
        goto errlabel;
      }
    }
  }
  return *root = (*root)->set(s, lev, t, init);

  errlabel:
    Werror("identifier `%s` in use",s);
    return NULL;
}

void killid(char * id, idhdl * ih)
{
  if (id!=NULL)
  {
    idhdl h = (*ih)->get(id,myynest);

    // id not found in global list, is it defined in current ring ?
    if (h==NULL)
    {
      if ((currRing!=NULL) && (*ih != (currRing->idroot)))
      {
        h = currRing->idroot->get(id,myynest);
        if (h!=NULL)
        {
          killhdl(h,&(currRing->idroot));
          return;
        }
      }
      Werror("`%s` is not defined",id);
      return;
    }
    killhdl(h,ih);
  }
  else
    Werror("kill what ?");
}

void killhdl(idhdl h)
{
  int t=IDTYP(h);
  if ((BEGIN_RING<t) && (t<END_RING) && (t!=QRING_CMD))
    killhdl(h,&currRing->idroot);
  else
  {
    idhdl s=idroot;
    while ((s!=h) && (s!=NULL)) s=s->next;
    if (s==NULL) killhdl(h,&currRing->idroot);
    else killhdl(h,&idroot);
  }
}

void killhdl(idhdl h, idhdl * ih)
{
  //printf("kill %s, id %x, typ %d lev: %d\n",IDID(h),(int)IDID(h),IDTYP(h),IDLEV(h));
  idhdl hh;
  BOOLEAN killOtherRing = TRUE;
  BOOLEAN needResetRing = FALSE;

  if (h->attribute!=NULL)
  {
    atKillAll(h);
    //h->attribute=NULL;
  }
  // ring / qring  --------------------------------------------------------
  // package  -------------------------------------------------------------
  if ((IDTYP(h) == RING_CMD) || (IDTYP(h) == QRING_CMD)
  || (IDTYP(h) == PACKAGE_CMD) )
  {
    idhdl savecurrRingHdl = currRingHdl;
    ring  savecurrRing = currRing;

    // any objects defined for this ring ?
    if (((IDTYP(h)==PACKAGE_CMD) || (IDRING(h)->ref<=0))
    &&  (IDRING(h)->idroot!=NULL))
    {
      idhdl * hd = &IDRING(h)->idroot;
      idhdl  hdh = IDNEXT(*hd);
      idhdl  temp;
      killOtherRing=(IDTYP(h)!=PACKAGE_CMD) && (IDRING(h)!=currRing);
      if (killOtherRing) //we are not killing the base ring, so switch
      {
        needResetRing=TRUE;
        rSetHdl(h,FALSE);
        /* no complete init*/
      }
      while (hdh!=NULL)
      {
        temp = IDNEXT(hdh);
        killhdl(hdh,&(IDRING(h)->idroot));
        hdh = temp;
      }
      killhdl(*hd,hd);
    }
    // reset currRing ?
    if (needResetRing) // && (killOtherRing)
    {
      //we have to switch back to the base ring
      //currRing = savecurrRing;
      //currRingHdl = savecurrRingHdl;
      if (savecurrRing!=NULL)
      {
        rSetHdl(savecurrRingHdl,TRUE);
      }
    }
    rKill(h);
  }
  // poly / vector -------------------------------------------------------
  else if ((IDTYP(h) == POLY_CMD) || (IDTYP(h) == VECTOR_CMD))
  {
    pDelete(&IDPOLY(h));
  }
  // ideal / module/ matrix / map ----------------------------------------
  else if ((IDTYP(h) == IDEAL_CMD)
           || (IDTYP(h) == MODUL_CMD)
           || (IDTYP(h) == MATRIX_CMD)
           || (IDTYP(h) == MAP_CMD))
  {
    ideal iid = IDIDEAL(h);
    if (IDTYP(h) == MAP_CMD)
    {
      map im = IDMAP(h);
      FreeL((ADDRESS)im->preimage);
    }
    idDelete(&iid);
  }
  // string / proc / binary ------------------------------------------------
  else if ((IDTYP(h) == STRING_CMD)
           ||(IDTYP(h) == PROC_CMD)
      #ifdef HAVE_DLD
           ||(IDTYP(h) == BINARY_CMD)
      #endif
      )
  {
    FreeL((ADDRESS)IDSTRING(h));
    //IDSTRING(h)=NULL;
  }
  // number -------------------------------------------------------------
  else if (IDTYP(h) == NUMBER_CMD)
  {
    nDelete(&IDNUMBER(h));
  }
  // intvec / intmat  ---------------------------------------------------
  else if ((IDTYP(h) == INTVEC_CMD)||(IDTYP(h) == INTMAT_CMD))
  {
    delete IDINTVEC(h);
  }
  // list  -------------------------------------------------------------
  else if (IDTYP(h)==LIST_CMD)
  {
    IDLIST(h)->Clean();
    //Free((ADDRESS)IDLIST(h)->m, (IDLIST(h)->nr+1)*sizeof(sleftv));
    //Free((ADDRESS)IDLIST(h), sizeof(slists));
  }
  // link  -------------------------------------------------------------
  else if (IDTYP(h)==LINK_CMD)
  {
    slKill(IDLINK(h));
    Free((ADDRESS)IDLINK(h),sizeof(ip_link));
  }
#ifdef TEST
  else if ((IDTYP(h)!= INT_CMD)&&(IDTYP(h)!=DEF_CMD))
    Warn("unknown type to kill: %s(%d)",Tok2Cmdname(IDTYP(h)),IDTYP(h));
#endif

  //  general  -------------------------------------------------------------
  // now dechain it and delete idrec
  FreeL((ADDRESS)IDID(h));
  //IDID(h)=NULL;
  if (h == (*ih))
  {
    // h is at the beginning of the list
    *ih = IDNEXT(*ih);
  }
  else
  {
    // h is somethere in the list:
    hh = *ih;
    loop
    {
      idhdl hhh = IDNEXT(hh);
      if (hhh == h)
      {
        IDNEXT(hh) = IDNEXT(hhh);
        break;
      }
      hh = hhh;
    }
  }
  Free((ADDRESS)h,sizeof(idrec));
}

idhdl ggetid(const char *n)
{
  idhdl h = idroot->get(n,myynest);
  idhdl h2=NULL;
  if ((currRing!=NULL) && ((h==NULL)||(IDLEV(h)!=myynest)))
  {
    h2 = currRing->idroot->get(n,myynest);
  }
  if (h2==NULL) return h;
  return h2;
}

void ipListFlag(idhdl h)
{
  if (hasFlag(h,FLAG_STD)) PrintS(" (SB)");
  if (hasFlag(h,FLAG_DRING)) PrintS(" (D)");
}

lists ipNameList(idhdl root)
{
  idhdl h=root;
  /* compute the length */
  int l=0;
  while (h!=NULL) { l++; h=IDNEXT(h); }
  /* allocate list */
  lists L=(lists)Alloc(sizeof(slists));
  L->Init(l);
  /* copy names */
  h=root;
  l=0;
  while (h!=NULL)
  {
    /* list is initialized with 0 => no need to clear anything */
    L->m[l].rtyp=STRING_CMD;
    L->m[l].data=mstrdup(IDID(h));
    l++;
    h=IDNEXT(h);
  }
  return L;
}

/*
* move 'tomove' from root1 list to root2 list
*/
static void ipSwapId(idhdl tomove, idhdl &root1, idhdl &root2)
{
  idhdl h;
  /* search 'tomove' in root2 : if found -> do nothing */
  h=root2;
  while ((h!=NULL) && (h!=tomove)) h=IDNEXT(h);
  if (h!=NULL) return;
  /* search predecessor of h in root1, remove 'tomove' */
  h=root1;
  if (tomove==h)
  {
    root1=IDNEXT(h);
  }
  else
  {
    while ((h!=NULL) && (IDNEXT(h)!=tomove)) h=IDNEXT(h);
    if (h==NULL) return; /* not in the list root1 -> do nothing */
    IDNEXT(h)=IDNEXT(tomove);
  }
  /* add to root2 list */
  IDNEXT(tomove)=root2;
  root2=tomove;
}

void  ipMoveId(idhdl tomove)
{
  if ((currRing!=NULL)&&(tomove!=NULL))
  {
    if (((QRING_CMD!=IDTYP(tomove)) && (BEGIN_RING<IDTYP(tomove)) && (IDTYP(tomove)<END_RING))
    || ((IDTYP(tomove)==LIST_CMD) && (lRingDependend(IDLIST(tomove)))))
    {
      /*move 'tomove' to ring id's*/
      ipSwapId(tomove,idroot,currRing->idroot);
    }
    else
    {
      /*move 'tomove' to global id's*/
      ipSwapId(tomove,currRing->idroot,idroot);
    }
  }
}
