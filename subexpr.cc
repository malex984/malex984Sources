/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/*
* ABSTRACT: handling of leftv
*/
/* $Id: subexpr.cc,v 1.87 2004-04-16 17:15:53 Singular Exp $ */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include "mod2.h"
#include "tok.h"
#include "ipid.h"
#include "intvec.h"
#include <omalloc.h>
#include "febase.h"
#include "polys.h"
#include "ideals.h"
#include "maps.h"
#include "matpol.h"
#include "kstd1.h"
#include "timer.h"
#include "ring.h"
#include "ffields.h"
#include "numbers.h"
#include "ipshell.h"
#include "lists.h"
#include "attrib.h"
#include "silink.h"
#include "syz.h"
#include "subexpr.h"


omBin sSubexpr_bin = omGetSpecBin(sizeof(sSubexpr));
omBin sleftv_bin = omGetSpecBin(sizeof(sleftv));
omBin procinfo_bin = omGetSpecBin(sizeof(procinfo));
omBin libstack_bin = omGetSpecBin(sizeof(libstack));
static omBin size_two_bin = omGetSpecBin(2);

sleftv     sLastPrinted;
const char sNoName[]="_";
#ifdef SIQ
BOOLEAN siq=FALSE;
#endif

void sleftv::Set(int val)
{
  Init();
  rtyp = INT_CMD;
  data = (void *)val;
}

int sleftv::listLength()
{
  int n = 1;
  leftv sl = next;
  while (sl!=NULL)
  {
    n++;
    sl=sl->next;
  }
  return n;
}

void sleftv::Print(leftv store, int spaces)
{
  int  t=Typ();
  if (errorreported) return;
#ifdef SIQ
  if (rtyp==COMMAND)
  {
    command c=(command)data;
    char ch[2];
    ch[0]=c->op;ch[1]='\0';
    char *s=ch;
    if (c->op>127) s=iiTwoOps(c->op);
    ::Print("##command %d(%s), %d args\n",
      c->op, s, c->argc);
    if (c->argc>0)
      c->arg1.Print(NULL,spaces+2);
    if(c->argc<4)
    {
      if (c->argc>1)
        c->arg2.Print(NULL,spaces+2);
      if (c->argc>2)
        c->arg3.Print(NULL,spaces+2);
    }
    PrintS("##end");
  }
  else
#endif
  {
    const char *n=Name();
    char *s;
    void *d=Data();
    if (errorreported)
      return;
    if ((store!=NULL)&&(store!=this))
      store->CleanUp();

    switch (t /*=Typ()*/)
      {
        case UNKNOWN:
        case DEF_CMD:
          ::Print("%-*.*s`%s`",spaces,spaces," ",n);
          break;
        case PACKAGE_CMD:
          ::Print("%-*.*s`%s`",spaces,spaces," ",n);
          break;
        case NONE:
          return;
        case INTVEC_CMD:
        case INTMAT_CMD:
          ((intvec *)d)->show(t,spaces);
          break;
        case RING_CMD:
        case QRING_CMD:
          ::Print("%-*.*s",spaces,spaces," ");
          rWrite((ring)d);
          break;
        case MATRIX_CMD:
          iiWriteMatrix((matrix)d,n,2,spaces);
          break;
        case MAP_CMD:
        case MODUL_CMD:
        case IDEAL_CMD:
          iiWriteMatrix((matrix)d,n,1,spaces);
          break;
        case POLY_CMD:
        case VECTOR_CMD:
          ::Print("%-*.*s",spaces,spaces," ");
          pWrite0((poly)d);
          break;
        case RESOLUTION_CMD:
          syPrint((syStrategy)d);
          break;
        case STRING_CMD:
          ::Print("%-*.*s%s",spaces,spaces," ",(char *)d);
          break;
       case INT_CMD:
          ::Print("%-*.*s%d",spaces,spaces," ",(int)d);
          break;
       case PROC_CMD:
         {
           procinfov pi=(procinfov)d;
           ::Print("%-*.*s// libname  : %s\n",spaces,spaces," ",
                   piProcinfo(pi, "libname"));
           ::Print("%-*.*s// procname : %s\n",spaces,spaces," ",
                   piProcinfo(pi, "procname"));
           ::Print("%-*.*s// type     : %s",spaces,spaces," ",
                   piProcinfo(pi, "type"));
           //           ::Print("%-*.*s// ref      : %s",spaces,spaces," ",
           //   piProcinfo(pi, "ref"));
           break;
         }
       case POINTER_CMD:
         { package pack = (package)d;
         ::Print("%-*.*s// %s\n",spaces,spaces," ","PointerTest");
         ::Print("%-*.*s// %s",spaces,spaces," ",IDID(pack->idroot));
         //::Print(((char *)(pack->idroot)->data), spaces);
         break;
         }
       case LINK_CMD:
          {
            si_link l=(si_link)d;
            ::Print("%-*.*s// type : %s\n",spaces,spaces," ",
                    slStatus(l, "type"));
            ::Print("%-*.*s// mode : %s\n",spaces,spaces," ",
                    slStatus(l, "mode"));
            ::Print("%-*.*s// name : %s\n",spaces,spaces," ",
                    slStatus(l, "name"));
            ::Print("%-*.*s// open : %s\n",spaces,spaces," ",
                    slStatus(l, "open"));
            ::Print("%-*.*s// read : %s\n",spaces,spaces," ",
                    slStatus(l, "read"));
            ::Print("%-*.*s// write: %s",spaces,spaces," ",
                    slStatus(l, "write"));
          break;
          }
        case NUMBER_CMD:
          s=String(d);
          if (s==NULL) return;
          ::Print("%-*.*s",spaces,spaces," ");
          PrintS(s);
          omFree((ADDRESS)s);
          break;
        case LIST_CMD:
        {
          lists l=(lists)d;
          if (l->nr<0)
             ::Print("%-*.*sempty list\n",spaces,spaces," ");
          else
          {
            int i=0;
            for (;i<=l->nr;i++)
            {
              if (l->m[i].rtyp!=DEF_CMD)
              {
                ::Print("%-*.*s[%d]:\n",spaces,spaces," ",i+1);
                l->m[i].Print(NULL,spaces+3);
              }
            }
          }
          break;
        }
#ifdef TEST
        default:
          ::Print("Print:unknown type %s(%d)", Tok2Cmdname(t),t);
#endif
      } /* end switch: (Typ()) */
  }
  if (next!=NULL)
  {
    if (t==COMMAND) PrintLn();
    else if (t!=LIST_CMD) PrintS(" ");
    next->Print(NULL,spaces);
  }
  else if (t!=LIST_CMD)
  {
    PrintLn();
  }
#ifdef SIQ
  if (rtyp!=COMMAND)
#endif
  {
    if ((store!=NULL)
    && (store!=this)
    && (t/*Typ()*/!=LINK_CMD)
    && (t/*Typ()*/!=RING_CMD)
    && (t/*Typ()*/!=QRING_CMD)
    && (t/*Typ()*/!=POINTER_CMD)
    && (t/*Typ()*/!=PACKAGE_CMD)
    && (t/*Typ()*/!=PROC_CMD)
    && (t/*Typ()*/!=DEF_CMD)
    )
    {
      store->rtyp=t/*Typ()*/;
      store->data=CopyD();
      if((e!=NULL)||(attribute!=NULL))
      {
        store->attribute=CopyA();
      }
      if (e==NULL)
      {
        store->flag=flag;
      }
      //else
      //{
      //}
    }
  }
}

