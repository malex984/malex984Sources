/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/* $Id: febase.cc,v 1.96 2002-11-26 13:34:21 Singular Exp $ */
/*
* ABSTRACT: i/o system
*/

/* I need myfread in standalone_parser */
#ifndef STANDALONE_PARSER
#include "mod2.h"

#include <stdlib.h>
#include <stdio.h>
#include <mylimits.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <ctype.h>
#ifndef __MWERKS__
#include <unistd.h>
#endif
#ifdef NeXT
#include <sys/file.h>
#endif

#ifdef HAVE_PWD_H
#include "pwd.h"
#endif

#include "tok.h"
#include "febase.h"
#include "omalloc.h"
#include "subexpr.h"
#include "ipshell.h"
#include "dError.h"
#include "sdb.h"

#define fePutChar(c) fputc((uchar)(c),stdout)
/*0 implementation */

char fe_promptstr[]
#ifdef macintosh
                   =" \n";
#else
                   ="  ";
#endif

#define INITIAL_PRINT_BUFFER 24*1024
static int feBufferLength=INITIAL_PRINT_BUFFER;
static char * feBuffer=(char *)omAlloc(INITIAL_PRINT_BUFFER);

int     si_echo = 0;
int     printlevel = 0;
#ifndef macintosh
int     pagelength = 24;
#else
int     pagelength = -1;
#endif
int     colmax = 80;
char    prompt_char = '>'; /*1 either '>' or '.'*/
extern "C" {
BITSET  verbose = 1
                  | Sy_bit(V_REDEFINE)
                  | Sy_bit(V_LOAD_LIB)
                  | Sy_bit(V_SHOW_USE)
                  | Sy_bit(V_PROMPT)
/*                  | Sy_bit(V_DEBUG_MEM) */
;}
BOOLEAN errorreported = FALSE;
char *  feErrors=NULL;
int     feErrorsLen=0;
BOOLEAN feWarn = TRUE;
BOOLEAN feOut = TRUE;

#ifdef macintosh
static  int lines = 0;
static  int cols = 0;
#endif

const char feNotImplemented[]="not implemented";

BOOLEAN feProt = FALSE;
FILE*   feProtFile;
BOOLEAN tclmode=FALSE;
/* TCL-Protocoll (Singular -x): <char type>:<int length>:<string> \n
*  E:l:s  error
*  W:l:s  warning
*  N:l:s  stdout
*  Q:0:   quit
*  P:l:   prompt > (ring defined)
*  U:l:   prompt > (no ring defined)
*  P:l:   prompt .
*  R:l:<ring-name> ring change
*  L:l:<lib name> library loaded
*  O:l:<list of options(space seperated)> option change
*  M:l:<mem-usage> output from "option(mem)"
*/


/**************************************************************************
* handling of 'voices'
**************************************************************************/

extern int blocknest; /* scaner.l internal */

int    yy_noeof=0;     // the scanner "state"
int    yy_blocklineno; // to get the lineno of the block start from scanner
Voice  *currentVoice = NULL;
FILE   *feFilePending; /*temp. storage for grammar.y */

static char * BT_name[]={"BT_none","BT_break","BT_proc","BT_example",
                       "BT_file","BT_execute","BT_if","BT_else"};
/*2
* the name of the current 'Voice': the procname (or filename)
*/
const char * VoiceName()
{
  if ((currentVoice!=NULL)
  && (currentVoice->filename!=NULL))
    return currentVoice->filename;
  return sNoName;
}

/*2
* the calling chain of Voices
*/
void VoiceBackTrack()
{
  Voice *p=currentVoice;
  while (p->prev!=NULL)
  {
    p=p->prev;
    char *s=p->filename;
    if (s==NULL)
      PrintS("-- called from ? --\n");
    else
      Print("-- called from %s --\n",s);
  }
}

/*2
* init a new voice similiar to the current
*/
void Voice::Next()
{
  Voice *p=new Voice;
  // OB: ???
  // Hmm... when Singular is used as batch file
  // then this voice is never freed
  omMarkAsStaticAddr(p);
  if (currentVoice != NULL)
  {
    currentVoice->curr_lineno=yylineno;
    currentVoice->next=p;
  }
  p->prev=currentVoice;
  currentVoice=p;
  //Print("Next:");
}

