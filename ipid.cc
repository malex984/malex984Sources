/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/* $Id$ */

/*
* ABSTRACT: identfier handling
*/

#include <string.h>

#include "mod2.h"
#include "static.h"
#include "omalloc.h"
#include "tok.h"
#include "options.h"
#include "ipshell.h"
#include "intvec.h"
#include "febase.h"
#include "numbers.h"
#include "longrat.h"
#include "polys.h"
#include "ring.h"
#include "ideals.h"
#include "matpol.h"
#include "lists.h"
#include "attrib.h"
#include "silink.h"
#include "syz.h"
#include "ipid.h"

#ifdef HAVE_DYNAMIC_LOADING
#include "mod_raw.h"
#endif /* HAVE_DYNAMIC_LOADING */

omBin sip_command_bin = omGetSpecBin(sizeof(sip_command));
omBin sip_package_bin = omGetSpecBin(sizeof(sip_package));
//omBin ip_package_bin = omGetSpecBin(sizeof(ip_package));
omBin idrec_bin = omGetSpecBin(sizeof(idrec));

proclevel *procstack=NULL;
#define TEST
idhdl idroot = NULL;

idhdl currPackHdl = NULL;
idhdl basePackHdl = NULL;
package currPack =NULL;
package basePack =NULL;
idhdl currRingHdl = NULL;
ring  currRing = NULL;
ideal currQuotient = NULL;
const char* iiNoName="_";

void paCleanUp(package pack);

/*0 implementation*/

int iiS2I(const char *s)
{
  int i;
  i=s[0];
  if (s[1]!='\0')
  {
    i=(i<<8)+s[1];
    if (s[2]!='\0')
    {
      i=(i<<8)+s[2];
      if (s[3]!='\0')
      {
        i=(i<<8)+s[3];
      }
    }
  }
  return i;  
}

idhdl idrec::get(const char * s, int lev)
{
  assume(s!=NULL);
  assume((lev>=0) && (lev<=1000)); //not really, but if it isnt in that bounds..
  idhdl h = this;
  idhdl found=NULL;
  int l;
  const char *id;
  int i=iiS2I(s);
  int less4=(i < (1<<24));
  while (h!=NULL)
  {
    omCheckAddr((ADDRESS)IDID(h));
    l=IDLEV(h);
    if ((l==0)||(l==lev))
    {
      if (i==h->id_i)
      {
        id=IDID(h);
        if (less4 || (0 == strcmp(s+4,id+4)))
        {
          if (l==lev) return h;
          found=h;
        }
      }
    }
    h = IDNEXT(h);
  }
  return found;
}

//idrec::~idrec()
//{
//  if (id!=NULL)
//  {
//    omFree((ADDRESS)id);
//    id=NULL;
//  }
//  /* much more !! */
//}

idhdl idrec::set(const char * s, int lev, idtyp t, BOOLEAN init)
{
  //printf("define %s, %x, lev: %d, typ: %d\n", s,s,lev,t);
  idhdl h = (idrec *)omAlloc0Bin(idrec_bin);
  int   len = 0;
  IDID(h)   = s;
  IDTYP(h)  = t;
  IDLEV(h)  = lev;
  IDNEXT(h) = this;
  h->id_i=iiS2I(s);
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
      case BIGINT_CMD:
        IDNUMBER(h) = nlInit(0, NULL /* dummy for nlInit*/);
        break;
      case IDEAL_CMD:
      case MODUL_CMD:
        IDFLAG(h) = Sy_bit(FLAG_STD);
      case MATRIX_CMD:
        IDIDEAL(h) = idInit(1,1);
        break;
      case MAP_CMD:
        IDIDEAL(h) = idInit(1,1);
        IDMAP(h)->preimage = omStrDup(IDID(currRingHdl));
        break;
      case STRING_CMD:
        IDSTRING(h) = omStrDup("");
        break;
      case LIST_CMD:
        IDLIST(h)=(lists)omAllocBin(slists_bin);
        IDLIST(h)->Init();
        break;
      case LINK_CMD:
        IDLINK(h)=(si_link) omAlloc0Bin(sip_link_bin);
        break;
      case RING_CMD:
        IDRING(h) = (ring) omAlloc0Bin(sip_sring_bin);
        break;
      case PACKAGE_CMD:
        IDPACKAGE(h) = (package) omAlloc0Bin(sip_package_bin);
        break;
      case PROC_CMD:
        IDPROC(h) = (procinfo*) omAlloc0Bin(procinfo_bin);
        break;
        //the types with the standard init: set the struct to zero
      case RESOLUTION_CMD:
        len=sizeof(ssyStrategy);
        break;
    //other types: without init (int,script,poly,def,package)
    }
    if (len!=0)
    {
      IDSTRING(h) = (char *)omAlloc0(len);
    }
    // additional settings:--------------------------------------
