/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/* $Id: tesths.cc,v 1.76 1999-09-29 10:59:43 obachman Exp $ */

/*
* ABSTRACT - initialize SINGULARs components, run Script and start SHELL
*/

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include "mod2.h"
#include "tok.h"
#include "ipshell.h"
#include "sing_dld.h"
#include "febase.h"
#include "cntrlc.h"
#include "mmemory.h"
#include "silink.h"
#include "ipid.h"
#include "timer.h"
#include "sdb.h"
#include "fegetopt.h"
#include "feOpt.h"
#include "distrib.h"
#include "version.h"

#ifdef HAVE_FACTORY
#define SI_DONT_HAVE_GLOBAL_VARS
#include <factory.h>
#endif

/*0 implementation*/
int main(          /* main entry to Singular */
    int argc,      /* number of parameter */
    char** argv)   /* parameter array */
{
#ifdef INIT_BUG
  jjInitTab1();
#endif
#ifdef GENTABLE
  extern void ttGen1();
  extern void ttGen2();
  #ifndef __MWERKS__
    extern mpsr_ttGen(); // For initialization of (CMD, MP_COP) tables
    mpsr_ttGen();
  #endif
  ttGen1();
  ttGen2();
#else
  /* initialize components */
  siRandomStart=inits();
  feOptSpec[FE_OPT_RANDOM].value = (void*) siRandomStart;
  int optc, option_index;
  char* errormsg;

  // do this first, because -v might print version path
  feInitResources(argv[0]);

  // parse command line options
  while((optc = fe_getopt_long(argc, argv,
                               SHORT_OPTS_STRING, feOptSpec, &option_index))
//                               "", feOptSpec, &option_index))
        != EOF)
  {
    if (optc == '?' || optc == 0)
    {
      fprintf(stderr, "Use '%s --help' for a complete list of options\n", feArgv0);
      exit(1);
    }

    if (optc != LONG_OPTION_RETURN)
      option_index = feGetOptIndex(optc);

    assume(option_index >= 0 && option_index < (int) FE_OPT_UNDEF);

    if (fe_optarg == NULL &&
        (feOptSpec[option_index].type == feOptBool ||
         feOptSpec[option_index].has_arg == optional_argument))
      errormsg = feSetOptValue((feOptIndex) option_index, (int) 1);
    else
      errormsg = feSetOptValue((feOptIndex) option_index, fe_optarg);

    if (errormsg)
    {
      if (fe_optarg == NULL)
        fprintf(stderr, "Error: Option '--%s' %s\n",
               feOptSpec[option_index].name, errormsg);
      else
        fprintf(stderr, "Error: Option '--%s=%s' %s\n",
               feOptSpec[option_index].name, fe_optarg, errormsg);
      fprintf(stderr, "Use '%s --help' for a complete list of options\n", feArgv0);
      exit(1);
    }
    if (optc == 'h') exit(0);
  }

  /* say hello */
  {
    idhdl h;
    namespaceroot->push(NULL, "Top", 0, TRUE);
#ifdef HAVE_NAMESPACES
    h=enterid(mstrdup("Top"), 0, PACKAGE_CMD, &IDROOT, TRUE);
    IDPACKAGE(h) = NSPACK(namespaceroot);
    IDPACKAGE(h)->language = LANG_TOP;
#endif /* HAVE_NAMESPACES */
  }
  if (BVERBOSE(0))
  {
    printf(
"                     SINGULAR                             /"
#ifndef MAKE_DISTRIBUTION
"  Development"
#endif
"\n"
" A Computer Algebra System for Polynomial Computations   /   version %s\n"
"                                                       0<\n"
"     by: G.-M. Greuel, G. Pfister, H. Schoenemann        \\   %s\n"
"FB Mathematik der Universitaet, D-67653 Kaiserslautern    \\\n"
, S_VERSION1,S_VERSION2);
  }
  else
  {
    sdb_flags = 0;
#ifdef __MWERKS__
    memcpy(stderr,stdout,sizeof(FILE));
#else
    dup2(1,2);
#endif
  }
  slStandardInit();
  dlInit(feArgv0);
  myynest=0;
  if (! feOptValue(FE_OPT_NO_STDLIB))
  {
    int vv=verbose;
    verbose &= ~Sy_bit(V_LOAD_LIB);
#ifdef HAVE_NAMESPACES
    iiLibCmd(mstrdup("standard.lib"),TRUE, TRUE);
#else /* HAVE_NAMESPACES */
    iiLibCmd(mstrdup("standard.lib"), TRUE);
#endif /* HAVE_NAMESPACES */
    verbose=vv;
  }
  errorreported = 0;
  mmMarkInitDBMCB();
  setjmp(si_start_jmpbuf);

  // Now, put things on the stack of stuff to do

  // Last thing to do is to execute given scripts
  if (fe_optind < argc)
  {
    int i = argc - 1;
    while (i >= fe_optind)
    {
      newFile(argv[i]);
      i--;
    }
  }
  else
  {
    currentVoice=feInitStdin();
  }

  // before scripts, we execute -c, if it was given
  if (feOptValue(FE_OPT_EXECUTE) != NULL)
    newBuffer(mstrdup((char*) feOptValue(FE_OPT_EXECUTE)), BT_execute);

  // first thing, however, is to load .singularrc from Singularpath
  // and cwd/$HOME (in that order).
  if (! feOptValue(FE_OPT_NO_RC))
  {
    char buf[MAXPATHLEN];
    FILE * rc = feFopen("." DIR_SEPP ".singularrc", "r", buf);
    if (rc == NULL) rc = feFopen("~" DIR_SEPP ".singularrc", "r", buf);
    if (rc == NULL) rc = feFopen(".singularrc", "r", buf);

    if (rc != NULL)
    {
      if (BVERBOSE(V_LOAD_LIB))
        Print("// ** executing %s\n", buf);
      fclose(rc);
      newFile(buf);
    }
  }

  /* start shell */
  if (fe_fgets_stdin==fe_fgets_dummy)
  {
#ifdef HAVE_MPSR
    extern int Batch_do(const char* port, const char* host);
    return Batch_do((char*) feOptValue(FE_OPT_MPPORT),
                    (char*) feOptValue(FE_OPT_MPHOST));
#else
    assume(0);
#endif
  }
  setjmp(si_start_jmpbuf);
  yyparse();
#endif
  //Print("at very end\n");
  m2_end(0);
  return 0;
}

