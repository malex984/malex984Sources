/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/* $Id: lists.cc,v 1.17 1999-09-27 15:05:24 obachman Exp $ */
/*
* ABSTRACT: handling of the list type
*/
#include "mod2.h"
#include "tok.h"
#include "febase.h"
//#include "ipid.h"
#include "polys.h"
#include "ideals.h"
#include "attrib.h"
#include "ipshell.h"
#include "intvec.h"
#include "lists.h"

lists lCopy(lists L)
{
  lists N=(lists)Alloc0(sizeof(slists));
  int n=L->nr;
  if (L->nr>=0)
    N->Init(n+1);
  else
    N->Init();
  for(;n>=0;n--)
  {
    N->m[n].Copy(&L->m[n]);
  }
#ifdef HAVE_NAMESPACES_N
  N->src_packhdl = L->src_packhdl;
#endif
  //Print("copy list with %d -> %d elems\n",L->nr,N->nr);
  return N;
}

/*2
* concat 2 lists
*/
BOOLEAN lAdd(leftv res, leftv u, leftv v)
{
  lists l=(lists) Alloc(sizeof(slists));
  lists ul=(lists)u->CopyD();
  lists vl=(lists)v->CopyD();
  l->Init(ul->nr+vl->nr+2);
  int i;

  for(i=0;i<=ul->nr;i++)
  {
    //Print("u[%d]->r[%d]\n",i,i);
    l->m[i].rtyp=ul->m[i].rtyp;
    l->m[i].data=ul->m[i].data;
  }
  for(i=0;i<=vl->nr;i++)
  {
    //Print("v[%d]->r[%d]\n",i,i+ul->nr+1);
    l->m[i+ul->nr+1].rtyp=vl->m[i].rtyp;
    l->m[i+ul->nr+1].data=vl->m[i].data;
  }
  Free((ADDRESS)ul->m,(ul->nr+1)*sizeof(sleftv));
  Free((ADDRESS)ul,sizeof(slists));
  Free((ADDRESS)vl->m,(vl->nr+1)*sizeof(sleftv));
  Free((ADDRESS)vl,sizeof(slists));
  memset(u,0,sizeof(*u));
  memset(v,0,sizeof(*v));
  res->data = (char *)l;
  //res->Print();
  return FALSE;
}

/*2
* insert v into list u, destroys u
*/
lists lInsert0(lists ul, leftv v, int pos)
{
  if ((pos<0)||(v->rtyp==NONE))
    return NULL;
  lists l=(lists) Alloc(sizeof(slists));
  l->Init(max(ul->nr+2,pos+1));
  int i,j;

  for(i=j=0;i<=ul->nr;i++,j++)
  {
    if(j==pos) j++;
    l->m[j].Copy(&ul->m[i]);
  }
  for(j=ul->nr+1;j<pos;j++)
    l->m[j].rtyp=DEF_CMD;
  // memset(&(l->m[pos]),0,sizeof(sleftv)); - done by Init
  l->m[pos].rtyp=v->Typ();
  l->m[pos].data=v->CopyD();
  l->m[pos].flag=v->flag;
  l->m[pos].attribute=v->CopyA();
  Free((ADDRESS)ul->m,(ul->nr+1)*sizeof(sleftv));
  Free((ADDRESS)ul,sizeof(slists));
  return l;
}

/*2
* insert v into list u, at the beginning
*/
BOOLEAN lInsert(leftv res, leftv u, leftv v)
{
  lists ul=(lists)u->CopyD();
  res->data=(char *)lInsert0(ul,v,0);
  if (res->data==NULL)
  {
    Werror("cannot insert type `%s`",Tok2Cmdname(v->Typ()));
    return TRUE;
  }
  return FALSE;
}