#if 0
    // this leads to a memory leak
    if (t == QRING_CMD)
    {
      // IDRING(h)=rCopy(currRing);
      /* QRING_CMD is ring dep => currRing !=NULL */
    }
    else
#endif
      if (t == PROC_CMD)
    {
      IDPROC(h)->language=LANG_NONE;
    }
    else if (t == PACKAGE_CMD)
    {
      IDPACKAGE(h)->language=LANG_NONE;
      IDPACKAGE(h)->loaded = FALSE;
    }
  }
  // --------------------------------------------------------
  return  h;
}

char * idrec::String()
{
  sleftv tmp;
  memset(&tmp,0,sizeof(sleftv));
  tmp.rtyp=IDTYP(this);
  tmp.data=IDDATA(this);
  tmp.name=IDID(this);
  return tmp.String();
}

//#define KAI
idhdl enterid(const char * s, int lev, idtyp t, idhdl* root, BOOLEAN init)
{
  idhdl h;
  s=omStrDup(s);
  // is it already defined in root ?
  if ((h=(*root)->get(s,lev))!=NULL)
  {
    if (IDLEV(h)==lev)
    {
      if ((IDTYP(h) == t)||(t==DEF_CMD))
      {
        if ((IDTYP(h)==PACKAGE_CMD)
        && (strcmp(s,"Top")==0))
        {
          goto errlabel;
        }
        if (BVERBOSE(V_REDEFINE))
          Warn("redefining %s **",s);
        if (s==IDID(h)) IDID(h)=NULL;
        killhdl2(h,root,currRing);
      }
      else
        goto errlabel;
    }
  }
  // is it already defined in idroot ?
  else if (*root != IDROOT)
  {
    if ((h=IDROOT->get(s,lev))!=NULL)
    {
      if (IDLEV(h)==lev)
      {
        if ((IDTYP(h) == t)||(t==DEF_CMD))
        {
          if (BVERBOSE(V_REDEFINE))
            Warn("redefining %s **",s);
          if (s==IDID(h)) IDID(h)=NULL;
          killhdl2(h,&IDROOT,NULL);
        }
        else
          goto errlabel;
      }
    }
  }
  // is it already defined in currRing->idroot ?
  else if ((currRing!=NULL)&&((*root) != currRing->idroot))
  {
    if ((h=currRing->idroot->get(s,lev))!=NULL)
    {
      if (IDLEV(h)==lev)
      {
        if ((IDTYP(h) == t)||(t==DEF_CMD))
        {
          if (BVERBOSE(V_REDEFINE))
            Warn("redefining %s **",s);
          IDID(h)=NULL;
          killhdl2(h,&currRing->idroot,currRing);
        }
        else
          goto errlabel;
      }
    }
  }
  *root = (*root)->set(s, lev, t, init);
#ifndef NDEBUG
  checkall();
#endif
  return *root;

  errlabel:
    //Werror("identifier `%s` in use(lev h=%d,typ=%d,t=%d, curr=%d)",s,IDLEV(h),IDTYP(h),t,lev);
    Werror("identifier `%s` in use",s);
    //listall();
    omFree((ADDRESS)s);
    return NULL;
}