feBufferTypes Voice::Typ()
{
  switch(typ)
  {
    case BT_proc:
    case BT_example:
    case BT_file:
      return typ;
    default:
      if (prev==NULL) return (feBufferTypes)0;
      return prev->Typ();
  }
}

/*2
* start the file 'fname' (STDIN is stdin) as a new voice (cf.VFile)
* return FALSE on success, TRUE if an error occurs (file cannot be opened)
*/
BOOLEAN newFile(char *fname,FILE* f)
{
  currentVoice->Next();
  //Print(":File%d(%s):%s(%x)\n",
  //  currentVoice->typ,BT_name[currentVoice->typ],fname,currentVoice);
  currentVoice->filename   = omStrDup(fname);
  omMarkAsStaticAddr(currentVoice->filename);
  if (strcmp(fname,"STDIN") == 0)
  {
    currentVoice->files = stdin;
    currentVoice->sw = BI_stdin;
    currentVoice->start_lineno = 1;
  }
  else
  {
    currentVoice->sw = BI_file; /* needed by exitVoice below */
    if (f!=NULL)
      currentVoice->files = f;
    else
    {
      currentVoice->files = feFopen(fname,"r",NULL,TRUE);
      if (currentVoice->files==NULL)
      {
        exitVoice();
        return TRUE;
      }
    }
    currentVoice->start_lineno = 0;
  }
  yylineno=currentVoice->start_lineno;
  //Voice *p=currentVoice;
  //Print("-----------------\ncurr:");
  //do
  //{
  //Print("voice fn:%s\n",p->filename);
  //p=p->prev;
  //}
  //while (p!=NULL);
  //Print("----------------\n");
  return FALSE;
}

void newBuffer(char* s, feBufferTypes t, procinfo* pi, int lineno)
{
  currentVoice->Next();
  //Print(":Buffer%d(%s):%s(%x)\n",
  //  t,BT_name[t],pname,currentVoice);
  if (pi!=NULL)
  {
    int l=strlen(pi->procname);
    if (pi->libname!=NULL) l+=strlen(pi->libname);
    currentVoice->filename = (char *)omAlloc(l+3);
    *currentVoice->filename='\0';
    if (pi->libname!=NULL) strcat(currentVoice->filename,pi->libname);
    strcat(currentVoice->filename,"::");
    strcat(currentVoice->filename,pi->procname);
    currentVoice->pi       = pi;
  }
  else
  {
    currentVoice->filename = omStrDup(currentVoice->prev->filename);
    currentVoice->pi       = currentVoice->prev->pi;
  }
  currentVoice->buffer   = s;
  currentVoice->sw       = BI_buffer;
  currentVoice->typ      = t;
  switch (t)
  {
    case BT_execute:
                     yylineno-=2;
                     break;
    case BT_proc:
    case BT_example:
                     currentVoice->oldb=myynewbuffer();
                     yylineno = lineno+1;
                     break;
    case BT_if:
    case BT_else:
    case BT_break:
                     yylineno = yy_blocklineno-1;
                     break;
    //case BT_file:
    default:
                     yylineno = 1;
                     break;
  }
  //Print("start body (%s) at line %d\n",BT_name[t],yylineno);
  currentVoice->start_lineno = yylineno;
  //printf("start buffer typ %d\n",t);
  //Voice *p=currentVoice;
  //Print("-----------------\ncurr:");
  //do
  //{
  //Print("voice fn:%s\n",p->filename);
  //p=p->prev;
  //}
  //while (p!=NULL);
  //Print("----------------\n");
}

/*2
* exit Buffer of type 'typ':
* returns 1 if buffer type could not be found
*/
BOOLEAN exitBuffer(feBufferTypes typ)
{
  //printf("exitBuffer: %d(%s),(%x)\n",
  //  typ,BT_name[typ], currentVoice);
  //Voice *p=currentVoice;
  //Print("-----------------\ncurr:");
  //do
  //{
  //Print("voice fn:%s\n",p->filename);
  //p=p->prev;
  //}
  //while (p!=NULL);
  //Print("----------------\n");
  if (typ == BT_break)  // valid inside for, while. may skip if, else
  {
    /*4 first check for valid buffer type, skip if/else*/
    Voice *p=currentVoice;
    loop
    {
      if ((p->typ != BT_if)
      &&(p->typ != BT_else))
      {
        if (p->typ == BT_break /*typ*/)
        {
          while (p != currentVoice)
          {
            exitVoice();
          }
          exitVoice();
          return FALSE;
        }
        else return TRUE;
      }
      if (p->prev==NULL) break;
      p=p->prev;
    }
    /*4 break not inside a for/while: return an error*/
    if (/*typ*/ BT_break != currentVoice->typ) return 1;
    return exitVoice();
  }

  if ((typ == BT_proc)
  || (typ == BT_example))
  {
    Voice *p=currentVoice;
    loop
    {
      if ((p->typ == BT_proc)
      || (p->typ == BT_example))
      {
        while (p != currentVoice)
        {
          exitVoice();
        }
        exitVoice();
        return FALSE;
      }
      if (p->prev==NULL) break;
      p=p->prev;
    }
  }
  /*4 return not inside a proc: return an error*/
  return TRUE;
}