/*2
* insert v into list u at pos w
*/
BOOLEAN lInsert3(leftv res, leftv u, leftv v, leftv w)
{
  lists ul=(lists)u->CopyD();
  res->data=(char *)lInsert0(ul,v,(int)w->Data());
  if (res->data==NULL)
  {
    Werror("cannot insert type `%s` at pos. %d",
      Tok2Cmdname(v->Typ()),(int)w->Data());
    return TRUE;
  }
  return FALSE;
}

/*2
* append v to list u
*/
BOOLEAN lAppend(leftv res, leftv u, leftv v)
{
  lists ul=(lists)u->CopyD();
  res->data=(char *)lInsert0(ul,v,ul->nr+1);
  return (res->data==NULL);
}

/*2
* delete v-th element from list u
*/
BOOLEAN lDelete(leftv res, leftv u, leftv v)
{
  lists ul=(lists)u->Data();
  int VIndex=(int)v->Data()-1;

  if((0<=VIndex)&&(VIndex<=ul->nr))
  {
    int i,j;
    lists l=(lists) Alloc(sizeof(slists));
    l->Init(ul->nr);

    ul=(lists)u->CopyD();
    for(i=j=0;i<=ul->nr;i++,j++)
    {
      if (i!=VIndex)
      {
        l->m[j].Copy(&(ul->m[i]));
      }
      else
      {
        j--;
        ul->m[i].CleanUp();
      }
    }
    Free((ADDRESS)ul->m,(ul->nr+1)*sizeof(sleftv));
    Free((ADDRESS)ul,sizeof(slists));
    res->data = (char *)l;
    return FALSE;
  }
  Werror("wrong index %d in list(%d)",VIndex+1,ul->nr+1);
  return TRUE;
}

/*2
* check, if a list contains any ring dependend data
*/
BOOLEAN lRingDependend(lists L)
{
  if (L==NULL) return TRUE;
  int i=0;
  while (i<=L->nr)
  {
    if ((L->m[i].rtyp!=QRING_CMD)
    && (BEGIN_RING<L->m[i].rtyp)
    && (L->m[i].rtyp<END_RING))
      return TRUE;
    if ((L->m[i].rtyp==LIST_CMD)&&lRingDependend((lists)L->m[i].data))
      return TRUE;
    i++;
  }
  return FALSE;
}

lists liMakeResolv(resolvente r, int length, int reallen,
  int typ0, intvec ** weights)
{
  lists L=(lists)Alloc0(sizeof(slists));
  if (length<=0)
  {
    // handle "empty" resolutions
    L->Init(0);
  }
  else
  {
    int oldlength=length;
    while (r[length-1]==NULL) length--;
    if (reallen<=0) reallen=pVariables;
    reallen=max(reallen,length);
    L->Init(reallen);
    int i=0;

    while (i<length)
    {
      if (r[i]!=NULL)
      {
        if (i==0)
        {
          L->m[i].rtyp=typ0;
          int j=IDELEMS(r[0])-1;
          while ((j>0) && (r[0]->m[j]==NULL)) j--;
          j++;
          if (j!=IDELEMS(r[0]))
          {
            pEnlargeSet(&(r[0]->m),IDELEMS(r[0]),j-IDELEMS(r[0]));
            IDELEMS(r[0])=j;
          }
        }
        else
        {
          L->m[i].rtyp=MODUL_CMD;
          int rank=IDELEMS(r[i-1]);
          if (idIs0(r[i-1]))
          {
            idDelete(&(r[i]));
            r[i]=idFreeModule(rank);
          }
          else
          {
            r[i]->rank=max(rank,idRankFreeModule(r[i]));
          }
          idSkipZeroes(r[i]);
        }
        L->m[i].data=(void *)r[i];
        if ((weights!=NULL) && (weights[i]!=NULL))
        {
          atSet((idhdl)&L->m[i],mstrdup("isHomog"),weights[i],INTVEC_CMD);
          weights[i] = NULL;
        }
      }
      #ifdef TEST
      else
      {
        // should not happen:
        Warn("internal NULL in resolvente");
        L->m[i].data=(void *)idInit(1,1);
      }
      #endif
      i++;
    }
    Free((ADDRESS)r,oldlength*sizeof(ideal));
    if (i==0)
    {
      L->m[0].rtyp=typ0;
      L->m[0].data=(char *)idInit(1,1);
      i=1;
    }
    while (i<reallen)
    {
      L->m[i].rtyp=MODUL_CMD;
      ideal I=(ideal)L->m[i-1].data;
      ideal J;
      int rank=IDELEMS(I);
      if (idIs0(I))
      {
        J=idFreeModule(rank);
      }
      else
      {
        J=idInit(1,rank);
      }
      L->m[i].data=(void *)J;
      i++;
    }
    //Print("make res of length %d (0..%d) L:%d\n",length,length-1,L->nr);
  }
  return L;
}