void killid(const char * id, idhdl * ih)
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
          killhdl2(h,&(currRing->idroot),currRing);
          return;
        }
      }
      Werror("`%s` is not defined",id);
      return;
    }
    killhdl2(h,ih,currRing);
  }
  else
    Werror("kill what ?");
}

void killhdl(idhdl h, package proot)
{
  int t=IDTYP(h);
  if (((BEGIN_RING<t) && (t<END_RING) && (t!=QRING_CMD))
  || ((t==LIST_CMD) && (lRingDependend((lists)IDDATA(h)))))
    killhdl2(h,&currRing->idroot,currRing);
  else
  {
    if(t==PACKAGE_CMD)
    {
      killhdl2(h,&(basePack->idroot),NULL);
    }
    else
    {
      idhdl s=proot->idroot;
      while ((s!=h) && (s!=NULL)) s=s->next;
      if (s!=NULL)
        killhdl2(h,&(proot->idroot),NULL);
      else if (basePack!=proot)
      {
        idhdl s=basePack->idroot;
        while ((s!=h) && (s!=NULL)) s=s->next;
        if (s!=NULL)
          killhdl2(h,&(basePack->idroot),currRing);
        else
          killhdl2(h,&(currRing->idroot),currRing);
       }
    }
  }
}

void killhdl2(idhdl h, idhdl * ih, ring r)
{
  //printf("kill %s, id %x, typ %d lev: %d\n",IDID(h),(int)IDID(h),IDTYP(h),IDLEV(h));
  idhdl hh;

  if (h->attribute!=NULL)
  {
    at_KillAll(h,r);
    //h->attribute=NULL;
  }
  if ((IDTYP(h) == PACKAGE_CMD) && (strcmp(IDID(h),"Top")==0))
  {
    WarnS("can not kill `Top`");
    return;
  }
  // ring / qring  --------------------------------------------------------
  if ((IDTYP(h) == RING_CMD) || (IDTYP(h) == QRING_CMD))
  {
    // any objects defined for this ring ? -> done by rKill
    
    rKill(h);
  }
  // package -------------------------------------------------------------
  else if (IDTYP(h) == PACKAGE_CMD)
  {
    // any objects defined for this package ?
    if ((IDPACKAGE(h)->ref<=0)  &&  (IDPACKAGE(h)->idroot!=NULL))
    {
      if (currPack==IDPACKAGE(h))
      {
        currPack=basePack;
        currPackHdl=NULL;
      }
      idhdl * hd = &IDRING(h)->idroot;
      idhdl  hdh = IDNEXT(*hd);
      idhdl  temp;
      while (hdh!=NULL)
      {
        temp = IDNEXT(hdh);
        killhdl2(hdh,&(IDPACKAGE(h)->idroot),NULL);
        hdh = temp;
      }
      killhdl2(*hd,hd,NULL);
      if (IDPACKAGE(h)->libname!=NULL) omFree((ADDRESS)(IDPACKAGE(h)->libname));
    }
    paKill(IDPACKAGE(h));
    if (currPackHdl==h) currPackHdl=packFindHdl(currPack);
    iiCheckPack(currPack);
  }
  // poly / vector -------------------------------------------------------
  else if ((IDTYP(h) == POLY_CMD) || (IDTYP(h) == VECTOR_CMD))
  {
    assume(r!=NULL);
    p_Delete(&IDPOLY(h),r);
  }
  // ideal / module/ matrix / map ----------------------------------------
  else if ((IDTYP(h) == IDEAL_CMD)
           || (IDTYP(h) == MODUL_CMD)
           || (IDTYP(h) == MATRIX_CMD)
           || (IDTYP(h) == MAP_CMD))
  {
    assume(r!=NULL);
    ideal iid = IDIDEAL(h);
    if (IDTYP(h) == MAP_CMD)
    {
      map im = IDMAP(h);
      omFree((ADDRESS)im->preimage);
    }
    id_Delete(&iid,r);
  }
  // string -------------------------------------------------------------
  else if (IDTYP(h) == STRING_CMD)
  {
    omFree((ADDRESS)IDSTRING(h));
    //IDSTRING(h)=NULL;
  }
  // proc ---------------------------------------------------------------
  else if (IDTYP(h) == PROC_CMD)
  {
    if (piKill(IDPROC(h))) return;
  }
  // number -------------------------------------------------------------
  else if (IDTYP(h) == NUMBER_CMD)
  {
    assume(r!=NULL);
    n_Delete(&IDNUMBER(h),r);
  }
  // bigint -------------------------------------------------------------
  else if (IDTYP(h) == BIGINT_CMD)
  {
    nlDelete(&IDNUMBER(h),NULL);
  }
  // intvec / intmat  ---------------------------------------------------
  else if ((IDTYP(h) == INTVEC_CMD)||(IDTYP(h) == INTMAT_CMD))
  {
    delete IDINTVEC(h);
  }
  // list  -------------------------------------------------------------
  else if (IDTYP(h)==LIST_CMD)
  {
    IDLIST(h)->Clean(r);
    //IDLIST(h)=NULL;
  }
  // link  -------------------------------------------------------------
  else if (IDTYP(h)==LINK_CMD)
  {
    slKill(IDLINK(h));
  }
  else if(IDTYP(h)==RESOLUTION_CMD)
  {
    assume(r!=NULL);
    if (IDDATA(h)!=NULL)
      syKillComputation((syStrategy)IDDATA(h),r);
  }
#ifdef TEST
  else if ((IDTYP(h)!= INT_CMD)&&(IDTYP(h)!=DEF_CMD) && (IDTYP(h)!=NONE))
    Warn("unknown type to kill: %s(%d)",Tok2Cmdname(IDTYP(h)),IDTYP(h));
#endif

  //  general  -------------------------------------------------------------
  // now dechain it and delete idrec
#ifdef KAI
  if(h->next != NULL)
    Print("=======>%s(%x) -> %s<====\n", IDID(h), IDID(h), IDID(h->next));
  else
    Print("=======>%s(%x)<====\n", IDID(h), IDID(h));
#endif

  if (IDID(h)!=NULL) // OB: ?????
    omFree((ADDRESS)IDID(h));
  IDID(h)=NULL;
  IDDATA(h)=NULL;
  if (h == (*ih))
  {
    // h is at the beginning of the list
    *ih = IDNEXT(h) /* ==*ih */;
  }
  else if (ih!=NULL)
  {
    // h is somethere in the list:
    hh = *ih;
    loop
    {
      if (hh==NULL)
      {
        PrintS(">>?<< not found for kill\n");
        return;
      }
      idhdl hhh = IDNEXT(hh);
      if (hhh == h)
      {
        IDNEXT(hh) = IDNEXT(hhh);
        break;
      }
      hh = hhh;
    }
  }
  omFreeBin((ADDRESS)h, idrec_bin);
}

