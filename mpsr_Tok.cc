/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/

/* $Id: mpsr_Tok.cc,v 1.23 1999-09-27 15:05:28 obachman Exp $ */

/***************************************************************
 *
 * File:       mpsr_Tok.cc
 * Purpose:    Routines which realize Singular CMD <-> MP (dict, cop) mappings
 *             (and ordering mappings)
 * Author:     Olaf Bachmann (1/97)
 *
 * Change History (most recent first):
 *
 ***************************************************************/

#include"mod2.h"

#ifdef HAVE_MPSR

#include"febase.h"
#include"tok.h"

#include "mpsr.h"
#include "mpsr_Tok.h"


#define MAX_COP 256 // there may be at most 256 cops

// this is the main data struct for storing the relation
// Singular token <-> (Dict, OP)
typedef struct mpsr_cmd
{

  short         tok;    // The Singular token encoding

  // The MP Dict tag in which this token is defined,
  MP_DictTag_t  dict;

  // The MP operator corresponding to this token
  MP_Common_t   cop; // operator
} mpsr_cmd;

#define MAX_SR_DICT     5

// this provides a mapping from MP dict tags to more useful (small)
// array indicies
inline short mpdict2srdict(MP_DictTag_t dict)
{
  if (dict == MP_SingularDict) return 0;
  else if (dict == MP_BasicDict) return 1;
  else if (dict == MP_PolyDict) return 2;
  else if (dict == MP_MatrixDict) return 3;
  else if (dict == MP_NumberDict) return 4;
  else return MAX_SR_DICT;
}

#ifdef GENTABLE
// This is the array which holds all mpsr_cmds
// It is initialized in mpsr_tok.inc
static mpsr_cmd mpsr_cmds[MAX_TOK];

// This is the array which stores the mapping from token to an mpsr_cmd
// A value is either an index into mpsr_cmds, or MAX_TOK
static short tok2mp[MAX_TOK];

// This is the array which stores the mapping from (dict, cop) to a
// mpsr_cmd. First index mpdict2srdict(dict), second is cop
static short mp2tok[MAX_SR_DICT][MAX_COP];

#else
// Here are the actual definition of these token tables
#include"mpsr_Tok.inc"
#endif


// And here are the main routines which provide the mappings
mpsr_Status_t mpsr_tok2mp(short tok, MP_DictTag_t *dict, MP_Common_t *cop)
{
  short tok_index = tok2mp[tok];

  if (tok_index != MAX_TOK)
  {
    *dict = mpsr_cmds[tok_index].dict;
    *cop = mpsr_cmds[tok_index].cop;
    return mpsr_Success;
  }
  else
    return mpsr_SetError(mpsr_UnknownSingularToken);
}

mpsr_Status_t mpsr_mp2tok(MP_DictTag_t dict, MP_Common_t cop, short *o_tok)
{
  short sr_dict = mpdict2srdict(dict);
  short tok;

  if (sr_dict == MAX_SR_DICT)
    return mpsr_SetError(mpsr_UnknownDictionary);

  tok = mp2tok[sr_dict][cop];
  if (tok == MAX_TOK)
    return mpsr_SetError(mpsr_UnkownOperator);

  *o_tok = tok;
  return mpsr_Success;
}


#define MAX_ORD ringorder_unspec

static struct
{
  int sing_ord;
  int mp_ord;
} sing_mp_ord[] =
{
  {ringorder_no,    MP_CcPolyOrdering_Unknown},
  {ringorder_a,     MP_CcPolyOrdering_Vector},
  {ringorder_c,     MP_CcPolyOrdering_IncComp},
  {ringorder_C,     MP_CcPolyOrdering_DecComp},
  {ringorder_M,     MP_CcPolyOrdering_Matrix},
  {ringorder_lp,    MP_CcPolyOrdering_Lex},
  {ringorder_dp,    MP_CcPolyOrdering_DegRevLex},
  {ringorder_Dp,    MP_CcPolyOrdering_DegLex},
  {ringorder_wp,    MP_CcPolyOrdering_RevLex},
  {ringorder_Wp,    MP_CcPolyOrdering_Lex},
  {ringorder_ls,    MP_CcPolyOrdering_NegLex},
  {ringorder_ds,    MP_CcPolyOrdering_NegDegRevLex},
  {ringorder_Ds,    MP_CcPolyOrdering_NegDegLex},
  {ringorder_ws,    MP_CcPolyOrdering_NegRevLex},
  {ringorder_Ws,    MP_CcPolyOrdering_NegLex},
  {ringorder_unspec, MP_CcPolyOrdering_Unknown}
};