void sleftv::CleanUp(ring r)
{
  if ((name!=NULL) && (name!=sNoName) && (rtyp!=IDHDL))
  {
    //::Print("free %x (%s)\n",name,name);
    omFree((ADDRESS)name);
  }
  //name=NULL;
  //flag=0;
  if (data!=NULL)
  {
    switch (rtyp)
    {
      case INTVEC_CMD:
      case INTMAT_CMD:
        delete (intvec *)data;
        break;
      case MAP_CMD:
        omFree((ADDRESS)((map)data)->preimage);
        ((map)data)->preimage=NULL;
        // no break: kill the image as an ideal
      case MATRIX_CMD:
      case MODUL_CMD:
      case IDEAL_CMD:
        id_Delete((ideal *)(&data),r);
        break;
      case STRING_CMD:
          omFree((ADDRESS)data);
        break;
      case POLY_CMD:
      case VECTOR_CMD:
        p_Delete((poly *)(&data),r);
        break;
      case NUMBER_CMD:
        n_Delete((number *)(&data),r);
        break;
      case LIST_CMD:
        ((lists)data)->Clean(r); // may contain ring-dep data
        break;
      case QRING_CMD:
      case RING_CMD:
        rKill((ring)data);
        break;
      case PROC_CMD:
        piKill((procinfov)data);
        break;
      case LINK_CMD:
        slKill((si_link)data);
        break;
      case COMMAND:
      {
        command cmd=(command)data;
        if (cmd->arg1.rtyp!=0) cmd->arg1.CleanUp();
        if (cmd->arg2.rtyp!=0) cmd->arg2.CleanUp();
        if (cmd->arg3.rtyp!=0) cmd->arg3.CleanUp();
        omFreeBin((ADDRESS)data, ip_command_bin);
        break;
      }
      case RESOLUTION_CMD:
      {
        syKillComputation((syStrategy)data,r);
        break;
      }
#ifdef TEST
      // the following types do not take memory
      // or are not copied
      case IDHDL:
      case PACKAGE_CMD:
      case ANY_TYPE:
      case VECHO:
      case VPAGELENGTH:
      case VPRINTLEVEL:
      case VCOLMAX:
      case VTIMER:
#ifdef HAVE_RTIMER
      case VRTIMER:
#endif
      case VOICE:
      case VMAXDEG:
      case VMAXMULT:
      case TRACE:
      case VSHORTOUT:
      case VNOETHER:
      case VMINPOLY:
      case LIB_CMD:
      case 0:
      case INT_CMD:
        break;
      default:
        ::Print("CleanUp: unknown type %d\n",rtyp);  /* DEBUG */
#endif
    } /* end switch: (rtyp) */
    //data=NULL;
  }
  if (attribute!=NULL)
  {
    switch (rtyp)
    {
      case POINTER_CMD:
      case PACKAGE_CMD:
      case IDHDL:
      case ANY_TYPE:
      case VECHO:
      case VPAGELENGTH:
      case VPRINTLEVEL:
      case VCOLMAX:
      case VTIMER:
#ifdef HAVE_RTIMER
      case VRTIMER:
#endif
      case VOICE:
      case VMAXDEG:
      case VMAXMULT:
      case TRACE:
      case VSHORTOUT:
      case VNOETHER:
      case VMINPOLY:
      case LIB_CMD:
      case 0:
        //attribute=NULL;
        break;
      default:
      {
        attr t;
        while (attribute!=NULL)
        {
          t=attribute->next;
          attribute->kill();
          attribute=t;
        }
      }
    }
  }
  Subexpr h;
  while (e!=NULL)
  {
    h=e->next;
    omFreeBin((ADDRESS)e, sSubexpr_bin);
    e=h;
  }
  //rtyp=NONE;
  if (next!=NULL)
  {
    leftv tmp_n;
    do
    {
      tmp_n=next->next;
      //next->name=NULL;
      next->next=NULL;
      next->CleanUp(r);
      omFreeBin((ADDRESS)next, sleftv_bin);
      next=tmp_n;
    } while (next!=NULL);
  }
  Init();
}