/*2
* jump to the beginning of a buffer
*/
BOOLEAN contBuffer(feBufferTypes typ)
{
  //printf("contBuffer: %d(%s),(%x)\n",
  //  typ,BT_name[typ], currentVoice);
  if (typ == BT_break)  // valid inside for, while. may skip if, else
  {
    // first check for valid buffer type
    Voice *p=currentVoice;
    loop
    {
      if ((p->typ != BT_if)
        &&(p->typ != BT_else))
      {
        if (p->typ == BT_break /*typ*/)
        {
          while (p != currentVoice)
          {
            exitVoice();
          }
          yylineno = currentVoice->start_lineno;
          currentVoice->fptr=0;
          return FALSE;
        }
        else return TRUE;
      }
      if (p->prev==NULL) break;
      p=p->prev;
    }
  }
  return TRUE;
}

/*2
* leave a voice: kill local variables
* setup everything from the previous level
* return 1 if leaving the top level, 0 otherwise
*/
BOOLEAN exitVoice()
{
  //printf("exitVoice: %d(%s),(%x)\n",
  //  currentVoice->typ,BT_name[currentVoice->typ], currentVoice);
  //{
  //Voice *p=currentVoice;
  //Print("-----------------\ncurr:");
  //do
  //{
  //Print("voice fn:%s\n",p->filename);
  //p=p->prev;
  //}
  //while (p!=NULL);
  //Print("----------------\n");
  //}
  if (currentVoice!=NULL)
  {
    if (currentVoice->oldb!=NULL)
    {
      myyoldbuffer(currentVoice->oldb);
      currentVoice->oldb=NULL;
    }
    if ((currentVoice->prev==NULL)&&(currentVoice->sw==BI_file))
    {
      currentVoice->prev=feInitStdin(currentVoice);
    }
    if (currentVoice->prev!=NULL)
    {
      //printf("exitVoice typ %d(%s)\n",
      //  currentVoice->typ,BT_name[currentVoice->typ]);
      if (currentVoice->typ==BT_if)
      {
        currentVoice->prev->ifsw=2;
      }
      else
      {
        currentVoice->prev->ifsw=0;
      }
      if ((currentVoice->sw == BI_file)
      && (currentVoice->files!=NULL))
      {
        fclose(currentVoice->files);
      }
      if (currentVoice->filename!=NULL)
      {
        omFree((ADDRESS)currentVoice->filename);
        currentVoice->filename=NULL;
      }
      if (currentVoice->buffer!=NULL)
      {
        omFree((ADDRESS)currentVoice->buffer);
        currentVoice->buffer=NULL;
      }
      yylineno=currentVoice->prev->curr_lineno;
      currentVoice->prev->next=NULL;
    }
    Voice *p=currentVoice->prev;
    delete currentVoice;
    currentVoice=p;
  }
  return currentVoice==NULL;
}

/*2
* set prompt_char
* only called with currentVoice->sw == BI_stdin
*/
static void feShowPrompt(void)
{
  fe_promptstr[0]=prompt_char;
#ifdef macintosh
  cols = 0;
  printf(fe_promptstr);mflush();
#endif
}