MP_Common_t mpsr_ord2mp(int sr_ord)
{
  int or = ringorder_no;

  while (sing_mp_ord[or].sing_ord != sr_ord &&
         sing_mp_ord[or].sing_ord <= ringorder_unspec) or++;

  return sing_mp_ord[or].mp_ord;
}

short mpsr_mp2ord(MP_Common_t mp_ord)
{
  int or = ringorder_no;

  while (sing_mp_ord[or].mp_ord != mp_ord &&
         sing_mp_ord[or].sing_ord <= ringorder_unspec) or++;
  return sing_mp_ord[or].sing_ord;
}


#ifdef GENTABLE

// This returns 1 if tok is a token which can appear in a Singular
// (quoted) command, and 0 otherwise
#include"ipshell.h" // has declarations of dArith


  // some special cmds which do not fit in with the others, and
  // nevertheless need to be transmitted
short ExtraCmds[] =
{
  OPTION_CMD,
  NAMES_CMD,
//  RESERVEDNAME_CMD,
  PROC_CMD,
  MAP_CMD,
  PACKAGE_CMD,
  '=',
  0
};


// This the list of all tokens which have an MP representation as a
// cop in the Singular dictionary
short sr_cmds[] =
{
  OPTION_CMD,
  NAMES_CMD,
  ATTRIB_CMD,
  CHARSTR_CMD,
  CLOSE_CMD,
  DEF_CMD,
  DEGREE_CMD,
  DEFINED_CMD,
  E_CMD,
  FREEMODULE_CMD,
  INT_CMD,
  INTERRED_CMD,
  INTMAT_CMD,
  INTVEC_CMD,
  IS_RINGVAR,
  KILLATTR_CMD,
  MAP_CMD,
  MEMORY_CMD,
  MONITOR_CMD,
  NAMEOF_CMD,
  NUMBER_CMD,
  NPARS_CMD,
  NVARS_CMD,
  OPEN_CMD,
  ORDSTR_CMD,
  PAR_CMD,
  PARSTR_CMD,
  PARDEG_CMD,
  POLY_CMD,
  PRINT_CMD,
  READ_CMD,
  SORTVEC_CMD,
  STRING_CMD,
  SYSTEM_CMD,
  TYPEOF_CMD,
  VECTOR_CMD,
  VAR_CMD,
  VARSTR_CMD,
  WEIGHT_CMD,
  '(',
  COEF_CMD,
  DELETE_CMD,
  FETCH_CMD,
  FIND_CMD,
  IMAP_CMD,
  INSERT_CMD,
  SIMPLIFY_CMD,
  SRES_CMD,
  DBPRINT_CMD,
  TEST_CMD,
  PROC_CMD,
  MSTD_CMD,
  RESERVEDNAME_CMD,
  WRITE_CMD,
  QRING_CMD,
  FGLM_CMD,
  DUMP_CMD,
  GETDUMP_CMD,
  STATUS_CMD,
  LIB_CMD,
  PACKAGE_CMD
};

// struct used for specifying the cmd <-> cop relations
typedef struct cmd_cop
{
  short cmd;
  MP_Common_t cop;
} cmd_op;

typedef struct cmd_dictcop
{
  MP_DictTag_t  dict;
  cmd_op        cmd_ops[255];
} cmd_dictcop;