idhdl ggetid(const char *n, BOOLEAN local, idhdl *packhdl)
{
  idhdl h = IDROOT->get(n,myynest);
  idhdl h2=NULL;
  *packhdl = NULL;
  if ((currRing!=NULL) && ((h==NULL)||(IDLEV(h)!=myynest)))
  {
    h2 = currRing->idroot->get(n,myynest);
  }
  if (h2==NULL) return h;
  return h2;
}

idhdl ggetid(const char *n)
{
  idhdl h = IDROOT->get(n,myynest);
  if ((h!=NULL)&&(IDLEV(h)==myynest)) return h;
  idhdl h2=NULL;
  if (currRing!=NULL)
  {
    h2 = currRing->idroot->get(n,myynest);
  }
  if (h2!=NULL) return h2;
  if (h!=NULL) return h;
  if (basePack!=currPack)
    return basePack->idroot->get(n,myynest);
  return NULL;
}

void ipListFlag(idhdl h)
{
  if (hasFlag(h,FLAG_STD)) PrintS(" (SB)");
#ifdef HAVE_PLURAL
  if (hasFlag(h,FLAG_TWOSTD)) PrintS(" (2SB)");
#endif
}

lists ipNameList(idhdl root)
{
  idhdl h=root;
  /* compute the length */
  int l=0;
  while (h!=NULL) { l++; h=IDNEXT(h); }
  /* allocate list */
  lists L=(lists)omAllocBin(slists_bin);
  L->Init(l);
  /* copy names */
  h=root;
  l=0;
  while (h!=NULL)
  {
    /* list is initialized with 0 => no need to clear anything */
    L->m[l].rtyp=STRING_CMD;
    L->m[l].data=omStrDup(IDID(h));
    l++;
    h=IDNEXT(h);
  }
  return L;
}