/*2
* print echo (si_echo or TRACE), set my_yylinebuf
*/
static int fePrintEcho(char *anf, char *b)
{
  char *ss=strrchr(anf,'\n');
  int len_s;
  if (ss==NULL)
  {
    len_s=strlen(anf);
  }
  else
  {
    len_s=ss-anf+1;
  }
  // my_yylinebuf:
  int mrc=min(len_s,79)-1;
  strcpy(my_yylinebuf,anf+(len_s-1)-mrc);
  if (my_yylinebuf[mrc] == '\n') my_yylinebuf[mrc] = '\0';
  mrc--;
  // handle echo:
  if (((si_echo>myynest)
    && ((currentVoice->typ==BT_proc)
      || (currentVoice->typ==BT_example)
      || (currentVoice->typ==BT_file)
      || (currentVoice->typ==BT_none)
    )
    && (strncmp(anf,";return();",10)!=0)
   )
  || (traceit&TRACE_SHOW_LINE)
  || (traceit&TRACE_SHOW_LINE1))
  {
    if (currentVoice->typ!=BT_example)
    {
      if (currentVoice->filename==NULL)
        Print("(none) %3d%c ",yylineno,prompt_char);
      else
        Print("%s %3d%c ",currentVoice->filename,yylineno,prompt_char);
     }
    #ifdef HAVE_TCL
    if(tclmode)
    {
      PrintTCL('N',len_s,anf);
    }
    else
    #endif
    {
      fwrite(anf,1,len_s,stdout);
      mflush();
    }
    if (traceit&TRACE_SHOW_LINE)
    {
      #ifdef HAVE_TCL
      if(!tclmode)
      #endif
      while(fgetc(stdin)!='\n');
    }
  }
  else if (traceit&TRACE_SHOW_LINENO)
  {
    Print("{%d}",yylineno);
    mflush();
  }
#ifdef HAVE_SDB
  if ((blocknest==0)
  && (currentVoice->pi!=NULL)
  && (currentVoice->pi->trace_flag!=0))
  {
    sdb(currentVoice, anf, len_s);
  }
#endif
  prompt_char = '.';
  return len_s;
}

int feReadLine(char* b, int l)
{
  char *s=NULL;
  int offset = 0; /* will not be used if s==NULL*/
  // try to read from the buffer into b, max l chars
  if (currentVoice!=NULL)
  {
    if((currentVoice->buffer!=NULL)
    && (currentVoice->buffer[currentVoice->fptr]!='\0'))
    {
  NewBuff:
      register int i=0;
      long startfptr=currentVoice->fptr;
      long tmp_ptr=currentVoice->fptr;
      l--;
      loop
      {
        register char c=
        b[i]=currentVoice->buffer[tmp_ptr/*currentVoice->fptr*/];
        i++;
        if (yy_noeof==noeof_block)
        {
          if (c<' ')  yylineno++;
          else if (c=='}') break;
        }
        else
        {
          if ((c<' ') ||
          (c==';') ||
          (c==')')
          )
            break;
        }
        if (i>=l) break;
        tmp_ptr++;/*currentVoice->fptr++;*/
        if(currentVoice->buffer[tmp_ptr/*currentVoice->fptr*/]=='\0') break;
      }
      currentVoice->fptr=tmp_ptr;
      b[i]='\0';
      if (currentVoice->sw==BI_buffer)
      {
        if (startfptr==0)
        {
          char *anf=currentVoice->buffer;
          char *ss=strchr(anf,'\n');
          int len;
          if (ss==NULL) len=strlen(anf);
          else          len=ss-anf;
          char *s=(char *)omAlloc(len+2);
          strncpy(s,anf,len+2);
          s[len+1]='\0';
          fePrintEcho(s,b);
          omFree((ADDRESS)s);
        }
        else if (/*(startfptr>0) &&*/
        (currentVoice->buffer[startfptr-1]=='\n'))
        {
          char *anf=currentVoice->buffer+startfptr;
          char *ss=strchr(anf,'\n');
          int len;
          if (ss==NULL) len=strlen(anf);
          else          len=ss-anf;
          char *s=(char *)omAlloc(len+2);
          strncpy(s,anf,len+2);
          s[len+1]='\0';
          yylineno++;
          fePrintEcho(s,b);
          omFree((ADDRESS)s);
        }
      }
      currentVoice->fptr++;
      return i;
    }
    // no buffer there or e-o-buffer or eoln:
    if (currentVoice->sw!=BI_buffer)
    {
      currentVoice->fptr=0;
      if (currentVoice->buffer==NULL)
      {
        currentVoice->buffer=(char *)omAlloc(4096-sizeof(ADDRESS));
        omMarkAsStaticAddr(currentVoice->buffer);
      }
    }
    offset=0;
  NewRead:
    yylineno++;
    if (currentVoice->sw==BI_stdin)
    {
      feShowPrompt();
      s=fe_fgets_stdin(fe_promptstr,
                       &(currentVoice->buffer[offset]),
                       (4096-1-sizeof(ADDRESS))-offset);
      int i=0;
      if (s!=NULL)
        while((s[i]!='\0') && (i<4096)) {s[i] &= (char)127;i++;}
    }
    else if (currentVoice->sw==BI_file)
    {
      s=fgets(currentVoice->buffer+offset,(4096-1-sizeof(ADDRESS))-offset,
              currentVoice->files);
    }
    //else /* BI_buffer */ s==NULL  => return 0
    // done by the default return
  }
  if (s!=NULL)
  {
    // handle prot:
    if (feProt&PROT_I)
    {
      fputs(s,feProtFile);
    }
    int rc=fePrintEcho(s,b)+1;
    //s[strlen(s)+1]='\0'; add an second \0 at the end of the string
    s[rc]='\0';
    // handel \\ :
    rc-=3;
    if ((s[rc]=='\\')&&(currentVoice->sw!=BI_buffer))
    {
      s[rc]='\0';
      offset+=rc;
      if (offset<(int)omSizeOfAddr(currentVoice->buffer)) goto NewRead;
    }
    goto NewBuff;
  }
  /* else if (s==NULL) */
  {
    char *err;
    switch(yy_noeof)
    {
      case noeof_brace:
      case noeof_block:
        err="{...}";
        break;
      case noeof_asstring:
        err="till `.`";
        break;
      case noeof_string:
        err="string";
        break;
      case noeof_bracket:
        err="(...)";
        break;
      case noeof_procname:
        err="proc";
        break;
      case noeof_comment:
        err="/*...*/";
        break;
      default:
        return 0;
    }
    Werror("premature end of file while reading %s",err);
    return 0;
  }
}