cmd_dictcop cmd_dictcops[] =
{
  {
    MP_PolyDict,
    // This is the list of all tokens which have an MP representation as a
    // cop in the Poly dictionary
    {
      {BETTI_CMD, MP_CopPolyBetti},
      {CHARACTERISTIC_CMD, MP_CopPolyChar},
      {CHAR_SERIES_CMD, MP_CopPolyCharSeries},
      {CONTENT_CMD, MP_CopPolyClearDenom },
      {DEG_CMD, MP_CopPolyDeg},
      {DIM_CMD, MP_CopPolyDim},
      {FAC_CMD, MP_CopPolyFactorize},
      {FACSTD_CMD, MP_CopPolyFacStd},
      {HILBERT_CMD, MP_CopPolyHilb},
      {HOMOG_CMD, MP_CopPolyHomog},
      {INDEPSET_CMD, MP_CopPolyInDepSet},
      {IDEAL_CMD, MP_CopPolyIdeal},
      {KBASE_CMD, MP_CopPolyKbase},
      {LEAD_CMD, MP_CopPolyLead},
      {LEADCOEF_CMD, MP_CopPolyLeadCoef},
      {LEADEXP_CMD, MP_CopPolyLeadExp},
      {MAXID_CMD, MP_CopPolyMaxIdeal},
      {MINBASE_CMD, MP_CopPolyMinBase},
      {MINRES_CMD, MP_CopPolyMinRes},
      {MODUL_CMD, MP_CopPolyModule},
      {MULTIPLICITY_CMD, MP_CopPolyMultiplicity},
      {ORD_CMD, MP_CopPolyOrder},
      {PRUNE_CMD, MP_CopPolyPrune},
      {QHWEIGHT_CMD, MP_CopPolyQHWeight},
      {REGULARITY_CMD, MP_CopPolyRegularity},
      {RESULTANT_CMD, MP_CopPolyResultant},
      {STD_CMD, MP_CopPolyStd},
      {SYZYGY_CMD, MP_CopPolySyz},
      {VDIM_CMD, MP_CopPolyVdim},
      {COEFFS_CMD,  MP_CopPolyCoeffs},
      {CONTRACT_CMD, MP_CopPolyContract},
      {ELIMINATION_CMD, MP_CopPolyEliminate},
      {JET_CMD, MP_CopPolyJet},
      {LIFT_CMD, MP_CopPolyLift},
      {LIFTSTD_CMD, MP_CopPolyLiftstd},
      {MODULO_CMD, MP_CopPolyModulo},
      {MRES_CMD, MP_CopPolyMres},
      {QUOTIENT_CMD, MP_CopPolyQuotient},
      {REDUCE_CMD, MP_CopPolyReduce},
      {PREIMAGE_CMD, MP_CopPolyPreimage},
      {RES_CMD, MP_CopPolyRes},
      {RING_CMD, MP_CopPolyRing},
      {MAX_TOK, 0}
    }
  },
  {
    MP_NumberDict,
    // This is the list of all tokens which have an MP representation as a
    // cop in the Number dictionary
    {
      {PRIME_CMD, MP_CopNumberPrime},
      {EXTGCD_CMD, MP_CopNumberExtGcd},
      {GCD_CMD, MP_CopNumberGcd},
      {RANDOM_CMD, MP_CopNumberRandom},
      {MAX_TOK, 0}
    }
  },
  {
    MP_MatrixDict,
    // This is the list of all tokens which have an MP representation as a
    // cop in the Matrix dictionary
    {
      {BAREISS_CMD, MP_CopMatrixBareiss},
      {COLS_CMD, MP_CopMatrixCols},
      {DET_CMD, MP_CopMatrixDet},
      {JACOB_CMD, MP_CopMatrixJacobi},
      {MATRIX_CMD, MP_CopMatrixDenseMatrix},
      {ROWS_CMD, MP_CopMatrixRows},
      {TRACE_CMD, MP_CopMatrixTrace},
      {TRANSPOSE_CMD, MP_CopMatrixTranspose},
      {KOSZUL_CMD, MP_CopMatrixKoszul},
      {MINOR_CMD, MP_CopMatrixMinor},
      {WEDGE_CMD, MP_CopMatrixWedge},
      {MAX_TOK, 0}
    }
  },
  {
    MP_BasicDict,
    // This is the list of all tokens which have an MP representation as a
    // cop in the MP Basic dictionary
    {
      {PLUSPLUS, MP_CopBasicInc},
      {MINUSMINUS,  MP_CopBasicDec},
      {COUNT_CMD, MP_CopBasicSize},
      {LIST_CMD, MP_CopBasicList},
      {'+', MP_CopBasicAdd},
      {'-', MP_CopBasicMinus},
      {'*', MP_CopBasicMult},
      {'/', MP_CopBasicDiv},
      {'%', MP_CopBasicMod},
      {'^', MP_CopBasicPow},
      {GE, MP_CopBasicGreaterEqual},
      {'<', MP_CopBasicGreater},
      {LE, MP_CopBasicLessEqual},
      {'>', MP_CopBasicLess},
      {'&', MP_CopBasicAnd},
      {'|', MP_CopBasicOr},
      {'=', MP_CopBasicAssign},
      {EQUAL_EQUAL, MP_CopBasicEqual},
      {NOTEQUAL, MP_CopBasicNotEqual},
      {DOTDOT, MP_CopBasicRange},
      {'[', MP_CopBasicIndex},
      {DIFF_CMD, MP_CopBasicDiff},
      {INTERSECT_CMD, MP_CopBasicInterSect},
      {SUBST_CMD, MP_CopBasicSubst},
      {NOT, MP_CopBasicNot},
      {COLONCOLON, MP_CopBasicPackage},
      {MAX_TOK, 0}
    }
  }
};