BOOLEAN sleftv::RingDependend()
{
  int rt=Typ();
  if(::RingDependend(rt) && (rt!=QRING_CMD))
    return TRUE;
  if (rt==LIST_CMD)
    return lRingDependend((lists)Data());
  return FALSE;
}

void * slInternalCopy(leftv source, int t, void *d, Subexpr e)
{
  switch (t)
  {
    case INTVEC_CMD:
    case INTMAT_CMD:
      return (void *)ivCopy((intvec *)d);
    case MATRIX_CMD:
      return (void *)mpCopy((matrix)d);
    case IDEAL_CMD:
    case MODUL_CMD:
      return  (void *)idCopy((ideal)d);
    case STRING_CMD:
      if ((e==NULL)
      || (source->rtyp==LIST_CMD)
      || ((source->rtyp==IDHDL)&&(IDTYP((idhdl)source->data)==LIST_CMD)))
        return (void *)omStrDup((char *)d);
      else if (e->next==NULL)
      {
        char *s=(char*)omAllocBin(size_two_bin);
        s[0]=*(char *)d;
        s[1]='\0';
        return s;
      }
      #ifdef TEST
      else
      {
        Werror("not impl. string-op in `%s`",my_yylinebuf);
        return NULL;
      }
      #endif
    case POINTER_CMD:
      return d;
    case PACKAGE_CMD:
      return  (void *)paCopy((package) d);
    case PROC_CMD:
      return  (void *)piCopy((procinfov) d);
    case POLY_CMD:
    case VECTOR_CMD:
      return  (void *)pCopy((poly)d);
    case INT_CMD:
      return  d;
    case NUMBER_CMD:
      return  (void *)nCopy((number)d);
    case MAP_CMD:
      return  (void *)maCopy((map)d);
    case LIST_CMD:
      return  (void *)lCopy((lists)d);
    case LINK_CMD:
      return (void *)slCopy((si_link) d);
    case RING_CMD:
    case QRING_CMD:
      {
        ring r=(ring)d;
        r->ref++;
        return d;
      }
    case RESOLUTION_CMD:
      return (void*)syCopy((syStrategy)d);
#ifdef TEST
    case DEF_CMD:
    case NONE:
      break; /* error recovery: do nothing */
    //case COMMAND:
    default:
      Warn("InternalCopy: cannot copy type %s(%d)",
            Tok2Cmdname(source->rtyp),source->rtyp);
#endif
  }
  return NULL;
}

void sleftv::Copy(leftv source)
{
  Init();
  rtyp=source->Typ();
  void *d=source->Data();
  if(!errorreported)
  {
    switch (rtyp)
    {
      case INTVEC_CMD:
      case INTMAT_CMD:
        data=(void *)ivCopy((intvec *)d);
        break;
      case MATRIX_CMD:
        data=(void *)mpCopy((matrix)d);
        break;
      case IDEAL_CMD:
      case MODUL_CMD:
        data= (void *)idCopy((ideal)d);
        break;
      case STRING_CMD:
        data= (void *)omStrDup((char *)d);
        break;
      case POINTER_CMD:
        data=d;
        break;
      case PROC_CMD:
        data= (void *)piCopy((procinfov) d);
        break;
      case POLY_CMD:
      case VECTOR_CMD:
        data= (void *)pCopy((poly)d);
        break;
      case INT_CMD:
        data= d;
        break;
      case NUMBER_CMD:
        data= (void *)nCopy((number)d);
        break;
      case MAP_CMD:
        data= (void *)maCopy((map)d);
        break;
      case LIST_CMD:
        data= (void *)lCopy((lists)d);
        break;
      case LINK_CMD:
        data = (void *)slCopy((si_link)d);
        break;
      case RING_CMD:
      case QRING_CMD:
        {
          if (d!=NULL)
          {
            ring r=(ring)d;
            r->ref++;
            data=d;
          }
          else
          {
            WerrorS("invalid ring description");
          }
          break;
        }
      case RESOLUTION_CMD:
        data=(void*)syCopy((syStrategy)d);
        break;
      #ifdef TEST
      case DEF_CMD:
      case NONE:
        break; /* error recovery: do nothing */
      //case COMMAND:
      default:
        Warn("Copy: cannot copy type %s(%d)",Tok2Cmdname(rtyp),rtyp);
      #endif
    }
    if ((source->attribute!=NULL)||(source->e!=NULL))
      attribute=source->CopyA();
    if(source->e==NULL)
    {
      flag=source->flag;
    }
    //else
    //{
    //}
    if (source->next!=NULL)
    {
      next=(leftv)omAllocBin(sleftv_bin);
      next->Copy(source->next);
    }
  }
}

void * sleftv::CopyD(int t)
{
  if ((rtyp!=IDHDL)&&(e==NULL))
  {
    if (iiCheckRing(t)) return NULL;
    void *x=data;
    if (rtyp==VNOETHER) x=(void *)pCopy(ppNoether);
    else if (rtyp==LIB_CMD)
      x=(void *)omStrDup((char *)Data());
    else if ((rtyp==VMINPOLY)&& (currRing->minpoly!=NULL)&&(!rField_is_GF()))
      x=(void *)nCopy(currRing->minpoly);
    data=NULL;
    return x;
  }
  void *d=Data(); // will also do a iiCheckRing
  if ((!errorreported) && (d!=NULL)) return slInternalCopy(this,t,d,e);
  return NULL;
}

//void * sleftv::CopyD()
//{
  //if ((rtyp!=IDHDL)&&(e==NULL)
  //&&(rtyp!=VNOETHER)&&(rtyp!=LIB_CMD)&&(rtyp!=VMINPOLY))
  //{
  //  void *x=data;
  //  data=NULL;
  //  return x;
  //}
