/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/***************************************************************
 *  File:    p_ProcsGenerate.cc
 *  Purpose: generate p_Procs*.inc at compile
 *  Note:    this file is included by p_Procs.cc
 *  Author:  obachman (Olaf Bachmann)
 *  Created: 8/00
 *  Version: $Id: p_Procs_Generate.cc,v 1.4 2000-12-31 15:14:39 obachman Exp $
 *******************************************************************/


#include <stdio.h>
#include <stdlib.h>

#include "mod2.h"
#include "dError.c"

#ifdef p_Procs_Static
#include "p_Procs_Static.h"
#else
#include "p_Procs_Dynamic.h"
#endif

#include "p_Procs_Impl.h"

#ifndef p_Procs_Static
int FieldGeneralProcs = 0,
  FieldIndepProcs = 0,
  FieldZpProcs = 0,
  FieldQProcs = 0,
  KernelProcs = 0,
  UnknownProcs = 0;

// returns 1, if proc should go into kernel, 0 otherwise
int IsKernelProc(p_Proc proc, p_Field field, p_Length length, p_Ord ord)
{
  // general procs go into kernel
  if (field == FieldGeneral && length == LengthGeneral && ord == OrdGeneral)
    return 1;
  
  // plus procs with FieldZp
  if ((field == FieldZp || field == FieldQ) && 
      // which are not general in length or ord
      !((length == LengthGeneral && p_ProcDependsOn_Length(proc)) ||
        (ord == OrdGeneral && p_ProcDependsOn_Ord(proc))) &&
      // and whose length is smaller than five 
      (!p_ProcDependsOn_Length(proc) || (length >= LengthFour)))
    return 1;

  
  
  return 0;
}

#endif

