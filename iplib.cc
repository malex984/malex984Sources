/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/* $Id: iplib.cc,v 1.101 2002-12-13 16:20:06 Singular Exp $ */
/*
* ABSTRACT: interpreter: LIB and help
*/

//#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#include "mod2.h"
#include "tok.h"
#include "ipid.h"
#include "omalloc.h"
#include "febase.h"
#include "ring.h"
#include "subexpr.h"
#include "ipshell.h"
#include "lists.h"

BOOLEAN load_modules(char *newlib, char *fullname, BOOLEAN tellerror);

#ifdef HAVE_LIBPARSER
#  include "libparse.h"
#else /* HAVE_LIBPARSER */
   procinfo *iiInitSingularProcinfo(procinfov pi, char *libname,
                                    char *procname, int line, long pos,
                                    BOOLEAN pstatic = FALSE);
#endif /* HAVE_LIBPARSER */
#define NS_LRING (procstack->cRing)

#include "mod_raw.h"

static char *iiConvName(const char *p);
#ifdef HAVE_LIBPARSER
void yylprestart (FILE *input_file );
int current_pos(int i=0);
extern int yylp_errno;
extern int yylplineno;
extern char *yylp_errlist[];
void print_init();
libstackv library_stack;
#endif

//int IsCmd(char *n, int tok);
char mytolower(char c);

/*2
* return TRUE if the libray libname is already loaded
*/
BOOLEAN iiGetLibStatus(char *lib)
{
  idhdl hl;

#ifndef HAVE_NS
  char *p;

  hl = IDROOT->get("LIB", 0);
  if (hl == NULL || (p=strstr(IDSTRING(hl), lib)) == NULL) return FALSE;
  if ((p!=IDSTRING(hl)) && (*(p-1)!=',')) return FALSE;

  return TRUE;
#else
  char *plib = iiConvName(lib);
  hl = basePack->idroot->get(plib,0);
  if((hl==NULL) ||(IDTYP(hl)!=PACKAGE_CMD))
  {
    omFree(plib);
    return FALSE;
  }
  omFree(plib);
  return (strcmp(lib,IDPACKAGE(hl)->libname)==0);
#endif
}

/*2
* find the library of an proc:
*  => return (pi->libname)
*/
char * iiGetLibName(procinfov pi)
{
  return pi->libname;
}

/*2
* given a line 'proc[ ]+{name}[ \t]*'
* return a pointer to name and set the end of '\0'
* changes the input!
* returns: e: pointer to 'end of name'
*          ct: changed char at the end of s
*/
char* iiProcName(char *buf, char & ct, char* &e)
{
  char *s=buf+5;
  while (*s==' ') s++;
  e=s+1;
  while ((*e>' ') && (*e!='(')) e++;
  ct=*e;
  *e='\0';
  return s;
}

/*2
* given a line with args, return the argstr
*/
char * iiProcArgs(char *e,BOOLEAN withParenth)
{
  while ((*e==' ') || (*e=='\t') || (*e=='(')) e++;
  if (*e<' ')
  {
    if (withParenth)
    {
      // no argument list, allow list #
      return omStrDup("parameter list #;");
    }
    else
    {
      // empty list
      return omStrDup("");
    }
  }
  BOOLEAN in_args;
  BOOLEAN args_found;
  char *s;
  char *argstr=(char *)omAlloc(127); // see ../omalloc/omTables.inc
  int argstrlen=127;
  *argstr='\0';
  do
  {
    args_found=FALSE;
    s=e; // set s to the starting point of the arg
         // and search for the end
    while ((*e!=',')&&(*e!=')')&&(*e!='\0'))
    {
      args_found=args_found || (*e>' ');
      e++;
    }
    in_args=(*e==',');
    if (args_found)
    {
      *e='\0';
      // check for space:
      if ((int)strlen(argstr)+12 /* parameter + ;*/ +(int)strlen(s)>= argstrlen)
      {
        argstrlen*=2;
        char *a=(char *)omAlloc( argstrlen);
        strcpy(a,argstr);
        omFree((ADDRESS)argstr);
        argstr=a;
      }
      // copy the result to argstr
      strcat(argstr,"parameter ");
      strcat(argstr,s);
      strcat(argstr,"; ");
      e++; // e was pointing to ','
    }
  } while (in_args);
  return argstr;
}