//  return CopyD(Typ());
//}

attr sleftv::CopyA()
{
  attr *a=Attribute();
  if ((a!=NULL) && (*a!=NULL))
    return (*a)->Copy();
  return NULL;
}

char *  sleftv::String(void *d, BOOLEAN typed, int dim)
{
#ifdef SIQ
  if (rtyp==COMMAND)
  {
    ::Print("##command %d\n",((command)data)->op);
    if (((command)data)->arg1.rtyp!=0)
      ((command)data)->arg1.Print(NULL,2);
    if (((command)data)->arg2.rtyp!=0)
      ((command)data)->arg2.Print(NULL,2);
    if (((command)data)->arg3.rtyp==0)
      ((command)data)->arg3.Print(NULL,2);
    PrintS("##end\n");
    return omStrDup("");
  }
#endif
  if (d==NULL) d=Data();
  if (!errorreported)
  {
    char *s;
    const char *n;
    if (name!=NULL) n=name;
    else n=sNoName;
    switch (Typ())
    {
        case INT_CMD:
          if (typed)
          {
            s=(char *)omAlloc(MAX_INT_LEN+7);
            sprintf(s,"int(%d)",(int)d);
          }
          else
          {
            s=(char *)omAlloc(MAX_INT_LEN+2);
            sprintf(s,"%d",(int)d);
          }
          return s;

        case STRING_CMD:
          if (d == NULL)
          {
            if (typed) return omStrDup("\"\"");
            return omStrDup("");
          }
          if (typed)
          {
            s = (char*) omAlloc(strlen((char*) d) + 3);
            sprintf(s,"\"%s\"", (char*) d);
            return s;
          }
          else
          {
            return omStrDup((char*)d);
          }

        case POLY_CMD:
        case VECTOR_CMD:
          if (typed)
          {
            char* ps = pString((poly) d);
            s = (char*) omAlloc(strlen(ps) + 10);
            sprintf(s,"%s(%s)", (Typ() == POLY_CMD ? "poly" : "vector"), ps);
            return s;
          }
          else
            return omStrDup(pString((poly)d));

        case NUMBER_CMD:
          StringSetS((char*) (typed ? "number(" : ""));
          if ((rtyp==IDHDL)&&(IDTYP((idhdl)data)==NUMBER_CMD))
          {
            nWrite(IDNUMBER((idhdl)data));
          }
          else if (rtyp==NUMBER_CMD)
          {
            number n=(number)data;
            nWrite(n);
            data=(char *)n;
          }
          else if((rtyp==VMINPOLY)&&(rField_is_GF()))
          {
            nfShowMipo();
          }
          else
          {
            number n=nCopy((number)d);
            nWrite(n);
            nDelete(&n);
          }
          s = StringAppendS((char*) (typed ? ")" : ""));
          return omStrDup(s);

        case MATRIX_CMD:
          s= iiStringMatrix((matrix)d,dim);
          if (typed)
          {
            char* ns = (char*) omAlloc(strlen(s) + 40);
            sprintf(ns, "matrix(ideal(%s),%d,%d)", s,
                    ((ideal) d)->nrows, ((ideal) d)->ncols);
            omCheckAddr(ns);
            return ns;
          }
          else
          {
            return omStrDup(s);
          }

        case MODUL_CMD:
        case IDEAL_CMD:
        case MAP_CMD:
          s= iiStringMatrix((matrix)d,dim);
          if (typed)
          {
            char* ns = (char*) omAlloc(strlen(s) + 10);
            sprintf(ns, "%s(%s)", (Typ()==MODUL_CMD ? "module" : "ideal"), s);
            omCheckAddr(ns);
            return ns;
          }
          return omStrDup(s);

        case INTVEC_CMD:
        case INTMAT_CMD:
        {
          intvec *v=(intvec *)d;
          s = v->String(dim);
          if (typed)
          {
            char* ns;
            if (Typ() == INTMAT_CMD)
            {
              ns = (char*) omAlloc(strlen(s) + 40);
              sprintf(ns, "intmat(intvec(%s),%d,%d)", s, v->rows(), v->cols());
            }
            else
            {
              ns = (char*) omAlloc(strlen(s) + 10);
              sprintf(ns, "intvec(%s)", s);
            }
            omCheckAddr(ns);
            omFree(s);
            return ns;
          }
          else
            return s;
        }

        case RING_CMD:
        case QRING_CMD:
          s  = rString((ring)d);

          if (typed)
          {
            char* ns;
            if (Typ() == QRING_CMD)
            {
              char* id = iiStringMatrix((matrix) ((ring) d)->qideal, dim);
              ns = (char*) omAlloc(strlen(s) + strlen(id) + 20);
              sprintf(ns, "\"%s\";%sideal(%s)", s,(dim == 2 ? "\n" : " "), id);
            }
            else
            {
              ns = (char*) omAlloc(strlen(s) + 4);
              sprintf(ns, "\"%s\"", s);
            }
            omFree(s);
            omCheckAddr(ns);
            return ns;
          }
          return s;

        case RESOLUTION_CMD:
        {
          lists l = syConvRes((syStrategy)d);
          s = lString(l, typed, dim);
          l->Clean();
          return s;
        }

        case PROC_CMD:
        {
          procinfo* pi = (procinfo*) d;
          if((pi->language == LANG_SINGULAR) && (pi->data.s.body!=NULL))
            s = (pi->data.s.body);
          else
            s = "";
          if (typed)
          {
            char* ns = (char*) omAlloc(strlen(s) + 4);
            sprintf(ns, "\"%s\"", s);
            omCheckAddr(ns);
            return ns;
          }
          return omStrDup(s);
        }

        case LINK_CMD:
          s = slString((si_link) d);
          if (typed)
          {
            char* ns = (char*) omAlloc(strlen(s) + 10);
            sprintf(ns, "link(\"%s\")", s);
            omFree(s);
            omCheckAddr(ns);
            return ns;
          }
          return s;

        case LIST_CMD:
          return lString((lists) d, typed, dim);
    } /* end switch: (Typ()) */
  }
  return omStrDup("");
}