static short IsCmdToken(short tok)
{
  short i = 0;
  // cmds with one arg
  while (dArith1[i].cmd != 0)
    if (dArith1[i].cmd == tok) return 1;
    else i++;

  // cmds with two args
  i=0;
  while (dArith2[i].cmd != 0)
    if (dArith2[i].cmd == tok) return 1;
    else i++;

  // cmds with three args
  i=0;
  while (dArith3[i].cmd != 0)
    if (dArith3[i].cmd == tok) return 1;
    else i++;

  // cmds with many args
  i=0;
  while (dArithM[i].cmd != 0)
    if (dArithM[i].cmd == tok) return 1;
    else i++;

  // cmds which are somewhat special (like those having 0 args)
  i=0;
  while (ExtraCmds[i] != 0)
    if (ExtraCmds[i] == tok) return 1;
    else i++;

  return 0;
}

// Given a Singular token, find matching (dict,op): Return 1 if one is
// found, 0, otherwise
static short GetMPDictTok(short tok, MP_DictTag_t *dict, MP_Common_t *cop)
{
  short i, l, j;
  cmd_op *cmd_ops;

  // first, look through Singular specific commands
  l = sizeof(sr_cmds)/sizeof(short);
  if (l > MAX_COP)
  {
    fprintf(stderr,
            "Error: There are more than 256 entries in MP_SingularDict\n");
    exit(1);
  }
  for (i=0; i<l; i++)
    if (sr_cmds[i] == tok)
    {
      *dict = MP_SingularDict;
      *cop = i;
      return 1;
    }

  // look through all the other dicts
  for (j=0; j<MAX_SR_DICT-1; j++)
  {
    cmd_ops = cmd_dictcops[j].cmd_ops;
    for (i=0; (cmd_ops[i]).cmd != MAX_TOK; i++)
    {
      if (i > MAX_COP)
      {
        fprintf(stderr,
                "Error: There are more than 256 entries in dict %d's\n",j);
        exit(1);
      }
      if (cmd_ops[i].cmd == tok)
      {
        *dict = cmd_dictcops[j].dict;
        *cop = cmd_ops[i].cop;
        return 1;
      }
    }
  }
  return 0;
}