/*2
* locate `procname` in lib `libname` and find the part `part`:
*  part=0: help, between, but excluding the line "proc ..." and "{...":
*    => return
*  part=1: body, between "{ ..." and "}", including the 1. line, w/o "{"
*    => set pi->data.s.body, return NULL
*  part=2: example, between, but excluding the line "exapmle {..." and "}":
*    => return
*/
char* iiGetLibProcBuffer(procinfo *pi, int part )
{
  char buf[256], *s = NULL, *p;
  long procbuflen;

  FILE * fp = feFopen( pi->libname, "rb", NULL, TRUE );
  if (fp==NULL)
  {
    return NULL;
  }

  fseek(fp, pi->data.s.proc_start, SEEK_SET);
  if(part==0)
  { // load help string
    int i, offset=0;
    long head = pi->data.s.def_end - pi->data.s.proc_start;
    procbuflen = pi->data.s.help_end - pi->data.s.help_start;
    if (procbuflen<5)
      return NULL; // help part does not exist
    //Print("Help=%ld-%ld=%d\n", pi->data.s.body_start,
    //    pi->data.s.proc_start, procbuflen);
    s = (char *)omAlloc(procbuflen+head+3);
    myfread(s, head, 1, fp);
    s[head] = '\n';
    fseek(fp, pi->data.s.help_start, SEEK_SET);
    myfread(s+head+1, procbuflen, 1, fp);
    s[procbuflen+head+1] = '\n';
    s[procbuflen+head+2] = '\0';
    offset=0;
    for(i=0;i<=procbuflen+head+2; i++)
    {
      if(s[i]=='\\' &&
         (s[i+1]=='"' || s[i+1]=='{' || s[i+1]=='}' || s[i+1]=='\\'))
      {
        i++;
        offset++;
      }
      if(offset>0) s[i-offset] = s[i];
    }
    return(s);
  }
  else if(part==1)
  { // load proc part - must exist
    procbuflen = pi->data.s.def_end - pi->data.s.proc_start;
    char *ss=(char *)omAlloc(procbuflen+2);
    //fgets(buf, sizeof(buf), fp);
    myfread( ss, procbuflen, 1, fp);
    char ct;
    char *e;
    s=iiProcName(ss,ct,e);
    char *argstr=NULL;
    *e=ct;
    argstr=iiProcArgs(e,TRUE);

    assume(pi->data.s.body_end > pi->data.s.body_start);

    procbuflen = pi->data.s.body_end - pi->data.s.body_start;
    pi->data.s.body = (char *)omAlloc( strlen(argstr)+procbuflen+15+
                                      strlen(pi->libname) );
    //Print("Body=%ld-%ld=%d\n", pi->data.s.body_end,
    //    pi->data.s.body_start, procbuflen);
    assume(pi->data.s.body != NULL);
    fseek(fp, pi->data.s.body_start, SEEK_SET);
    strcpy(pi->data.s.body,argstr);
    myfread( pi->data.s.body+strlen(argstr), procbuflen, 1, fp);
    procbuflen+=strlen(argstr);
    omFree(argstr);
    omFree(ss);
    fclose( fp );
    pi->data.s.body[procbuflen] = '\0';
    strcat( pi->data.s.body+procbuflen, "\n;return();\n\n" );
    strcat( pi->data.s.body+procbuflen+13,pi->libname);
    s=strchr(pi->data.s.body,'{');
    if (s!=NULL) *s=' ';
    return NULL;
  }
  else if(part==2)
  { // example
    if ( pi->data.s.example_lineno == 0)
      return NULL; // example part does not exist
    // load example
    fseek(fp, pi->data.s.example_start, SEEK_SET);
    fgets(buf, sizeof(buf), fp); // skip line with "example"
    procbuflen = pi->data.s.proc_end - pi->data.s.example_start - strlen(buf);
    //Print("Example=%ld-%ld=%d\n", pi->data.s.proc_end,
    //  pi->data.s.example_start, procbuflen);
    s = (char *)omAlloc(procbuflen+14);
    myfread(s, procbuflen, 1, fp);
    s[procbuflen] = '\0';
    strcat(s+procbuflen-3, "\n;return();\n\n" );
    p=strchr(s,'{');
    if (p!=NULL) *p=' ';
    return(s);
  }
  return NULL;
}

/*2
* start a proc
* parameters are built as exprlist
* TODO:interrupt
* return FALSE on success, TRUE if an error occurs
*/
BOOLEAN iiPStart(idhdl pn, sleftv  * v)
{
  char * str;
  BOOLEAN err=FALSE;
  int old_echo=si_echo;
  char save_flags=0;
  procinfov pi=NULL;

  /* init febase ======================================== */
  /* we do not enter this case if filename != NULL !! */
  if (pn!=NULL)
  {
    pi = IDPROC(pn);
    if(pi!=NULL)
    {
      save_flags=pi->trace_flag;
      if( pi->data.s.body==NULL )
      {
        iiGetLibProcBuffer(pi);
        if (pi->data.s.body==NULL) return TRUE;
      }
      newBuffer( omStrDup(pi->data.s.body), BT_proc,
                 pi, pi->data.s.body_lineno-(v!=NULL) );
    }
  }
  /* generate argument list ======================================*/
  if (v!=NULL)
  {
    iiCurrArgs=(leftv)omAllocBin(sleftv_bin);
    memcpy(iiCurrArgs,v,sizeof(sleftv));
    memset(v,0,sizeof(sleftv));
  }
  else
  {
    iiCurrArgs=NULL;
  }
  /* start interpreter ======================================*/
  myynest++;
  err=yyparse();
#ifdef HAVE_NS
  checkall();
#endif
  if (sLastPrinted.rtyp!=0)
  {
    sLastPrinted.CleanUp();
  }
  //Print("kill locals for %s (level %d)\n",IDID(pn),myynest);
  killlocals(myynest);
#ifdef HAVE_NS
  checkall();
#endif
  //Print("end kill locals for %s (%d)\n",IDID(pn),myynest);
  myynest--;
  si_echo=old_echo;
  if (pi!=NULL)
    pi->trace_flag=save_flags;
  return err;
}