int  sleftv::Typ()
{
  if (e==NULL)
  {
    switch (rtyp)
    {
      case IDHDL:
        return IDTYP((idhdl)data);
      case VECHO:
      case VPAGELENGTH:
      case VPRINTLEVEL:
      case VCOLMAX:
      case VTIMER:
#ifdef HAVE_RTIMER
      case VRTIMER:
#endif
      case VOICE:
      case VMAXDEG:
      case VMAXMULT:
      case TRACE:
      case VSHORTOUT:
        return INT_CMD;
      case LIB_CMD:
        return STRING_CMD;
      case VMINPOLY:
        return NUMBER_CMD;
      case VNOETHER:
        return POLY_CMD;
      //case COMMAND:
      //  return COMMAND;
      default:
        return rtyp;
    }
  }
  int r=0;
  int t=rtyp;
  if (t==IDHDL) t=IDTYP((idhdl)data);
  switch (t)
  {
    case INTVEC_CMD:
    case INTMAT_CMD:
      r=INT_CMD;
      break;
    case IDEAL_CMD:
    case MATRIX_CMD:
    case MAP_CMD:
      r=POLY_CMD;
      break;
    case MODUL_CMD:
      r=VECTOR_CMD;
      break;
    case STRING_CMD:
      r=STRING_CMD;
      break;
    case LIST_CMD:
    {
      lists l;
      if (rtyp==IDHDL) l=IDLIST((idhdl)data);
      else             l=(lists)data;
      if ((0<e->start)&&(e->start<=l->nr+1))
      {
        l->m[e->start-1].e=e->next;
        r=l->m[e->start-1].Typ();
        l->m[e->start-1].e=NULL;
      }
      else
      {
        //Warn("out of range: %d not in 1..%d",e->start,l->nr+1);
        r=NONE;
      }
      break;
    }
    default:
      Werror("cannot index type %d",t);
  }
  return r;
}

int  sleftv::LTyp()
{
  lists l=NULL;
  int r;
  if (rtyp==LIST_CMD)
    l=(lists)data;
  else if ((rtyp==IDHDL)&& (IDTYP((idhdl)data)==LIST_CMD))
    l=IDLIST((idhdl)data);
  else
    return Typ();
  //if (l!=NULL)
  {
    if ((e!=NULL) && (e->next!=NULL))
    {
      if ((0<e->start)&&(e->start<=l->nr+1))
      {
        l->m[e->start-1].e=e->next;
        r=l->m[e->start-1].LTyp();
        l->m[e->start-1].e=NULL;
      }
      else
      {
        //Warn("out of range: %d not in 1..%d",e->start,l->nr+1);
        r=NONE;
      }
      return r;
    }
    return LIST_CMD;
  }
  return Typ();
}

void sleftv::SetData(void* what)
{
  if (rtyp == IDHDL)
  {
    IDDATA((idhdl)data) = (char *)what;
  }
  else
  {
    data = what;
  }
}