#define DoSetProc(what, field, length, ord) \
      GenerateProc(#what, what##_Proc, field, length, ord)

char*** generated_p_procs;

inline int AlreadyHaveProc(p_Proc proc, p_Field field, p_Length length, p_Ord ord)
{
  return (generated_p_procs[proc])[index(proc, field, length, ord)] != 0;
}

const char* macros_field[] = {"n_Copy","n_Delete", "n_Mult", "n_Add", "n_Sub", "n_IsZero", "n_Equal" , "n_Neg", NULL};

const char* macros_length[] =
{"p_MemCopy", "p_MemAdd", "p_MemSum", "p_MemDiff", NULL};

const char* macros_length_ord[] = {"p_MemCmp", NULL};
int DummyProcs = 0;

int NumberOfHaveProcs = 0;

void AddProc(const char* s_what, p_Proc proc, p_Field field, p_Length length, p_Ord ord)
{
  NumberOfHaveProcs++;
  int i;
  const char* s_length = p_LengthEnum_2_String(length);
  const char* s_ord = p_OrdEnum_2_String(ord);
  const char* s_field = p_FieldEnum_2_String(field);
  char* s_full_proc_name = (char*) malloc(200);
  
  sprintf(s_full_proc_name, "%s__%s_%s_%s", s_what, s_field, s_length, s_ord);
             
  (generated_p_procs[proc])[index(proc, field, length, ord)] = s_full_proc_name;
  // define all macros 
  printf("\n// definition of %s\n", s_full_proc_name);
#ifndef p_Procs_Static
  if (IsKernelProc(proc, field, length, ord))
  {
    KernelProcs++;
    printf("#ifdef p_Procs_Kernel\n");
  }
  else
  {
    const char* module = p_ProcField_2_Module(proc, field);
    if (strcmp(module, "FieldGeneral") == 0)
      FieldGeneralProcs++;
    else if (strcmp(module, "FieldIndep") == 0)
      FieldIndepProcs++;
    else if (strcmp(module, "FieldZp") == 0)
      FieldZpProcs++;
    else if (strcmp(module, "FieldQ") == 0)
      FieldQProcs++;
    else
      UnknownProcs++;

    printf("#ifdef p_Procs_%s\n", module);
  }
#endif  
  i = 0;
  while (macros_field[i] != NULL)
  {
    printf("#undef %s\n#define %s\t%s_%s\n", 
           macros_field[i], macros_field[i],  macros_field[i], s_field);
    i++;
  }
  i = 0;
  while (macros_length[i] != NULL)
  {
    printf("#undef %s\n#define %s\t%s_%s\n", 
           macros_length[i], macros_length[i], macros_length[i], s_length);
    i++;
  }
  i = 0;
  while (macros_length_ord[i] != NULL)
  {
    printf("#undef %s\n#define %s\t%s_%s_%s\n", 
           macros_length_ord[i], macros_length_ord[i], macros_length_ord[i], s_length, s_ord);
    i++;
  }

  // define DECLARE_LENGTH
  printf("#undef DECLARE_LENGTH\n");
  printf("#undef p_MemAddAdjust\n");
  if (length != LengthGeneral)
  {
    printf("#define DECLARE_LENGTH(what) ((void)0)\n");
    printf("#define p_MemAddAdjust(p, r) ((void)0)\n");
  }
  else
  {
    printf("#define DECLARE_LENGTH(what) what\n");
    if (proc != pp_Mult_Coeff_mm_DivSelectMult_Proc)
      printf("#define p_MemAddAdjust(p, r) p_MemAdd_NegWeightAdjust(p, r)\n");
    else
      printf("#define p_MemAddAdjust(p, r) ((void)0)\n");
  }
  
  // define DECLARE_ORDSGN
  printf("#undef DECLARE_ORDSGN\n");
  if (ord != OrdGeneral)
    printf("#define DECLARE_ORDSGN(what) ((void)0)\n");
  else
    printf("#define DECLARE_ORDSGN(what) what\n");

  if (proc == pp_Mult_Coeff_mm_DivSelectMult_Proc)
  {
    printf("#undef DECLARE_LENGTH_2\n");
    printf("#undef p_MemCmp_Bitmask_2\n");
    if (length != LengthGeneral)
    {
      printf("#define DECLARE_LENGTH_2(what) ((void)0)\n");
      if (length < LengthTwo)
        printf("#define p_MemCmp_Bitmask_2 p_MemCmp_Bitmask_%s\n", p_LengthEnum_2_String((p_Length) ((int) length + 2)));
      else
      printf("#define p_MemCmp_Bitmask_2 p_MemCmp_Bitmask_LengthZero\n");
    }
    else
    {
      printf("#define DECLARE_LENGTH_2(what) what \n");
      printf("#define p_MemCmp_Bitmask_2 p_MemCmp_Bitmask_LengthGeneral\n");
    }
    
      
    printf("#undef p_MemAddAdjust\n");
    printf("#define p_MemAddAdjust(p, r) ((void)0)\n");
  }

  printf("#undef %s\n#define %s %s\n", s_what, s_what, s_full_proc_name);
  printf("#include \"%s__Template.cc\"\n", s_what);
  printf("#undef %s\n", s_what);
#ifndef p_Procs_Static
  printf("#endif // p_Procs_[Kernel|Field*]\n");
#endif
}

void GenerateProc(const char* s_what, p_Proc proc, p_Field field, p_Length length, p_Ord ord)
{
  if (! AlreadyHaveProc(proc, field, length, ord))
    AddProc(s_what, proc, field, length, ord);
}

int main()
{
  int field = FieldGeneral;
  int length  = LengthGeneral;
  int ord = OrdGeneral;
  int i;
  
  
  printf("/* -*-c++-*- */
/***************************************************************
 * This file was generated automatically by p_ProcsGenerate.cc: DO NOT EDIT
 *
 * This file provides the needed implementation of p_Procs for 
 *               %s
 * See the end for a summary. 
 *******************************************************************/\n",

#if defined(p_Procs_Static)
         "p_Procs_Static"
#else
         "p_Procs_Dynamic"
#endif
         );

  generated_p_procs = (char***) malloc(p_Unknown_Proc*sizeof(char**));
  for (i=0; i<p_Unknown_Proc; i++)
  {
    generated_p_procs[i] = 
      (char**) calloc(index((p_Proc)i, FieldUnknown, LengthUnknown, OrdUnknown), sizeof(char*));
  }

  // set default procs
  for (field = 0; field < (int) FieldUnknown; field++)
  {
    for (length=0; length < (int) LengthUnknown; length++)
    {
      for (ord=0; ord < (int)OrdUnknown; ord++)
      {
        if (IsValidSpec((p_Field) field, (p_Length) length, (p_Ord) ord))
            SetProcs((p_Field) field, (p_Length) length, (p_Ord) ord);
      }
    }
  }

// we only need lookup tables for p_Procs_Static
#ifdef p_Procs_Static
  int j;
  printf("
/***************************************************************
  Names of procs for RDEBUG */
#ifdef RDEBUG\n");

  for (i=0; i<p_Unknown_Proc; i++)
  {
    printf("static const char* %s_names[] = {", p_ProcEnum_2_String((p_Proc)i));
    for (j=0;j<index((p_Proc)i, FieldUnknown, LengthUnknown, OrdUnknown); j++)
    {
      char* s = (generated_p_procs[i])[j];
      if (s != 0)
      {
        printf("\n\"%s\",", s);
      }
      else
        printf("0,");
          
    }
    printf("\n};\n");
  }
  printf("
#endif // RDEBUG


/***************************************************************/
/* Tables for lookup of procedures: */\n");

  for (i=0; i<p_Unknown_Proc; i++)
  {
    printf("static const %s_Ptr %s_funcs[] = {", p_ProcEnum_2_String((p_Proc)i), p_ProcEnum_2_String((p_Proc)i));
    for (j=0;j<index((p_Proc)i, FieldUnknown, LengthUnknown, OrdUnknown); j++)
    {
      char* s = (generated_p_procs[i])[j];
      if (s != 0)
      {
        printf("\n%s,", s);
      }
      else
        printf("0,");
          
    }
    printf("\n};\n");
  }
#endif

  printf("
/***************************************************************
 * Summary: 
 *   HAVE_FAST_P_PROCS  = %d, 
 *   HAVE_FAST_FIELD    = %d, 
 *   HAVE_FAST_LENGTH   = %d, 
 *   HAVE_FAST_ORD      = %d, 
 *   HAVE_FAST_ZERO_ORD = %d
 *   
 *   Generated PolyProcs= %d\n",
         HAVE_FAST_P_PROCS, HAVE_FAST_FIELD, HAVE_FAST_LENGTH, HAVE_FAST_ORD, 
         HAVE_FAST_ZERO_ORD, NumberOfHaveProcs);

#ifndef p_Procs_Static
  printf(" * 
 * KernelProcs          = %d
 * FieldIndepProcs      = %d
 * FieldZpProcs         = %d
 * FieldQProcs          = %d
 * FieldGeneralProcs    = %d
 * FieldUnknownProcs    = %d\n",
         KernelProcs, FieldIndepProcs, FieldZpProcs,FieldQProcs,
         FieldGeneralProcs, UnknownProcs);
#endif  

  printf(" *
 *******************************************************************/\n");
}