#ifdef __MWERKS__
#ifdef __cplusplus
extern "C" {
#endif
#ifdef macintosh
int    isatty(int filedes);
#else
int    _isatty(int filedes);
#define isatty  _isatty
#endif /* macintosh */
#ifdef __cplusplus
}
#endif
#endif
/*2
* init all data structures
*/
#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#endif
Voice * feInitStdin(Voice *pp)
{
  Voice *p = new Voice;
  p->files = stdin;
  #ifdef HAVE_TCL
  p->sw = (tclmode || isatty(STDIN_FILENO)) ? BI_stdin : BI_file;
  #else
  p->sw = (isatty(STDIN_FILENO)) ? BI_stdin : BI_file;
  #endif
  if ((pp!=NULL) && (pp->files==stdin))
  {
    p->files=freopen("/dev/tty","r",stdin);
    stdin=p->files;
    p->sw = BI_stdin;
  }
  p->filename   = omStrDup("STDIN");
  p->start_lineno   = 1;
  omMarkAsStaticAddr(p);
  omMarkAsStaticAddr(p->filename);
  return p;
}
/*****************************************************************
 *
 * File handling
 *
 *****************************************************************/

FILE * feFopen(char *path, char *mode, char *where,int useWerror,
               int path_only)
{
  char longpath[MAXPATHLEN];
  if (path[0]=='~')
  {
    if (path[1] == DIR_SEP)
    {
      char* home = getenv("HOME");
#ifdef ix86_Win
      if ((home==NULL)||(!access(home,X_OK)))
        home = getenv("SINGHOME");
#endif
      if (home != NULL)
      {
        strcpy(longpath, home);
        strcat(longpath, &(path[1]));
        path = longpath;
      }
    }
#if defined(HAVE_PWD_H) && defined(HAVE_GETPWNAM)
    else
    {
      char* dir_sep;
      struct passwd *pw_entry;
      strcpy (longpath, path);
      dir_sep = strchr(longpath, DIR_SEP);
      *dir_sep = '\0';
      pw_entry = getpwnam(&longpath[1]);
      if (pw_entry != NULL)
      {
        strcpy(longpath, pw_entry->pw_dir);
        dir_sep = strchr(path, DIR_SEP);
        strcat(longpath, dir_sep);
        path = longpath;
      }
    }
#endif
  }
  FILE * f=NULL;
  if (! path_only)
  {
    struct stat statbuf;
    if ((stat(path,&statbuf)==0)
    && (S_ISREG(statbuf.st_mode)))
      f = myfopen(path,mode);
  }
  if (where!=NULL) strcpy(where,path);
  if ((*mode=='r') &&
      (path[0]!=DIR_SEP) &&
      ! (path[0] == '.' && path[1] == DIR_SEP) &&
      (f==NULL))
  {
    char found = 0;
    char* spath = feResource('s');
    char *s;

    if (where==NULL) s=(char *)omAlloc(250);
    else             s=where;

    if (spath!=NULL)
    {
      char *p,*q;
      p = spath;
      while( (q=strchr(p, fePathSep)) != NULL)
      {
        *q = '\0';
        strcpy(s,p);
        *q = fePathSep;
        strcat(s, DIR_SEPP);
        strcat(s, path);
        #ifndef macintosh
          if(!access(s, R_OK)) { found++; break; }
        #else
          f=fopen(s,mode); /* do not need myfopen: we test only the access */
          if (f!=NULL)  { found++; fclose(f); break; }
        #endif
        p = q+1;
      }
      if(!found)
      {
        strcpy(s,p);
        strcat(s, DIR_SEPP);
        strcat(s, path);
      }
      f=myfopen(s,mode);
      if (f!=NULL)
      {
        if (where==NULL) omFree((ADDRESS)s);
        return f;
      }
    }
    else
    {
      if (where!=NULL) strcpy(s/*where*/,path);
      f=myfopen(path,mode);
    }
    if (where==NULL) omFree((ADDRESS)s);
  }
  if ((f==NULL)&&(useWerror))
    Werror("cannot open `%s`",path);
  return f;
}