// This actually generates the tables of mpsr_tok.inc
void mpsr_ttGen()
{
  mpsr_cmd mpsrcmds[MAX_TOK];
  short tok2mp[MAX_TOK];
  short mp2tok[MAX_SR_DICT][MAX_COP];
  short max_cmd = 0, i, j;
  MP_Common_t cop;
  FILE *outfile;
  MP_DictTag_t dict;


  // init all arrays
  for (i=0; i<MAX_TOK; i++)
  {
    mpsrcmds[i].tok = MAX_TOK;
    tok2mp[i] = MAX_TOK;
  }
  for (i=0; i<MAX_SR_DICT; i++)
    for (j=0; j<MAX_COP; j++)
      mp2tok[i][j] = MAX_TOK;

  // Now go through all the token and test them
  for (i=0; i<MAX_TOK; i++)
  {
    if (IsCmdToken(i))
    {
      if (GetMPDictTok(i, &dict, &cop))
      {
        mpsrcmds[max_cmd].tok = i;
        mpsrcmds[max_cmd].dict = dict;
        mpsrcmds[max_cmd].cop = cop;
        tok2mp[i] = max_cmd;
        mp2tok[mpdict2srdict(dict)][cop] = i;
        max_cmd++;
      }
      else
      {
        fprintf(stderr, "Warning: mpsr_ttGen: Unknown Cmd Token: %d(%s)\n",
                        i, iiTwoOps(i));
      }
    }
  }

  // Generate the template file
  outfile = myfopen("mpsr_Tok.inc", "w");
  if (outfile == NULL)
  {
    fprintf(stderr, "Error: mpsr_ttGen: Cannot open file mpsr_Tok.inc\n");
    exit(1);
  }

  // header
  fprintf(outfile,"/***************************************************************
 *
 * File:       mpsr_tok.inc
 * Purpose:    tables for mapping Singular cmds to/from MP (dict, op)
 *
 * THIS FILE WAS AUTOMATICALLY GENERATED BY mpsr_ttGen(). DO NOT EDIT!
 *
 ***************************************************************/
#ifndef MPSR_STRING_TABLES
mpsr_cmd mpsr_cmds[] =
{
  { %d,\t %d,\t %d }", mpsrcmds[0].tok, mpsrcmds[0].dict, mpsrcmds[0].cop);

  // mpsrcmds
  for (i=1; i<max_cmd; i++)
  {
    fprintf(outfile, ",\n  { %d,\t %d,\t %d }",
            mpsrcmds[i].tok, mpsrcmds[i].dict, mpsrcmds[i].cop);
  }
  fprintf(outfile,"\n};\n\n");

  // tok2mp
  fprintf(outfile, "short tok2mp[] = { %d", tok2mp[0]);
  for (i=1; i<MAX_TOK; i++)
    fprintf(outfile, ", %d", tok2mp[i]);
  fprintf(outfile, "};\n\n");

  // mp2tok
  fprintf(outfile, "short mp2tok[MAX_SR_DICT][MAX_COP] = \n{");
  for (i=0; i<MAX_SR_DICT; i++)
  {
    fprintf(outfile, "\n{\n");
    for (j=0; j<MAX_COP; j++)
    {
      fprintf(outfile, " %d",mp2tok[i][j]);
      if  (j!=MAX_COP-1) fprintf(outfile, ",");
    }
    if (i!=MAX_SR_DICT-1) fprintf(outfile, "},");
    else                  fprintf(outfile, "}");
  }
  fprintf(outfile,"\n};\n\n");

  fprintf(outfile, "
#else /* MPSR_STRING_TABLE */
mpsr_cmd mpsr_cmds[] =
{
  { \"%s\",\t %d,\t %d }", iiTwoOps(mpsrcmds[0].tok), mpsrcmds[0].dict, mpsrcmds[0].cop);

  for (i=1; i<max_cmd; i++)
  {
    fprintf(outfile, ",\n  { \"%s\",\t %d,\t %d }",
            iiTwoOps(mpsrcmds[i].tok), mpsrcmds[i].dict, mpsrcmds[i].cop);
  }
  fprintf(outfile, ",\n { NULL, \t 0, \t 0}");
  fprintf(outfile,"\n};\n\n#endif /* ! MPSR_STRING_TABLE */");

  fclose(outfile);
} // That's all

#endif // GENTABLE

#else // NOT HAVE_MPSR

#ifdef GENTABLE

// simply touch mpsr_Tok.inc so that Make does not get confused
#ifndef macintosh
extern "C" int system(char *);
#else
#include <stdio.h>
#endif

void mpsr_ttGen()
{
#ifndef macintosh
  system("touch mpsr_Tok.inc");
#else
  // simulate touch on a macintosh
  FILE *fd = fopen("mpsr_Tok.inc", "w");
  close(fd);
#endif
}
#endif

#endif // HAVE_MPSR