resolvente liFindRes(lists L, int * len, int *typ0,intvec *** weights)
{
  resolvente r;
  intvec ** w=NULL,*tw=NULL;

  *len=L->nr+1;
  if (*len<=0)
  {
    WerrorS("empty list");
    return NULL;
  }
  r=(ideal *)Alloc0((*len)*sizeof(ideal));
  w=(intvec**)Alloc0((*len)*sizeof(intvec*));
  int i=0;
  *typ0=MODUL_CMD;
  while (i<(*len))
  {
    if (L->m[i].rtyp != MODUL_CMD)
    {
      if (L->m[i].rtyp!=IDEAL_CMD)
      {
        Werror("element %d is not of type module",i+1);
        Free((ADDRESS)r,(*len)*sizeof(ideal));
        return NULL;
      }
      *typ0=IDEAL_CMD;
    }
    if ((i>0) && (idIs0(r[i-1])))
    {
      //*len=i-1;
      break;
    }
    r[i]=(ideal)L->m[i].data;
    tw=(intvec*)atGet((idhdl)&L->m[i],mstrdup("isHomog"));
    if (tw!=NULL)
    {
      w[i]=ivCopy(tw);
    }
    tw = NULL;
    i++;
  }
  BOOLEAN hom_complex=TRUE;
  int j=0;
  while ((j<i) && hom_complex)
  {
    hom_complex = hom_complex && (w[i]!=NULL);
    j++;
  }
  if ((!hom_complex) || (weights==NULL))
  {
    for (j=0;j<i;j++)
    {
      if (w[j]!=NULL) delete w[j];
    }
    Free((ADDRESS)w,(*len)*sizeof(intvec*));
  }
  else
  {
    *weights = w;
  }
  //Print("find res of length %d (0..%d) L:%d\n",*len,(*len)-1,L->nr);
  return r;
}

char* lString(lists l, BOOLEAN typed, int dim)
{
  if (l->nr == -1)
  {
    if (typed) return mstrdup("list()");
    return mstrdup("");
  }

  char** slist = (char**) Alloc((l->nr+1) * sizeof(char*));
  int i, j, k;
  char *s;
  for (i=0, j = 0, k = 0; i<=l->nr; i++)
  {
    slist[i] = l->m[i].String(NULL, typed, dim);
    assume(slist[i] != NULL);
    mmTestL(slist[i]);
    if (*(slist[i]) != '\0')
    {
      j += strlen(slist[i]);
      k++;
    }
  }
  s = (char*) AllocL(j+k+2+(typed ? 10 : 0) + (dim == 2 ? k : 0));

  if (typed)
    sprintf(s, "list(");
  else
    *s = '\0';

  for (i=0; i<=l->nr; i++)
  {
    if (*(slist[i]) != '\0')
    {
      strcat(s, slist[i]);
      strcat(s, ",");
      if (dim == 2) strcat(s, "\n");
    }
    mmTestL(s);
    FreeL(slist[i]);
  }
  if (k > 0) s[strlen(s) - (dim == 2 ? 2 : 1)] = '\0';
  if (typed) strcat(s, ")");
  mmTestL(s);
  Free(slist, (l->nr+1) * sizeof(char*));
  return s;
}