#ifdef USE_IILOCALRING
ring    *iiLocalRing;
#endif
sleftv  *iiRETURNEXPR;
int     iiRETURNEXPR_len=0;

#ifdef RDEBUG
static void iiShowLevRings()
{
  int i;
#ifdef USE_IILOCALRING
  for (i=1;i<=myynest;i++)
  {
    Print("lev %d:",i);
    if (iiLocalRing[i]==NULL) PrintS("NULL");
    else                      Print("%d",iiLocalRing[i]);
    PrintLn();
  }
#endif
#if  0
//#ifdef HAVE_NS
  i=myynest;
  proclevel *p=procstack;
  while (p!=NULL)
  {
    Print("lev %d:",i);
    if (p->cRingHdl==NULL) PrintS("NULL");
    else                   Print("%s",IDID(p->cRingHdl));
    PrintLn();
    p=p->next;
  }
#endif
  if (currRing==NULL) PrintS("curr:NULL\n");
  else                Print ("curr:%x\n",currRing);
}
#endif /* RDEBUG */

static void iiCheckNest()
{
  if (myynest >= iiRETURNEXPR_len-1)
  {
    iiRETURNEXPR=(sleftv *)omreallocSize(iiRETURNEXPR,
                                   iiRETURNEXPR_len*sizeof(sleftv),
                                   (iiRETURNEXPR_len+16)*sizeof(sleftv));
    omMarkAsStaticAddr(iiRETURNEXPR);
#ifdef USE_IILOCALRING
    iiLocalRing=(ring *)omreallocSize(iiLocalRing,
                                   iiRETURNEXPR_len*sizeof(ring),
                                   (iiRETURNEXPR_len+16)*sizeof(ring));
#endif
    iiRETURNEXPR_len+=16;
  }
}
#ifdef HAVE_NS
sleftv * iiMake_proc(idhdl pn, package pack, sleftv* sl)
#else /* HAVE_NS */
sleftv * iiMake_proc(idhdl pn, sleftv* sl)
#endif /* HAVE_NS */
{
  int err;
  procinfov pi = IDPROC(pn);
  char *plib = iiConvName(pi->libname);
  omFree((ADDRESS)plib);
  if(pi->is_static && myynest==0)
  {
    Werror("'%s::%s()' is a local procedure and cannot be accessed by an user.",
           pi->libname, pi->procname);
    return NULL;
  }
  iiCheckNest();
#ifdef USE_IILOCALRING
  iiLocalRing[myynest]=currRing;
#endif
  iiRETURNEXPR[myynest+1].Init();
  procstack->push(pi->procname);
  if ((traceit&TRACE_SHOW_PROC)
  || (pi->trace_flag&TRACE_SHOW_PROC))
  {
    if (traceit&TRACE_SHOW_LINENO) PrintLn();
    Print("entering%-*.*s %s (level %d)\n",myynest*2,myynest*2," ",IDID(pn),myynest);
  }
#ifdef RDEBUG
  if (traceit&TRACE_SHOW_RINGS) iiShowLevRings();
#endif
  switch (pi->language)
  {
      default:
      case LANG_NONE:
                 err=TRUE;
                 break;

    case LANG_SINGULAR:
                 #ifdef HAVE_NS
                 if ((pi->pack!=NULL)&&(currPack!=pi->pack))
                 {
                   currPack=pi->pack;
		   iiCheckPack(currPack);
                   currPackHdl=packFindHdl(currPack);
                   //Print("set pack=%s\n",IDID(currPackHdl));
                 }
                 else if ((pack!=NULL)&&(currPack!=pack))
                 {
                   currPack=pack;
		   iiCheckPack(currPack);
                   currPackHdl=packFindHdl(currPack);
                   //Print("set pack=%s\n",IDID(currPackHdl));
                 }
                 #endif
                 err=iiPStart(pn,sl);
                 #ifdef HAVE_NS
                 #endif
                 break;
    case LANG_C:
                 leftv res = (leftv)omAlloc0Bin(sleftv_bin);
                 err = (pi->data.o.function)(res, sl);
                 iiRETURNEXPR[myynest+1].Copy(res);
                 omFreeBin((ADDRESS)res,  sleftv_bin);
                 break;
  }
  if ((traceit&TRACE_SHOW_PROC)
  || (pi->trace_flag&TRACE_SHOW_PROC))
  {
    if (traceit&TRACE_SHOW_LINENO) PrintLn();
    Print("leaving %-*.*s %s (level %d)\n",myynest*2,myynest*2," ",IDID(pn),myynest);
  }
#ifdef RDEBUG
  if (traceit&TRACE_SHOW_RINGS) iiShowLevRings();
#endif
  if (err)
  {
    iiRETURNEXPR[myynest+1].CleanUp();
    //iiRETURNEXPR[myynest+1].Init(); //done by CleanUp
  }
#ifdef USE_IILOCALRING
#if 0
  if(procstack->cRing != iiLocalRing[myynest]) Print("iiMake_proc: 1 ring not saved procs:%x, iiLocal:%x\n",procstack->cRing, iiLocalRing[myynest]);
#endif
  if (iiLocalRing[myynest] != currRing)
  {
    if (((iiRETURNEXPR[myynest+1].Typ()>BEGIN_RING)
      && (iiRETURNEXPR[myynest+1].Typ()<END_RING))
    || ((iiRETURNEXPR[myynest+1].Typ()==LIST_CMD)
      && (lRingDependend((lists)iiRETURNEXPR[myynest+1].Data()))))
    {
      //idhdl hn;
      char *n;
      char *o;
      if (iiLocalRing[myynest]!=NULL) o=rFindHdl(iiLocalRing[myynest],NULL, NULL)->id;
      else                            o="none";
      if (currRing!=NULL)             n=rFindHdl(currRing,NULL, NULL)->id;
      else                            n="none";
      Werror("ring change during procedure call: %s -> %s",o,n);
      iiRETURNEXPR[myynest+1].CleanUp();
      err=TRUE;
    }
    currRing=iiLocalRing[myynest];
  }
  if ((currRing!=NULL) &&
    ((currRingHdl==NULL)||(IDRING(currRingHdl)!=currRing)
     ||(IDLEV(currRingHdl)>=myynest)))
  {
    rSetHdl(rFindHdl(currRing,NULL, NULL));
    iiLocalRing[myynest]=NULL;
  }
#else /* USE_IILOCALRING */
  if (procstack->cRing != currRing)
  {
    //if (procstack->cRingHdl!=NULL)
    //Print("procstack:%s,",IDID(procstack->cRingHdl));
    //if (currRingHdl!=NULL)
    //Print(" curr:%s\n",IDID(currRingHdl));
    //Print("pr:%x, curr: %x\n",procstack->cRing,currRing);
    if (((iiRETURNEXPR[myynest+1].Typ()>BEGIN_RING)
      && (iiRETURNEXPR[myynest+1].Typ()<END_RING))
    || ((iiRETURNEXPR[myynest+1].Typ()==LIST_CMD)
      && (lRingDependend((lists)iiRETURNEXPR[myynest+1].Data()))))
    {
      //idhdl hn;
      char *n;
      char *o;
      if (procstack->cRing!=NULL)
      {
        //PrintS("reset ring\n");
        procstack->cRingHdl=rFindHdl(procstack->cRing,NULL, NULL);
        #ifdef HAVE_NS
        if (procstack->cRingHdl==NULL)
          procstack->cRingHdl=
           rFindHdl(procstack->cRing,NULL,procstack->currPack->idroot);
        if (procstack->cRingHdl==NULL)
          procstack->cRingHdl=
           rFindHdl(procstack->cRing,NULL,basePack->idroot);
        #endif
        o=IDID(procstack->cRingHdl);
        currRing=procstack->cRing;
        currRingHdl=procstack->cRingHdl;
      }
      else                            o="none";
      if (currRing!=NULL)             n=IDID(currRingHdl);
      else                            n="none";
      if (currRing==NULL)
      {
        Werror("ring change during procedure call: %s -> %s",o,n);
        iiRETURNEXPR[myynest+1].CleanUp();
        err=TRUE;
      }
    }
    if (procstack->cRingHdl!=NULL)
    {
      rSetHdl(procstack->cRingHdl);
    }
    else
    { currRingHdl=NULL; currRing=NULL; }
  }
#endif /* USE_IILOCALRING */
  if (iiCurrArgs!=NULL)
  {
    if (!err) Warn("too many arguments for %s",IDID(pn));
    iiCurrArgs->CleanUp();
    omFreeBin((ADDRESS)iiCurrArgs, sleftv_bin);
    iiCurrArgs=NULL;
  }
  procstack->pop();
  if (err)
    return NULL;
  return &iiRETURNEXPR[myynest+1];
}