static char * feBufferStart;
  /* only used in StringSet(S)/StringAppend(S)*/
char * StringAppend(char *fmt, ...)
{
  va_list ap;
  char *s = feBufferStart; /*feBuffer + strlen(feBuffer);*/
  int more, vs;
  va_start(ap, fmt);
  if ((more=feBufferStart-feBuffer+strlen(fmt)+100)>feBufferLength)
  {
    more = ((more + (4*1024-1))/(4*1024))*(4*1024);
    int l=s-feBuffer;
    feBuffer=(char *)omReallocSize((ADDRESS)feBuffer,feBufferLength,
                                                     more);
    omMarkAsStaticAddr(feBuffer);
    feBufferLength=more;
    s=feBuffer+l;
#ifndef BSD_SPRINTF
    feBufferStart=s;
#endif
  }
#ifdef BSD_SPRINTF
  vsprintf(s, fmt, ap);
  while (*s!='\0') s++;
  feBufferStart =s;
#else
#ifdef HAVE_VSNPRINTF
  vs = vsnprintf(s, feBufferLength - (feBufferStart - feBuffer), fmt, ap);
  if (vs == -1)
  {
    assume(0);
    feBufferStart = feBuffer + feBufferLength -1;
  }
  else
  {
    feBufferStart += vs;
  }
#else
  feBufferStart += vsprintf(s, fmt, ap);
#endif
#endif
  omCheckAddrSize(feBuffer, feBufferLength);
  va_end(ap);
  return feBuffer;
}

char * StringAppendS(char *st)
{
  /* feBufferStart is feBuffer + strlen(feBuffer);*/
  int more,l;
  int ll=feBufferStart-feBuffer;
  if ((more=ll+2+(l=strlen(st)))>feBufferLength)
  {
    more = ((more + (4*1024-1))/(4*1024))*(4*1024);
    feBuffer=(char *)omReallocSize((ADDRESS)feBuffer,feBufferLength,
                                                     more);
    feBufferLength=more;
    feBufferStart=feBuffer+ll;
  }
  strcat(feBufferStart, st);
  feBufferStart +=l;
  return feBuffer;
}

char * StringSetS(char *st)
{
  int more,l;
  if ((l=strlen(st))>feBufferLength)
  {
    more = ((l + (4*1024-1))/(4*1024))*(4*1024);
    feBuffer=(char *)omReallocSize((ADDRESS)feBuffer,feBufferLength,
                                                     more);
    feBufferLength=more;
  }
  strcpy(feBuffer,st);
  feBufferStart=feBuffer+l;
  return feBuffer;
}