/*
* move 'tomove' from root1 list to root2 list
*/
static int ipSwapId(idhdl tomove, idhdl &root1, idhdl &root2)
{
  idhdl h;
  /* search 'tomove' in root2 : if found -> do nothing */
  h=root2;
  while ((h!=NULL) && (h!=tomove)) h=IDNEXT(h);
  if (h!=NULL) return FALSE; /*okay */
  /* search predecessor of h in root1, remove 'tomove' */
  h=root1;
  if (tomove==h)
  {
    root1=IDNEXT(h);
  }
  else
  {
    while ((h!=NULL) && (IDNEXT(h)!=tomove)) h=IDNEXT(h);
    if (h==NULL) return TRUE; /* not in the list root1 -> do nothing */
    IDNEXT(h)=IDNEXT(tomove);
  }
  /* add to root2 list */
  IDNEXT(tomove)=root2;
  root2=tomove;
  return FALSE;
}

void  ipMoveId(idhdl tomove)
{
  if ((currRing!=NULL)&&(tomove!=NULL))
  {
    if (((QRING_CMD!=IDTYP(tomove)) && RingDependend(IDTYP(tomove)))
    || ((IDTYP(tomove)==LIST_CMD) && (lRingDependend(IDLIST(tomove)))))
    {
      /*move 'tomove' to ring id's*/
      if (ipSwapId(tomove,IDROOT,currRing->idroot))
      ipSwapId(tomove,basePack->idroot,currRing->idroot);
    }
    else
    {
      /*move 'tomove' to global id's*/
      ipSwapId(tomove,currRing->idroot,IDROOT);
    }
  }
}

const char * piProcinfo(procinfov pi, const char *request)
{
  if(pi == NULL) return "empty proc";
  else if (strcmp(request, "libname")  == 0) return pi->libname;
  else if (strcmp(request, "procname") == 0) return pi->procname;
  else if (strcmp(request, "type")     == 0)
  {
    switch (pi->language)
    {
      case LANG_SINGULAR: return "singular"; break;
      case LANG_C:        return "object";   break;
      case LANG_NONE:     return "none";     break;
      default:            return "unknow language";
    }
  }
  else if (strcmp(request, "ref")      == 0)
  {
    char p[8];
    sprintf(p, "%d", pi->ref);
    return omStrDup(p);  // MEMORY-LEAK
  }
  return "??";
}

void piCleanUp(procinfov pi)
{
  (pi->ref)--;
  if (pi->ref <= 0)
  {
    if (pi->libname != NULL) // OB: ????
      omFree((ADDRESS)pi->libname);
    if (pi->procname != NULL) // OB: ????
      omFree((ADDRESS)pi->procname);

    if( pi->language == LANG_SINGULAR)
    {
      if (pi->data.s.body != NULL) // OB: ????
        omFree((ADDRESS)pi->data.s.body);
    }
    if( pi->language == LANG_C)
    {
    }
    memset((void *) pi, 0, sizeof(procinfo));
    pi->language=LANG_NONE;
  }
}