void * sleftv::Data()
{
  if (rtyp!=IDHDL && iiCheckRing(rtyp))
     return NULL;
  if (e==NULL)
  {
    switch (rtyp)
    {
      case VECHO:      return (void *)si_echo;
      case VPAGELENGTH:return (void *)pagelength;
      case VPRINTLEVEL:return (void *)printlevel;
      case VCOLMAX:    return (void *)colmax;
      case VTIMER:     return (void *)getTimer();
#ifdef HAVE_RTIMER
      case VRTIMER:    return (void *)getRTimer();
#endif
      case VOICE:      return (void *)(myynest+1);
      case VMAXDEG:    return (void *)Kstd1_deg;
      case VMAXMULT:   return (void *)Kstd1_mu;
      case TRACE:      return (void *)traceit;
      case VSHORTOUT:  return (void *)(currRing != NULL ? currRing->ShortOut : 0);
      case VMINPOLY:   if (currRing != NULL &&
                           (currRing->minpoly!=NULL)&&(!rField_is_GF()))
                       /* Q(a), Fp(a), but not GF(q) */
                         return (void *)currRing->minpoly;
                       else
                         return (void *)nNULL;
      case VNOETHER:   return (void *) ppNoether;
#ifndef HAVE_NS
      case LIB_CMD:    {
                         idhdl h = ggetid( "LIB" );
                         if(h==NULL) return (void *)sNoName;
                         return IDSTRING(h);
                       }
#else
      case LIB_CMD:    {
                         return (void *)sNoName;
                       }
#endif
      case IDHDL:
        return IDDATA((idhdl)data);
      case POINTER_CMD:
        return IDDATA((idhdl)data);
      case COMMAND:
        //return NULL;
      default:
        return data;
    }
  }
  /* e != NULL : */
  int t=rtyp;
  void *d=data;
  if (t==IDHDL)
  {
    t=((idhdl)data)->typ;
    d=IDDATA((idhdl)data);
  }
  if (iiCheckRing(t))
    return NULL;
  char *r=NULL;
  int index=e->start;
  switch (t)
  {
    case INTVEC_CMD:
    {
      intvec *iv=(intvec *)d;
      if ((index<1)||(index>iv->length()))
      {
        if (!errorreported)
          Werror("wrong range[%d] in intvec(%d)",index,iv->length());
      }
      else
        r=(char *)((*iv)[index-1]);
      break;
    }
    case INTMAT_CMD:
    {
      intvec *iv=(intvec *)d;
      if ((index<1)
         ||(index>iv->rows())
         ||(e->next->start<1)
         ||(e->next->start>iv->cols()))
      {
        if (!errorreported)
        Werror("wrong range[%d,%d] in intmat(%dx%d)",index,e->next->start,
                                                     iv->rows(),iv->cols());
      }
      else
        r=(char *)(IMATELEM((*iv),index,e->next->start));
      break;
    }
    case IDEAL_CMD:
    case MODUL_CMD:
    case MAP_CMD:
    {
      ideal I=(ideal)d;
      if ((index<1)||(index>IDELEMS(I)))
      {
        if (!errorreported)
          Werror("wrong range[%d] in ideal/module(%d)",index,IDELEMS(I));
      }
      else
        r=(char *)I->m[index-1];
      break;
    }
    case STRING_CMD:
    {
      // this was a memory leak
      // we evalute it, cleanup and replace this leftv by it's evalutated form
      // the evalutated form will be build in tmp
      sleftv tmp;
      tmp.Init();
      tmp.rtyp=STRING_CMD;
      r=(char *)omAllocBin(size_two_bin);
      if ((index>0)&& (index<=(int)strlen((char *)d)))
      {
        r[0]=*(((char *)d)+index-1);
        r[1]='\0';
      }
      else
      {
        r[0]='\0';
      }
      tmp.data=r;
      if ((rtyp==IDHDL)||(rtyp==STRING_CMD))
      {
        tmp.next=next; next=NULL;
        data=NULL; d=NULL;
        CleanUp();
        memcpy(this,&tmp,sizeof(tmp));
      }
      // and, remember, r is also the result...
      else
      {
        // ???
        // here we still have a memory leak...
        // example: list L="123","456";
        // L[1][2];
        // therefore, it should never happen:
        assume(0);
        // but if it happens: here is the temporary fix:
        // omMarkAsStaticAddr(r);
      }
      break;
    }
    case MATRIX_CMD:
    {
      if ((index<1)
         ||(index>MATROWS((matrix)d))
         ||(e->next->start<1)
         ||(e->next->start>MATCOLS((matrix)d)))
      {
        if (!errorreported)
          Werror("wrong range[%d,%d] in intmat(%dx%d)",
                  index,e->next->start,
                  MATROWS((matrix)d),MATCOLS((matrix)d));
      }
      else
        r=(char *)MATELEM((matrix)d,index,e->next->start);
      break;
    }
    case LIST_CMD:
    {
      lists l=(lists)d;
      if ((0<index)&&(index<=l->nr+1))
      {
        if ((e->next!=NULL)
        && (l->m[index-1].rtyp==STRING_CMD))
        // string[..].Data() modifies sleftv, so let's do it ourself
        {
          char *dd=(char *)l->m[index-1].data;
          int j=e->next->start-1;
          r=(char *)omAllocBin(size_two_bin);
          if ((j>=0) && (j<(int)strlen(dd)))
          {
            r[0]=*(dd+j);
            r[1]='\0';
          }
          else
          {
            r[0]='\0';
          }
        }
        else
        {
          l->m[index-1].e=e->next;
          r=(char *)l->m[index-1].Data();
          l->m[index-1].e=NULL;
        }
      }
      else //if (!errorreported)
        Werror("wrong range[%d] in list(%d)",index,l->nr+1);
      break;
    }
#ifdef TEST
    default:
      Werror("cannot index type %s(%d)",Tok2Cmdname(t),t);
#endif
  }
  return r;
}

attr * sleftv::Attribute()
{
  if (e==NULL) return &attribute;
  if ((rtyp==LIST_CMD)
  ||((rtyp==IDHDL)&&(IDTYP((idhdl)data)==LIST_CMD)))
  {
    leftv v=LData();
    return &(v->attribute);
  }
  return NULL;
}

leftv sleftv::LData()
{
  if (e!=NULL)
  {
    lists l=NULL;

    if (rtyp==LIST_CMD)
      l=(lists)data;
    if ((rtyp==IDHDL)&& (IDTYP((idhdl)data)==LIST_CMD))
      l=IDLIST((idhdl)data);
    if (l!=NULL)
    {
      if ((0>=e->start)||(e->start>l->nr+1))
        return NULL;
      if (e->next!=NULL)
      {
        l->m[e->start-1].e=e->next;
        leftv r=l->m[e->start-1].LData();
        l->m[e->start-1].e=NULL;
        return r;
      }
      return &(l->m[e->start-1]);
    }
  }
  return this;
}

leftv sleftv::LHdl()
{
  if (e!=NULL)
  {
    lists l=NULL;

    if (rtyp==LIST_CMD)
      l=(lists)data;
    if ((rtyp==IDHDL)&& (IDTYP((idhdl)data)==LIST_CMD))
      l=IDLIST((idhdl)data);
    if (l!=NULL)
    {
      if ((0>=e->start)||(e->start>l->nr+1))
        return NULL;
      if (e->next!=NULL)
      {
        l->m[e->start-1].e=e->next;
        leftv r=l->m[e->start-1].LHdl();
        l->m[e->start-1].e=NULL;
        return r;
      }
      return &(l->m[e->start-1]);
    }
  }
  return this;
}

BOOLEAN assumeStdFlag(leftv h)
{
  if ((h->e!=NULL)&&(h->LTyp()==LIST_CMD))
  {
    return assumeStdFlag(h->LData());
  }
  if (!hasFlag(h,FLAG_STD))
  {
    if (!TEST_VERB_NSB)
      Warn("%s is no standard basis",h->Name());
    return FALSE;
  }
  return TRUE;
}