#ifndef __MWERKS__
#ifdef HAVE_TCL
extern "C" {
void PrintTCLS(const char c, const char *s)
{
  int l=strlen(s);
  if (l>0) PrintTCL(c,l,s);
}
}
#endif
#endif

extern "C" {
void WerrorS(const char *s)
{
#ifdef HAVE_MPSR
  if (fe_fgets_stdin==fe_fgets_dummy)
  {
    if (feErrors==NULL)
    {
      feErrors=(char *)omAlloc(256);
      feErrorsLen=256;
      *feErrors = '\0';
    }
    else
    {
      if (((int)(strlen((char *)s)+ 20 +strlen(feErrors)))>=feErrorsLen)
      {
        feErrors=(char *)omReallocSize(feErrors,feErrorsLen,feErrorsLen+256);
        feErrorsLen+=256;
      }
    }
    strcat(feErrors, "Singular error: ");
    strcat(feErrors, (char *)s);
  }
  else
#endif
  {
#ifdef HAVE_TCL
    if (tclmode)
    {
      PrintTCLS('E',(char *)s);
      PrintTCLS('E',"\n");
    }
    else
#endif
    {
      fwrite("   ? ",1,5,stderr);
      fwrite((char *)s,1,strlen((char *)s),stderr);
      fwrite("\n",1,1,stderr);
      fflush(stderr);
      if (feProt&PROT_O)
      {
        fwrite("   ? ",1,5,feProtFile);
        fwrite((char *)s,1,strlen((char *)s),feProtFile);
        fwrite("\n",1,1,feProtFile);
      }
    }
  }
  errorreported = TRUE;
}

void Werror(char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  char *s=(char *)omAlloc(256);
  vsprintf(s, fmt, ap);
  WerrorS(s);
  omFreeSize(s,256);
  va_end(ap);
}

void WarnS(const char *s)
{
  #define warn_str "// ** "
#ifdef HAVE_TCL
  if (tclmode)
  {
    PrintTCLS('W',warn_str);
    PrintTCLS('W',s);
    PrintTCLS('W',"\n");
  }
  else
#endif
  if (feWarn) /* ignore warnings if option --no-warn was given */
  {
    fwrite(warn_str,1,6,stdout);
    fwrite(s,1,strlen(s),stdout);
    fwrite("\n",1,1,stdout);
    fflush(stdout);
    if (feProt&PROT_O)
    {
      fwrite(warn_str,1,6,feProtFile);
      fwrite(s,1,strlen(s),feProtFile);
      fwrite("\n",1,1,feProtFile);
    }
  }
}
} /* end extern "C" */

void Warn(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  char *s=(char *)omAlloc(256);
  vsprintf(s, fmt, ap);
  WarnS(s);
  omFreeSize(s,256);
  va_end(ap);
}


#ifdef macintosh
static  int lines = 0;
static  int cols = 0;

void mwrite(uchar c)
{
  if (c == '\n')
  {
    cols = 0;
    if (lines == pagelength)
    {
      lines = 0;
      fputs("pause>\n",stderr);
      uchar c = fgetc(stdin);
    }
    else
    {
      lines++;
      fePutChar(c);
    }
  }
  else
  {
    fePutChar(c);
    cols++;
    if (cols == colmax)
    {
      // cols = 0;   //will be done by mwrite('\n');
      mwrite('\n');
    }
  }
}
#endif

// some routines which redirect the output of print to a string
static char* sprint = NULL;
void SPrintStart()
{
  sprint = omStrDup("");
}

static void SPrintS(char* s)
{
  omCheckAddr(sprint);
  if (s == NULL) return;
  int ls = strlen(s);
  if (ls == 0) return;

  char* ns;
  int l = strlen(sprint);
  ns = (char*) omAlloc((l + ls + 1)*sizeof(char));
  if (l > 0) strcpy(ns, sprint);

  strcpy(&(ns[l]), s);
  omFree(sprint);
  sprint = ns;
  omCheckAddr(sprint);
}

char* SPrintEnd()
{
  char* ns = sprint;
  sprint = NULL;
  omCheckAddr(ns);
  return ns;
}