BOOLEAN piKill(procinfov pi)
{
  Voice *p=currentVoice;
  while (p!=NULL)
  {
    if (p->pi==pi && pi->ref <= 1)
    {
      Warn("`%s` in use, can not be killed",pi->procname);
      return TRUE;
    }
    p=p->next;
  }
  piCleanUp(pi);
  if (pi->ref <= 0)
    omFreeBin((ADDRESS)pi,  procinfo_bin);
  return FALSE;
}

void paCleanUp(package pack)
{
  (pack->ref)--;
  if (pack->ref < 0)
  {
#ifndef HAVE_STATIC
    if( pack->language == LANG_C)
    {
      Print("//dlclose(%s)\n",pack->libname);
#ifdef HAVE_DYNAMIC_LOADING
      dynl_close (pack->handle);
#endif /* HAVE_DYNAMIC_LOADING */
    }
#endif /* HAVE_STATIC */
    omfree((ADDRESS)pack->libname);
    memset((void *) pack, 0, sizeof(sip_package));
    pack->language=LANG_NONE;
  }
}

char *idhdl2id(idhdl pck, idhdl h)
{
  char *name = (char *)omAlloc(strlen(pck->id) + strlen(h->id) + 3);
  sprintf(name, "%s::%s", pck->id, h->id);
  return(name);
}

void iiname2hdl(const char *name, idhdl *pck, idhdl *h)
{
  const char *q = strchr(name, ':');
  char *p, *i;

  if(q==NULL)
  {
    p = omStrDup("");
    i = (char *)omAlloc(strlen(name)+1);
    *i = '\0';
    sscanf(name, "%s", i);
  }
  else {
    if( *(q+1) != ':') return;
    i = (char *)omAlloc(strlen(name)+1);
    *i = '\0';
    if(name == q)
    {
      p = omStrDup("");
      sscanf(name, "::%s", i);
    }
    else
    {
      p = (char *)omAlloc(strlen(name)+1);
      sscanf(name, "%[^:]::%s", p, i);
    }
  }
  //printf("Package: '%s'\n", p);
  //printf("Id Rec : '%s'\n", i);
  omFree(p);
  omFree(i);
}

#if 0
char *getnamelev()
{
  char buf[256];
  sprintf(buf, "(%s:%d)", namespaceroot->name,namespaceroot->lev);
  return(buf);
}
// warning: address of local variable `buf' returned
#endif

void proclevel::push(char *n)
{
  //Print("push %s\n",n);
  proclevel *p=(proclevel*)omAlloc0(sizeof(proclevel));
  p->cRing=currRing;
  p->cRingHdl=currRingHdl;
  p->name=n;
  p->cPackHdl=currPackHdl;
  p->cPack=currPack;
  p->next=this;
  procstack=p;
}
void proclevel::pop()
{
  //Print("pop %s\n",name);
  //if (currRing!=::currRing) PrintS("currRing wrong\n");;
  //::currRing=this->currRing;
  //if (r==NULL) Print("set ring to NULL at lev %d(%s)\n",myynest,name);
  //::currRingHdl=this->currRingHdl;
  //if((::currRingHdl==NULL)||(IDRING(::currRingHdl)!=(::currRing)))
  //  ::currRingHdl=rFindHdl(::currRing,NULL,NULL);
  //Print("restore pack=%s,1.obj=%s\n",IDID(currPackHdl),IDID(currPack->idroot));
  currPackHdl=this->cPackHdl;
  currPack=this->cPack;
  iiCheckPack(currPack);
  proclevel *p=this;
  procstack=next;
  omFreeSize(p,sizeof(proclevel));
}

idhdl packFindHdl(package r)
{
  idhdl h=basePack->idroot;
  while (h!=NULL)
  {
    if ((IDTYP(h)==PACKAGE_CMD)
        && (IDPACKAGE(h)==r))
      return h;
    h=IDNEXT(h);
  }
  return NULL;
}