/*2
* transforms a name (as an string created by omAlloc or omStrDup)
* into an expression (sleftv), deletes the string
* utility for grammar and iparith
*/
void syMake(leftv v,char * id, idhdl packhdl)
{
  /* resolv an identifier: (to DEF_CMD, if siq>0)
  * 1) reserved id: done by scanner
  * 2) `basering` / 'Current`
  * 3) existing identifier, local
  * 4) ringvar, local ring
  * 5) existing identifier, global
  * 6) monom (resp. number), local ring: consisting of:
  * 6') ringvar, global ring
  * 6'') monom (resp. number), local ring
  * 7) monom (resp. number), non-local ring
  * 8) basering
  * 9) `_`
  * 10) everything else is of type 0
  */
#ifdef TEST
  if ((*id<' ')||(*id>(char)126))
  {
    Print("wrong id :%s:\n",id);
  }
#endif
  v->Init();
#ifdef HAVE_NS
  v->packhdl = NULL;
  if(packhdl != NULL)
  {
  //  Print("setting req_packhdl to %s\n",IDID(packhdl));
    v->req_packhdl = IDPACKAGE(packhdl);
  }  
  else v->req_packhdl = currPack;
//  if (v->req_packhdl!=basePack)
//    Print("search %s in %s\n",id,v->req_packhdl->libname);
#endif /* HAVE_NS */
#ifdef SIQ
  if (siq<=0)
#endif
  {
    idhdl h=NULL;
    if (!isdigit(id[0]))
    {
      if (strcmp(id,"basering")==0)
      {
        if (currRingHdl!=NULL)
        {
          if (id!=IDID(currRingHdl)) omFree((ADDRESS)id);
          v->rtyp = IDHDL;
          v->data = (char *)currRingHdl;
          v->name = IDID(currRingHdl);
          v->flag = IDFLAG(currRingHdl);
          return;
        }
        else
        {
          v->name = id;
          return; /* undefined */
        }
      }
#ifdef HAVE_NS
      if (strcmp(id,"Current")==0)
      {
        if (currPackHdl!=NULL)
        {
          omFree((ADDRESS)id);
          v->rtyp = IDHDL;
          v->data = (char *)currPackHdl;
          v->name = IDID(currPackHdl);
          v->flag = IDFLAG(currPackHdl);
          return;
        }
        else
        {
          v->name = id;
          return; /* undefined */
        }
      }
      if(v->req_packhdl!=currPack)
      {
        h=v->req_packhdl->idroot->get(id,myynest);
      }
      else
#endif
      h=ggetid(id);
      /* 3) existing identifier, local */
      if ((h!=NULL) && (IDLEV(h)==myynest))
      {
        if (id!=IDID(h)) omFree((ADDRESS)id);
        v->rtyp = IDHDL;
        v->data = (char *)h;
        v->flag = IDFLAG(h);
        v->name = IDID(h);
        v->attribute=IDATTR(h);
        return;
      }
    }
    /* 4. local ring: ringvar */
    if ((currRingHdl!=NULL) && (IDLEV(currRingHdl)==myynest))
    {
      int vnr;
      if ((vnr=rIsRingVar(id))>=0)
      {
        poly p=pOne();
        pSetExp(p,vnr+1,1);
        pSetm(p);
        v->data = (void *)p;
        v->name = id;
        v->rtyp = POLY_CMD;
        return;
      }
    }
    /* 5. existing identifier, global */
    if (h!=NULL)
    {
      if (id!=IDID(h)) omFree((ADDRESS)id);
      v->rtyp = IDHDL;
      v->data = (char *)h;
      v->flag = IDFLAG(h);
      v->name = IDID(h);
      v->attribute=IDATTR(h);
      return;
    }
    /* 6. local ring: number/poly */
    if ((currRingHdl!=NULL) && (IDLEV(currRingHdl)==myynest))
    {
      BOOLEAN ok=FALSE;
      poly p = (!yyInRingConstruction) ? pmInit(id,ok) : (poly)NULL;
      if (ok)
      {
        if (p==NULL)
        {
          v->data = (void *)nInit(0);
          v->rtyp = NUMBER_CMD;
          omFree((ADDRESS)id);
        }
        else
        if (pIsConstant(p))
        {
          v->data = pGetCoeff(p);
          pGetCoeff(p)=NULL;
          pLmFree(p);
          v->rtyp = NUMBER_CMD;
          v->name = id;
        }
        else
        {
          v->data = p;
          v->rtyp = POLY_CMD;
          v->name = id;
        }
        return;
      }
    }
    /* 7. non-local ring: number/poly */
    {
      BOOLEAN ok=FALSE;
      poly p = ((currRing!=NULL)     /* ring required */
               && (currRingHdl!=NULL)
               && (!yyInRingConstruction) /* not in decl */
               && (IDLEV(currRingHdl)!=myynest)) /* already in case 4/6 */
                     ? pmInit(id,ok) : (poly)NULL;
      if (ok)
      {
        if (p==NULL)
        {
          v->data = (void *)nInit(0);
          v->rtyp = NUMBER_CMD;
          omFree((ADDRESS)id);
        }
        else
        if (pIsConstant(p))
        {
          v->data = pGetCoeff(p);
          pGetCoeff(p)=NULL;
          pLmFree(p);
          v->rtyp = NUMBER_CMD;
          v->name = id;
        }
        else
        {
          v->data = p;
          v->rtyp = POLY_CMD;
          v->name = id;
        }
        return;
      }
    }
    /* 8. basering ? */
    if ((myynest>1)&&(currRingHdl!=NULL))
    {
      if (strcmp(id,IDID(currRingHdl))==0)
      {
        if (IDID(currRingHdl)!=id) omFree((ADDRESS)id);
        v->rtyp=IDHDL;
        v->data=currRingHdl;
        v->name=IDID(currRingHdl);
        v->attribute=IDATTR(currRingHdl);
        return;
      }
    }
#ifdef HAVE_NS
    if((v->req_packhdl!=basePack) && (v->req_packhdl==currPack))
    {
      h=basePack->idroot->get(id,myynest);
      if (h!=NULL)
      {	
        if (id!=IDID(h)) omFree((ADDRESS)id);
        v->rtyp = IDHDL;
        v->data = (char *)h;
        v->flag = IDFLAG(h);
        v->name = IDID(h);
        v->attribute=IDATTR(h);
        v->req_packhdl=basePack;
        return;
      }
    }
#endif
  }
#ifdef SIQ
  else
    v->rtyp=DEF_CMD;
#endif
  /* 9: _ */
  if (strcmp(id,"_")==0)
  {
    omFree((ADDRESS)id);
    v->Copy(&sLastPrinted);
  }
  else
  {
    /* 10: everything else */
    /* v->rtyp = UNKNOWN;*/
    v->name = id;
  }
}