// Print routines
extern "C" {
void PrintS(char *s)
{
  if (sprint != NULL)
  {
    SPrintS(s);
    return;
  }

  if (feOut) /* do not print when option --no-out was given */
  {

#ifdef macintosh
    char c;
    while ('\0' != (c = *s++))
    {
      mwrite(c);
    }
#else
#ifdef HAVE_TCL
    if (tclmode)
    {
      PrintTCLS('N',s);
    }
    else
#endif
    {
      fwrite(s,1,strlen(s),stdout);
      fflush(stdout);
      if (feProt&PROT_O)
      {
        fwrite(s,1,strlen(s),feProtFile);
      }
    }
#endif
  }
}

void PrintLn()
{
  PrintS("\n");
}

void Print(char *fmt, ...)
{
  if (sprint != NULL)
  {
    int ls = strlen(fmt);
    va_list ap;
    va_start(ap, fmt);
    omCheckAddr(sprint);
    if (fmt != NULL && ls > 0)
    {
      char* ns;
      int l = strlen(sprint);
      ns = (char*) omAlloc(sizeof(char)*(ls + l + 256));
      if (l > 0)  strcpy(ns, sprint);

#ifdef HAVE_VSNPRINTF
      l = vsnprintf(&(ns[l]), ls+255, fmt, ap);
      assume(l != -1);
#else
      vsprintf(&(ns[l]), fmt, ap);
#endif
      omCheckAddr(ns);
      omFree(sprint);
      sprint = ns;
    }
    va_end(ap);
    return;
  }
  if (feOut)
  {
    va_list ap;
    va_start(ap, fmt);
#ifdef HAVE_TCL
    if(tclmode)
#endif
#if (defined(HAVE_TCL) || defined(macintosh))
    {
      char *s=(char *)omAlloc(strlen(fmt)+256);
      vsprintf(s,fmt, ap);
#ifdef HAVE_TCL
      PrintTCLS('N',s);
#endif
#ifdef macintosh
      char c;
      while ('\0' != (c = *s++))
      {
        mwrite(c);
      }
      if (feProt&PROT_O)
      {
        vfprintf(feProtFile,fmt,ap);
      }
#endif
    }
#endif
#if !defined(macintosh) || defined(HAVE_TCL)
#ifdef HAVE_TCL
    else
#endif
    {
      vfprintf(stdout, fmt, ap);
      fflush(stdout);
      if (feProt&PROT_O)
      {
        vfprintf(feProtFile,fmt,ap);
      }
    }
#endif
    va_end(ap);
  }
}

/* end extern "C" */
}

void monitor(char* s, int mode)
{
  if (feProt)
  {
    fclose(feProtFile);
    feProt = 0;
  }
  if ((s!=NULL) && (*s!='\0'))
  {
    feProtFile = myfopen(s,"w");
    if (feProtFile==NULL)
    {
      Werror("cannot open %s",s);
      feProt=0;
    }
    else
      feProt = (BOOLEAN)mode;
  }
}


char* eati(char *s, int *i)
{
  int l=0;

  if    (*s >= '0' && *s <= '9')
  {
    *i = 0;
    while (*s >= '0' && *s <= '9')
    {
      *i *= 10;
      *i += *s++ - '0';
      l++;
      if ((l>=MAX_INT_LEN)||((*i) <0))
      {
        s-=l;
        Werror("`%s` greater than %d(max. integer representation)",
                s,MAX_INT_VAL);
        return s;
      }
    }
  }
  else *i = 1;
  return s;
}
#else /* ! STANDALONE_PARSER */
#include <stdio.h>

#endif

#ifdef ix86_Win
// Make sure that mode contains binary option
FILE* myfopen(char *path, char *mode)
{
  char mmode[4];
  int i;
  int done = 0;

  for (i=0;;i++)
  {
    mmode[i] = mode[i];
    if (mode[i] == '\0') break;
    if (mode[i] == 'b') done = 1;
  }

  if (! done)
  {
    mmode[i] = 'b';
    mmode[i+1] = '\0';
  }
  return fopen(path, mmode);
}
#endif
// replace "\r\n" by " \n" and "\r" by "\n"

size_t myfread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
  size_t got = fread(ptr, size, nmemb, stream) * size;
  size_t i;

  for (i=0; i<got; i++)
  {
    if ( ((char*) ptr)[i] == '\r')
    {
      if (i+1 < got && ((char*) ptr)[i+1] == '\n')
        ((char*) ptr)[i] = ' ';
      else
        ((char*) ptr)[i] = '\n';
    }
  }
  return got;
}