/*2
* start an example (as a proc),
* destroys the string 'example'
*/
BOOLEAN iiEStart(char* example, procinfo *pi)
{
  BOOLEAN err;
  int old_echo=si_echo;

  newBuffer( example, BT_example, pi,
             (pi != NULL ? pi->data.s.example_lineno: 0));

  iiCheckNest();
  procstack->push(example);
#ifdef USE_IILOCALRING
  iiLocalRing[myynest]=currRing;
#endif
  if (traceit&TRACE_SHOW_PROC)
  {
    if (traceit&TRACE_SHOW_LINENO) printf("\n");
    printf("entering example (level %d)\n",myynest);
  }
  myynest++;
  err=yyparse();
  if (sLastPrinted.rtyp!=0)
  {
    sLastPrinted.CleanUp();
    //memset(&sLastPrinted,0,sizeof(sleftv)); //done by CleanUp
  }
  killlocals(myynest);
  myynest--;
  si_echo=old_echo;
  if (traceit&TRACE_SHOW_PROC)
  {
    if (traceit&TRACE_SHOW_LINENO) printf("\n");
    printf("leaving  -example- (level %d)\n",myynest);
  }
#ifdef USE_IILOCALRING
  if (iiLocalRing[myynest] != currRing)
  {
    if (iiLocalRing[myynest]!=NULL)
    {
      rSetHdl(rFindHdl(iiLocalRing[myynest],NULL, NULL));
      iiLocalRing[myynest]=NULL;
    }
    else
    {
      currRingHdl=NULL;
      currRing=NULL;
    }
  }
#else /* USE_IILOCALRING */
#endif /* USE_IILOCALRING */
  if (NS_LRING != currRing)
  {
    if (NS_LRING!=NULL)
    {
      idhdl rh=procstack->cRingHdl;
      if ((rh==NULL)||(IDRING(rh)!=NS_LRING))
        rh=rFindHdl(NS_LRING,NULL, NULL);
      rSetHdl(rh);
    }
    else
    {
      currRingHdl=NULL;
      currRing=NULL;
    }
  }
//#endif /* USE_IILOCALRING */
  procstack->pop();
  return err;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
static BOOLEAN iiLoadLIB(FILE *fp, char *libnamebuf, char *newlib,
                         idhdl pl, BOOLEAN autoexport, BOOLEAN tellerror);

BOOLEAN iiTryLoadLib(leftv v, char *id)
{
  BOOLEAN LoadResult = TRUE;
#ifdef HAVE_NAMESPACES
  char libnamebuf[128];
  char *libname = (char *)omAlloc(strlen(id)+5);
  char *suffix[] = { "", ".lib", ".so", ".sl", NULL };
  int i = 0;
  FILE *fp;
  package pack;
  idhdl packhdl;
  lib_types LT;

  for(i=0; suffix[i] != NULL; i++)
  {
    sprintf(libname, "%s%s", id, suffix[i]);
    *libname = mytolower(*libname);
    if((LT = type_of_LIB(libname, libnamebuf)) > LT_NOTFOUND)
    {
      char *s=omStrDup(libname);
      char libnamebuf[256];

      if (LT==LT_SINGULAR)
        LoadResult = iiLibCmd(s, FALSE);
      else if ((LT==LT_ELF) || (LT==LT_HPUX))
        LoadResult = load_modules(s,libnamebuf,FALSE);
      if(!LoadResult )
      {
        v->name = iiConvName(libname);
        break;
      }
    }
  }
  omFree(libname);
#else /* HAVE_NAMESPACES */
#endif /* HAVE_NAMESPACES */
  return LoadResult;
}

BOOLEAN iiReLoadLib(idhdl packhdl)
{
  BOOLEAN LoadResult = TRUE;
#ifdef HAVE_NAMESPACES
  char libnamebuf[128];
  package pack = IDPACKAGE(packhdl);

  if(pack->language == LANG_NONE) return FALSE;

  FILE * fp = feFopen( pack->libname, "r", libnamebuf, FALSE);
  if (fp==NULL)
  {
    return TRUE;
  }
  namespaceroot->push(IDPACKAGE(packhdl), IDID(packhdl));
  LoadResult = iiLoadLIB(fp, libnamebuf, IDPACKAGE(packhdl)->libname,
                         packhdl, FALSE, FALSE);
  namespaceroot->pop();
#else /* HAVE_NAMESPACES */
#endif /* HAVE_NAMESPACES */
  return LoadResult;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/* check, if library lib has already been loaded
   if yes, writes filename of lib into where and returns TRUE,
      no, returns FALSE
*/
BOOLEAN iiLocateLib(const char* lib, char* where)
{
  idhdl hl;

  char *p;

  hl = IDROOT->get("LIB", 0);
  if (hl == NULL || (p=strstr(IDSTRING(hl), lib)) == NULL) return FALSE;
  if ((p!=IDSTRING(hl)) && (*(p-1)!=',')) return FALSE;

  if (strstr(IDSTRING(hl), ",") == NULL)
  {
    strcpy(where, IDSTRING(hl));
  }
  else
  {
    char* tmp = omStrDup(IDSTRING(hl));
    char* tok = strtok(tmp, ",");
    do
    {
      if (strstr(tok, lib) != NULL) break;
      tok = strtok(NULL, ",");
    }
    while (tok != NULL);
    assume(tok != NULL);
    strcpy(where, tok);
    omFree(tmp);
  }
  return TRUE;
}

BOOLEAN iiLibCmd( char *newlib, BOOLEAN tellerror )
{
  char buf[256];
  char libnamebuf[128];
  idhdl h;
  BOOLEAN LoadResult = TRUE;
#ifdef HAVE_NS
  idhdl pl;
#endif
  idhdl hl;
  int lines = 1;
  long pos = 0L;
  procinfov pi;
#ifdef HAVE_NS
  char *plib = iiConvName(newlib);
#endif
  FILE * fp = feFopen( newlib, "r", libnamebuf, tellerror );
  if (fp==NULL)
  {
    return TRUE;
  }
#ifndef HAVE_NS
  hl = idroot->get("LIB",0);
  if (hl==NULL)
  {
    hl = enterid( "LIB",0, STRING_CMD, &idroot, FALSE );
    IDSTRING(hl) = omStrDup(newlib);
  }
  else
  {
#ifdef TEST
    if (IDSTRING(hl) != NULL)
#endif
    {
      char *s = (char *)omAlloc( strlen(newlib) + strlen(IDSTRING(hl)) + 2 );
      strcpy(s,IDSTRING(hl));
      BOOLEAN f=FALSE;
      if(strchr(s,',')==NULL)
      {
        if (strcmp(s,newlib)==0)
          f=TRUE;
      }
      else
      {
        char *p=strtok(s,",");
        do
        {
          if(strcmp(p,newlib)==0)
          {
            f=TRUE;
            break;
          }
          p=strtok(NULL,",");
        } while (p!=NULL);
      }
      if (f)
        omFree((ADDRESS)s);
      else
      {
        sprintf( s, "%s,%s", IDSTRING(hl), newlib);
        omFree((ADDRESS)IDSTRING(hl));
        IDSTRING(hl) = s;
      }
    }
#ifdef TEST
    else
    {
      PrintS("## empty LIB string\n");
      IDSTRING(hl) = omStrDup(newlib);
    }
#endif
  }
#endif /* HAVE_NS */
#ifdef HAVE_TCL
  if (tclmode)
  {
    PrintTCLS('L',newlib);
  }
#endif
#ifdef HAVE_NS
  pl = basePack->idroot->get(plib,0);
  if (pl==NULL)
  {
    pl = enterid( plib,0, PACKAGE_CMD,
                  &(basePack->idroot), TRUE );
    IDPACKAGE(pl)->language = LANG_SINGULAR;
    IDPACKAGE(pl)->libname=omStrDup(newlib);
  }
  else
  {
    if(IDTYP(pl)!=PACKAGE_CMD)
    {
      WarnS("not of typ package.");
      fclose(fp);
      return TRUE;
    }
  }
  LoadResult = iiLoadLIB(fp, libnamebuf, newlib, pl, TRUE, tellerror);
#else /* HAVE_NS */
  LoadResult = iiLoadLIB(fp, libnamebuf, newlib, NULL, FALSE, tellerror);
#endif /* HAVE_NS */

  omFree((ADDRESS)newlib);

#ifdef HAVE_NS
  if(!LoadResult) IDPACKAGE(pl)->loaded = TRUE;
  omFree((ADDRESS)plib);
#endif /* HAVE_NS */

 return LoadResult;
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
static void iiCleanProcs(idhdl &root)
{
  idhdl prev=NULL;
  loop
  {
    if (root==NULL) return;
    if (IDTYP(root)==PROC_CMD)
    {
      procinfo *pi=(procinfo*)IDDATA(root);
      if ((pi->language == LANG_SINGULAR)
      && (pi->data.s.body_start == 0L))
      {
        // procinfo data incorrect:
        // - no proc body can start at the beginning of the file
        killhdl(root);
        if (prev==NULL)
          root=IDROOT;
        else
        {
          root=prev;
          prev=NULL;
        }
        continue;
      }
    }
    prev=root;
    root=IDNEXT(root);
  }
}
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
static BOOLEAN iiLoadLIB(FILE *fp, char *libnamebuf, char*newlib,
             idhdl pl, BOOLEAN autoexport, BOOLEAN tellerror)
{
  char buf[256];
  extern FILE *yylpin;
  libstackv ls_start = library_stack;
  lib_style_types lib_style;

  yylpin = fp;
  #if YYLPDEBUG > 1
  print_init();
  #endif
  extern int lpverbose;
  if (BVERBOSE(V_DEBUG_LIB)) lpverbose=1;
  else lpverbose=0;
  #ifdef HAVE_NS
    yylplex(newlib, libnamebuf, &lib_style, pl, autoexport);
  #else
    yylplex(newlib, libnamebuf, &lib_style);
  #endif /* HAVE_NS */
  if(yylp_errno)
  {
    Werror("Library %s: ERROR occured: in line %d, %d.", newlib, yylplineno,
         current_pos(0));
    if(yylp_errno==YYLP_BAD_CHAR)
    {
      Werror(yylp_errlist[yylp_errno], *text_buffer, yylplineno);
      omFree((ADDRESS)text_buffer);
      text_buffer=NULL;
    }
    else
      Werror(yylp_errlist[yylp_errno], yylplineno);
    Werror("Cannot load library,... aborting.");
    reinit_yylp();
    fclose( yylpin );
    iiCleanProcs(IDROOT);
    return TRUE;
  }
  if (BVERBOSE(V_LOAD_LIB))
    Print( "// ** loaded %s %s\n", libnamebuf, text_buffer);
  if( (lib_style == OLD_LIBSTYLE) && (BVERBOSE(V_LOAD_LIB)))
  {
    Warn( "library %s has old format. This format is still accepted,", newlib);
    Warn( "but for functionality you may wish to change to the new");
    Warn( "format. Please refer to the manual for further information.");
  }
  reinit_yylp();
  fclose( yylpin );
  fp = NULL;

  {
    libstackv ls;
    for(ls = library_stack; (ls != NULL) && (ls != ls_start); )
    {
      if(ls->to_be_done)
      {
        ls->to_be_done=FALSE;
        iiLibCmd(ls->get());
        ls = ls->pop(newlib);
      }
    }
#if 0
    PrintS("--------------------\n");
    for(ls = library_stack; ls != NULL; ls = ls->next)
    {
      Print("%s: LIB-stack:(%d), %s %s\n", newlib, ls->cnt, ls->get(),
        ls->to_be_done ? "not loaded" : "loaded");
    }
    PrintS("--------------------\n");
#endif
  }

  if(fp != NULL) fclose(fp);
  return FALSE;
}


/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
procinfo *iiInitSingularProcinfo(procinfov pi, char *libname, char *procname,
                                 int line, long pos, BOOLEAN pstatic)
{
  pi->libname = omStrDup(libname);

  if( strcmp(procname,"_init")==0)
  {
    pi->procname = iiConvName(libname);
  }
  else
    pi->procname = omStrDup(procname);
  pi->language = LANG_SINGULAR;
  pi->ref = 1;
#ifdef HAVE_NS
  pi->pack = NULL;
#endif
  pi->is_static = pstatic;
  pi->data.s.proc_start = pos;
  pi->data.s.def_end    = 0L;
  pi->data.s.help_start = 0L;
  pi->data.s.help_end   = 0L;
  pi->data.s.body_start = 0L;
  pi->data.s.body_end   = 0L;
  pi->data.s.example_start = 0L;
  pi->data.s.proc_lineno = line;
  pi->data.s.body_lineno = 0;
  pi->data.s.example_lineno = 0;
  pi->data.s.body = NULL;
  pi->data.s.help_chksum = 0;
  return(pi);
}

#ifdef HAVE_DYNAMIC_LOADING
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
int iiAddCproc(char *libname, char *procname, BOOLEAN pstatic,
               BOOLEAN(*func)(leftv res, leftv v))
{
  procinfov pi;
  idhdl h;

  h = enterid(procname,0, PROC_CMD, &IDROOT, TRUE);
  if ( h!= NULL )
  {
    pi = IDPROC(h);
    pi->libname = omStrDup(libname);
    pi->procname = omStrDup(procname);
    pi->language = LANG_C;
    pi->ref = 1;
    pi->is_static = pstatic;
    pi->data.o.function = func;
    return(1);
  }
  else
  {
    PrintS("iiAddCproc: failed.\n");
  }
  return(0);
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
BOOLEAN load_modules(char *newlib, char *fullname, BOOLEAN tellerror)
{
  int iiAddCproc(char *libname, char *procname, BOOLEAN pstatic,
                 BOOLEAN(*func)(leftv res, leftv v));
  typedef int (*fktn_t)(int(*iiAddCproc)(char *libname, char *procname,
                               BOOLEAN pstatic,
                               BOOLEAN(*func)(leftv res, leftv v)));
  fktn_t fktn;
  idhdl pl;
  char *plib = iiConvName(newlib);
  BOOLEAN RET=TRUE;
  int token;
  char FullName[256];

  memset(FullName,0,256);

  if( *fullname != '/' &&  *fullname != '.' )
    sprintf(FullName, "./%s", newlib);
  else strncpy(FullName, fullname,255);


  if(IsCmd(plib, token))
  {
    Werror("'%s' is resered identifier\n", plib);
    goto load_modules_end;
  }
  pl = IDROOT->get(plib,0);
  if (pl==NULL)
  {
    pl = enterid( plib,0, PACKAGE_CMD, &IDROOT,
                  TRUE );
    IDPACKAGE(pl)->language = LANG_C;
    IDPACKAGE(pl)->libname=omStrDup(newlib);
  }
  else
  {
    if(IDTYP(pl)!=PACKAGE_CMD)
    {
      Warn("not of typ package.");
      goto load_modules_end;
    }
  }
  if((IDPACKAGE(pl)->handle=dynl_open(FullName))==(void *)NULL)
  {
    Werror("dynl_open failed:%s", dynl_error());
    Werror("%s not found", newlib);
    goto load_modules_end;
  }
  else
  {
#ifdef HAVE_NS
    package s=currPack;
    currPack=IDPACKAGE(pl);
#endif
    fktn = (fktn_t)dynl_sym(IDPACKAGE(pl)->handle, "mod_init");
    if( fktn!= NULL) (*fktn)(iiAddCproc);
    else Werror("mod_init: %s\n", dynl_error());
    if (BVERBOSE(V_LOAD_LIB)) Print( "// ** loaded %s \n", fullname);
#ifdef HAVE_NS
    currPack->loaded=1;
    currPack=s;
#endif
  }
  RET=FALSE;

  load_modules_end:
  return RET;
}
#endif /* HAVE_DYNAMIC_LOADING */

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
char mytoupper(char c)
{
  if(c>=97 && c<=(97+26)) c-=32;
  return(c);
}

char mytolower(char c)
{
  if(c>=65 && c<=(65+26)) c+=32;
  return(c);
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
//#if defined(WINNT)
//#  define  FS_SEP '\\'
//#elif defined(macintosh)
//#  define FS_SEP ','
//#else
//#  define FS_SEP '/'
//#endif

static char *iiConvName(const char *libname)
{
  char *tmpname = omStrDup(libname);
  char *p = strrchr(tmpname, DIR_SEP);
  char *r;
  if(p==NULL) p = tmpname;
  else p++;
  r = strchr(p, '.');
  if( r!= NULL) *r = '\0';
  r = omStrDup(p);
  *r = mytoupper(*r);
  // printf("iiConvName: '%s' '%s' => '%s'\n", libname, tmpname, r);
  omFree((ADDRESS)tmpname);

  return(r);
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
void piShowProcList()
{
  idhdl h;
  procinfo *proc;
  char *name;

  Print( "%-15s  %20s      %s,%s  %s,%s   %s,%s\n", "Library", "function",
         "line", "start", "line", "body", "line", "example");
  for(h = IDROOT; h != NULL; h = IDNEXT(h))
  {
    if(IDTYP(h) == PROC_CMD)
    {
      proc = IDPROC(h);
      if(strcmp(proc->procname, IDID(h))!=0)
      {
        name = (char *)omAlloc(strlen(IDID(h))+strlen(proc->procname)+4);
        sprintf(name, "%s -> %s", IDID(h), proc->procname);
        Print( "%d %-15s  %20s ", proc->is_static ? 1 : 0, proc->libname, name);
        omFree((ADDRESS)name);
      }
      else
        Print( "%d %-15s  %20s ", proc->is_static ? 1 : 0, proc->libname,
               proc->procname);
      if(proc->language==LANG_SINGULAR)
        Print("line %4d,%-5ld  %4d,%-5ld  %4d,%-5ld\n",
              proc->data.s.proc_lineno, proc->data.s.proc_start,
              proc->data.s.body_lineno, proc->data.s.body_start,
              proc->data.s.example_lineno, proc->data.s.example_start);
      else if(proc->language==LANG_C)
        Print("type: object\n");
    }
  }
}

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
//char *iiLineNo(char *procname, int lineno)
//{
//  char buf[256];
//  idhdl pn = ggetid(procname);
//  procinfo *pi = IDPROC(pn);
//
//  sprintf(buf, "%s %3d\0", procname, lineno);
//  //sprintf(buf, "%s::%s %3d\0", pi->libname, pi->procname,
//  //  lineno + pi->data.s.body_lineno);
//  return(buf);
//}
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
#ifdef HAVE_LIBPARSER
void libstack::push(char *p, char *libname)
{
  libstackv lp;
  if( !iiGetLibStatus(libname))
  {
    for(lp = this;lp!=NULL;lp=lp->next)
    {
      if(strcmp(lp->get(), libname)==0) break;
    }
    if(lp==NULL)
    {
      libstackv ls = (libstack *)omAlloc0Bin(libstack_bin);
      ls->next = this;
      ls->libname = omStrDup(libname);
      ls->to_be_done = TRUE;
      if(this != NULL) ls->cnt = this->cnt+1; else ls->cnt = 0;
      library_stack = ls;
    }
  }
}

libstackv libstack::pop(char *p)
{
  libstackv ls = this;
  //omFree((ADDRESS)ls->libname);
  library_stack = ls->next;
  omFreeBin((ADDRESS)ls,  libstack_bin);
  return(library_stack);
}

#endif /* HAVE_LIBPARSER */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
#ifndef HOWMANY
# define HOWMANY 8192           /* how much of the file to look at */
#endif

lib_types type_of_LIB(char *newlib, char *libnamebuf)
{
  char        buf[HOWMANY+1];        /* one extra for terminating '\0' */
  struct stat sb;
  int nbytes = 0;
  int ret;
  lib_types LT=LT_NONE;

  FILE * fp = feFopen( newlib, "r", libnamebuf, FALSE );
  ret = stat(libnamebuf, &sb);

  if (fp==NULL)
  {
    return LT_NOTFOUND;
  }
  if((sb.st_mode & S_IFMT) != S_IFREG)
  {
    goto lib_type_end;
  }
  if ((nbytes = fread((char *)buf, sizeof(char), HOWMANY, fp)) == -1)
  {
    goto lib_type_end;
    /*NOTREACHED*/
  }
  if (nbytes == 0)
    goto lib_type_end;
  else
  {
    buf[nbytes++] = '\0';        /* null-terminate it */
  }
  if( (strncmp(buf, "\177ELF\01\01\01", 7)==0) && buf[16]=='\03')
  {
    LT = LT_ELF;
    //omFree(newlib);
    //newlib = omStrDup(libnamebuf);
    goto lib_type_end;
  }
  if( (strncmp(buf, "\02\020\01\016\05\022@", 7)==0))
  {
    LT = LT_HPUX;
    //omFree(newlib);
    //newlib = omStrDup(libnamebuf);
    goto lib_type_end;
  }
  if(isprint(buf[0]) || buf[0]=='\n')
  { LT = LT_SINGULAR; goto lib_type_end; }

  lib_type_end:
  fclose(fp);
  return LT;
}
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