int sleftv::Eval()
{
  BOOLEAN nok=FALSE;
  leftv nn=next;
  next=NULL;
  if(rtyp==IDHDL)
  {
    int t=Typ();
    if (t!=PROC_CMD)
    {
      void *d=CopyD(t);
      data=d;
      rtyp=t;
      name=NULL;
      e=NULL;
    }
  }
  else if (rtyp==COMMAND)
  {
    command d=(command)data;
    if(d->op==PROC_CMD) //assume d->argc==2
    {
      char *what=(char *)(d->arg1.Data());
      idhdl h=ggetid(what);
      if((h!=NULL)&&(IDTYP(h)==PROC_CMD))
      {
        nok=d->arg2.Eval();
        if(!nok)
        {
#ifdef HAVE_NS
          leftv r=iiMake_proc(h,req_packhdl,&d->arg2);
#else /* HAVE_NS */
          leftv r=iiMake_proc(h,&d->arg2);
#endif /* HAVE_NS */
          if (r!=NULL)
            memcpy(this,r,sizeof(sleftv));
          else
            nok=TRUE;
        }
      }
      else nok=TRUE;
    }
    else if (d->op=='=') //assume d->argc==2
    {
      if ((d->arg1.rtyp!=IDHDL)&&(d->arg1.rtyp!=DEF_CMD))
      {
        nok=d->arg1.Eval();
      }
      if (!nok)
      {
        char *n=d->arg1.name;
        nok=(n == NULL) || d->arg2.Eval();
        if (!nok)
        {
          int save_typ=d->arg1.rtyp;
          omCheckAddr(n);
          if (d->arg1.rtyp!=IDHDL)
          syMake(&d->arg1,n);
          omCheckAddr(d->arg1.name);
          if (d->arg1.rtyp==IDHDL)
          {
            n=omStrDup(IDID((idhdl)d->arg1.data));
            killhdl((idhdl)d->arg1.data);
            d->arg1.Init();
            //d->arg1.data=NULL;
            d->arg1.name=n;
          }
          d->arg1.rtyp=DEF_CMD;
          sleftv t;
          if(save_typ!=PROC_CMD) save_typ=d->arg2.rtyp;
          if (::RingDependend(d->arg2.rtyp))
            nok=iiDeclCommand(&t,&d->arg1,0,save_typ,&currRing->idroot);
          else
            nok=iiDeclCommand(&t,&d->arg1,0,save_typ,&IDROOT);
          memcpy(&d->arg1,&t,sizeof(sleftv));
          omCheckAddr(d->arg1.name);
          nok=nok||iiAssign(&d->arg1,&d->arg2);
          omCheckIf(d->arg1.name != NULL,  // OB: ????
                    omCheckAddr(d->arg1.name));
          if (!nok)
          {
            memset(&d->arg1,0,sizeof(sleftv));
            this->CleanUp();
            rtyp=NONE;
          }
        }
      }
      else nok=TRUE;
    }
    else if (d->argc==1)
    {
      nok=d->arg1.Eval();
      nok=nok||iiExprArith1(this,&d->arg1,d->op);
    }
    else if(d->argc==2)
    {
      nok=d->arg1.Eval();
      nok=nok||d->arg2.Eval();
      nok=nok||iiExprArith2(this,&d->arg1,d->op,&d->arg2);
    }
    else if(d->argc==3)
    {
      nok=d->arg1.Eval();
      nok=nok||d->arg2.Eval();
      nok=nok||d->arg3.Eval();
      nok=nok||iiExprArith3(this,d->op,&d->arg1,&d->arg2,&d->arg3);
    }
    else if(d->argc!=0)
    {
      nok=d->arg1.Eval();
      nok=nok||iiExprArithM(this,&d->arg1,d->op);
    }
    else // d->argc == 0
    {
      nok = iiExprArithM(this, NULL, d->op);
    }
  }
  else if (((rtyp==0)||(rtyp==DEF_CMD))
    &&(name!=NULL))
  {
     syMake(this,name);
  }
#ifdef MDEBUG
  switch(Typ())
  {
    case NUMBER_CMD:
#ifdef LDEBUG
      nTest((number)Data());
#endif
      break;
    case POLY_CMD:
      pTest((poly)Data());
      break;
    case IDEAL_CMD:
    case MODUL_CMD:
    case MATRIX_CMD:
      {
        ideal id=(ideal)Data();
        omCheckAddrSize(id,sizeof(*id));
        int i=id->ncols*id->nrows-1;
        for(;i>=0;i--) pTest(id->m[i]);
      }
      break;
  }
#endif
  if (nn!=NULL) nok=nok||nn->Eval();
  next=nn;
  return nok;
}

char *iiSleftv2name(leftv v)
{
  return(v->name);
}
