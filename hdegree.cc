/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/* $Id: hdegree.cc,v 1.10 1997-12-15 22:46:23 obachman Exp $ */
/*
*  ABSTRACT -  dimension, multiplicity, HC, kbase
*/

#include "mod2.h"
#include "tok.h"
#include "lists.h"
#include "febase.h"
#include "mmemory.h"
#include "ipid.h"
#include "ideals.h"
#include "polys.h"
#include "intvec.h"
#include "numbers.h"
#include "hutil.h"
#include "stairc.h"

static int  hCo, hMu, hMu2;

/*0 implementation*/

// dimension

static void hDimSolve(scmon pure, int Npure, scfmon rad, int Nrad,
 varset var, int Nvar)
{
  int  dn, iv, rad0, b, c, x;
  scmon pn;
  scfmon rn;
  if (Nrad < 2)
  {
    dn = Npure + Nrad;
    if (dn < hCo)
      hCo = dn;
    return;
  }
  if (Npure+1 >= hCo)
    return;
  iv = Nvar;
  while(pure[var[iv]]) iv--;
  hStepR(rad, Nrad, var, iv, &rad0);
  if (rad0!=0)
  {
    iv--;
    if (rad0 < Nrad)
    {
      pn = hGetpure(pure);
      rn = hGetmem(Nrad, rad, radmem[iv]);
      hDimSolve(pn, Npure + 1, rn, rad0, var, iv);
      b = rad0;
      c = Nrad;
      hElimR(rn, &rad0, b, c, var, iv);
      hPure(rn, b, &c, var, iv, pn, &x);
      hLex2R(rn, rad0, b, c, var, iv, hwork);
      rad0 += (c - b);
      hDimSolve(pn, Npure + x, rn, rad0, var, iv);
    }
    else
    {
      hDimSolve(pure, Npure, rad, Nrad, var, iv);
    }
  }
  else
    hCo = Npure + 1;
}

int  scDimInt(ideal S, ideal Q)
{
  Exponent_t  mc;
  hexist = hInit(S, Q, &hNexist);
  if (!hNexist)
    return pVariables;
  hwork = (scfmon)Alloc(hNexist * sizeof(scmon));
  hvar = (varset)Alloc((pVariables + 1) * sizeof(int));
  hpure = (scmon)Alloc((1 + (pVariables * pVariables)) * sizeof(Exponent_t));
  mc = hisModule;
  if (!mc)
  {
    hrad = hexist;
    hNrad = hNexist;
  }
  else
    hrad = (scfmon)Alloc(hNexist * sizeof(scmon));
  radmem = hCreate(pVariables - 1);
  hCo = pVariables + 1;
  loop
  {
    if (mc)
      hComp(hexist, hNexist, mc, hrad, &hNrad);
    if (hNrad)
    {
      hNvar = pVariables;
      hRadical(hrad, &hNrad, hNvar);
      hSupp(hrad, hNrad, hvar, &hNvar);
      if (hNvar)
      {
        memset(hpure, 0, (pVariables + 1) * sizeof(Exponent_t));
        hPure(hrad, 0, &hNrad, hvar, hNvar, hpure, &hNpure);
        hLexR(hrad, hNrad, hvar, hNvar);
        hDimSolve(hpure, hNpure, hrad, hNrad, hvar, hNvar);
      }
    }
    else
    {
      hCo = 0;
      break;
    }
    mc--;
    if (mc <= 0)
      break;
  }
  hKill(radmem, pVariables - 1);
  Free((ADDRESS)hpure, (1 + (pVariables * pVariables)) * sizeof(Exponent_t));
  Free((ADDRESS)hvar, (pVariables + 1) * sizeof(int));
  Free((ADDRESS)hwork, hNexist * sizeof(scmon));
  hDelete(hexist, hNexist);
  if (hisModule)
    Free((ADDRESS)hrad, hNexist * sizeof(scmon));
  return pVariables - hCo;
}

// independent set
static scmon hInd;

static void hIndSolve(scmon pure, int Npure, scfmon rad, int Nrad,
 varset var, int Nvar)
{
  int  dn, iv, rad0, b, c, x;
  scmon pn;
  scfmon rn;
  if (Nrad < 2)
  {
    dn = Npure + Nrad;
    if (dn < hCo)
    {
      hCo = dn;
      for (iv=pVariables; iv; iv--)
      {
        if (pure[iv])
          hInd[iv] = 0;
        else
          hInd[iv] = 1;
      }
      if (Nrad)
      {
        pn = *rad;
        iv = Nvar;
        loop
        {
          x = var[iv];
          if (pn[x])
          {
            hInd[x] = 0;
            break;
          }
          iv--;
        }
      }
    }
    return;
  }
  if (Npure+1 >= hCo)
    return;
  iv = Nvar;
  while(pure[var[iv]]) iv--;
  hStepR(rad, Nrad, var, iv, &rad0);
  if (rad0)
  {
    iv--;
    if (rad0 < Nrad)
    {
      pn = hGetpure(pure);
      rn = hGetmem(Nrad, rad, radmem[iv]);
      pn[var[iv + 1]] = 1;
      hIndSolve(pn, Npure + 1, rn, rad0, var, iv);
      pn[var[iv + 1]] = 0;
      b = rad0;
      c = Nrad;
      hElimR(rn, &rad0, b, c, var, iv);
      hPure(rn, b, &c, var, iv, pn, &x);
      hLex2R(rn, rad0, b, c, var, iv, hwork);
      rad0 += (c - b);
      hIndSolve(pn, Npure + x, rn, rad0, var, iv);
    }
    else
    {
      hIndSolve(pure, Npure, rad, Nrad, var, iv);
    }
  }
  else
  {
    hCo = Npure + 1;
    for (x=pVariables; x; x--)
    {
      if (pure[x])
        hInd[x] = 0;
      else
        hInd[x] = 1;
    }
    hInd[var[iv]] = 0;
  }
}

intvec * scIndIntvec(ideal S, ideal Q)
{
  intvec *Set=new intvec(pVariables);
  Exponent_t  mc,i;
  hexist = hInit(S, Q, &hNexist);
  if (hNexist==0)
  {
    for(i=0; i<pVariables; i++)
      (*Set)[i]=1;
    return Set;
  }
  hwork = (scfmon)Alloc(hNexist * sizeof(scmon));
  hvar = (varset)Alloc((pVariables + 1) * sizeof(int));
  hpure = (scmon)Alloc((1 + (pVariables * pVariables)) * sizeof(Exponent_t));
  hInd = (scmon)Alloc((1 + pVariables) * sizeof(Exponent_t));
  mc = hisModule;
  if (mc==0)
  {
    hrad = hexist;
    hNrad = hNexist;
  }
  else
    hrad = (scfmon)Alloc(hNexist * sizeof(scmon));
  radmem = hCreate(pVariables - 1);
  hCo = pVariables + 1;
  loop
  {
    if (mc!=0)
      hComp(hexist, hNexist, mc, hrad, &hNrad);
    if (hNrad!=0)
    {
      hNvar = pVariables;
      hRadical(hrad, &hNrad, hNvar);
      hSupp(hrad, hNrad, hvar, &hNvar);
      if (hNvar!=0)
      {
        memset(hpure, 0, (pVariables + 1) * sizeof(Exponent_t));
        hPure(hrad, 0, &hNrad, hvar, hNvar, hpure, &hNpure);
        hLexR(hrad, hNrad, hvar, hNvar);
        hIndSolve(hpure, hNpure, hrad, hNrad, hvar, hNvar);
      }
    }
    else
    {
      hCo = 0;
      break;
    }
    mc--;
    if (mc <= 0)
      break;
  }
  for(i=0; i<pVariables; i++)
    (*Set)[i] = hInd[i+1];
  hKill(radmem, pVariables - 1);
  Free((ADDRESS)hpure, (1 + (pVariables * pVariables)) * sizeof(Exponent_t));
  Free((ADDRESS)hInd, (1 + pVariables) * sizeof(Exponent_t));
  Free((ADDRESS)hvar, (pVariables + 1) * sizeof(int));
  Free((ADDRESS)hwork, hNexist * sizeof(scmon));
  hDelete(hexist, hNexist);
  if (hisModule)
    Free((ADDRESS)hrad, hNexist * sizeof(scmon));
  return Set;
}

typedef struct sindlist indlist;
typedef indlist * indset;

struct sindlist
{
  indset nx;
  intvec * set;
};
indset ISet, JSet;

static BOOLEAN hNotZero(scfmon rad, int Nrad, varset var, int Nvar)
{
  int  k1, i;
  k1 = var[Nvar];
  i = 0;
  loop
  {
    if (rad[i][k1]==0)
      return FALSE;
    i++;
    if (i == Nrad)
      return TRUE;
  }
}

static void hIndep(scmon pure)
{
  int iv;
  intvec *Set;

  Set = ISet->set = new intvec(pVariables);
  for (iv=pVariables; iv!=0 ; iv--)
  {
    if (pure[iv])
      (*Set)[iv-1] = 0;
    else
      (*Set)[iv-1] = 1;
  }
  ISet = ISet->nx = (indset)Alloc0(sizeof(indlist));
  hMu++;
}

static void hIndMult(scmon pure, int Npure, scfmon rad, int Nrad,
 varset var, int Nvar)
{
  int  dn, iv, rad0, b, c, x;
  scmon pn;
  scfmon rn;
  if (Nrad < 2)
  {
    dn = Npure + Nrad;
    if (dn == hCo)
    {
      if (Nrad==0)
        hIndep(pure);
      else
      {
        pn = *rad;
        for (iv = Nvar; iv!=0; iv--)
        {
          x = var[iv];
          if (pn[x])
          {
            pure[x] = 1;
            hIndep(pure);
            pure[x] = 0;
          }
        }
      }
    }
    return;
  }
  iv = Nvar;
  dn = Npure+1;
  if (dn >= hCo)
  {
    if (dn > hCo)
      return;
    loop
    {
      if(!pure[var[iv]])
      {
        if(hNotZero(rad, Nrad, var, iv))
        {
          pure[var[iv]] = 1;
          hIndep(pure);
          pure[var[iv]] = 0;
        }
      }
      iv--;
      if (!iv)
        return;
    }
  }
  while(pure[var[iv]]) iv--;
  hStepR(rad, Nrad, var, iv, &rad0);
  iv--;
  if (rad0 < Nrad)
  {
    pn = hGetpure(pure);
    rn = hGetmem(Nrad, rad, radmem[iv]);
    pn[var[iv + 1]] = 1;
    hIndMult(pn, Npure + 1, rn, rad0, var, iv);
    pn[var[iv + 1]] = 0;
    b = rad0;
    c = Nrad;
    hElimR(rn, &rad0, b, c, var, iv);
    hPure(rn, b, &c, var, iv, pn, &x);
    hLex2R(rn, rad0, b, c, var, iv, hwork);
    rad0 += (c - b);
    hIndMult(pn, Npure + x, rn, rad0, var, iv);
  }
  else
  {
    hIndMult(pure, Npure, rad, Nrad, var, iv);
  }
}

/*3
* consider indset x := !pure
* (for all i) (if(sm(i) > x) return FALSE)
* else return TRUE
*/
static BOOLEAN hCheck1(indset sm, scmon pure)
{
  int iv;
  intvec *Set;
  while (sm->nx != NULL)
  {
    Set = sm->set;
    iv=pVariables;
    loop
    {
      if (((*Set)[iv-1] == 0) && (pure[iv] == 0))
        break;
      iv--;
      if (iv == 0)
        return FALSE;
    }
    sm = sm->nx;
  }
  return TRUE;
}

/*3
* consider indset x := !pure
* (for all i) if(x > sm(i)) delete sm(i))
* return (place for x)
*/
static indset hCheck2(indset sm, scmon pure)
{
  int iv;
  intvec *Set;
  indset be, a1 = NULL;
  while (sm->nx != NULL)
  {
    Set = sm->set;
    iv=pVariables;
    loop
    {
      if ((pure[iv] == 1) && ((*Set)[iv-1] == 1))
        break;
      iv--;
      if (iv == 0)
      {
        if (a1 == NULL)
        {
          a1 = sm;
        }
        else
        {
          hMu2--;
          be->nx = sm->nx;
          delete Set;
          Free((ADDRESS)sm,sizeof(indlist));
          sm = be;
        }
        break;
      }
    }
    be = sm;
    sm = sm->nx;
  }
  if (a1 != NULL)
  {
    return a1;
  }
  else
  {
    hMu2++;
    sm->set = new intvec(pVariables);
    sm->nx = (indset)Alloc0(sizeof(indlist));
    return sm;
  }
}

/*2
*  definition x >= y
*      x(i) == 0 => y(i) == 0
*      > ex. j with x(j) == 1 and y(j) == 0
*/
static void hCheckIndep(scmon pure)
{
  intvec *Set;
  indset res;
  int iv;
  if (hCheck1(ISet, pure))
  {
    if (hCheck1(JSet, pure))
    {
      res = hCheck2(JSet,pure);
      if (res == NULL)
        return;
      Set = res->set;
      for (iv=pVariables; iv; iv--)
      {
        if (pure[iv])
          (*Set)[iv-1] = 0;
        else
          (*Set)[iv-1] = 1;
      }
    }
  }
}

static void hIndAllMult(scmon pure, int Npure, scfmon rad, int Nrad,
 varset var, int Nvar)
{
  int  dn, iv, rad0, b, c, x;
  scmon pn;
  scfmon rn;
  if (Nrad < 2)
  {
    dn = Npure + Nrad;
    if (dn > hCo)
    {
      if (!Nrad)
        hCheckIndep(pure);
      else
      {
        pn = *rad;
        for (iv = Nvar; iv; iv--)
        {
          x = var[iv];
          if (pn[x])
          {
            pure[x] = 1;
            hCheckIndep(pure);
            pure[x] = 0;
          }
        }
      }
    }
    return;
  }
  iv = Nvar;
  while(pure[var[iv]]) iv--;
  hStepR(rad, Nrad, var, iv, &rad0);
  iv--;
  if (rad0 < Nrad)
  {
    pn = hGetpure(pure);
    rn = hGetmem(Nrad, rad, radmem[iv]);
    pn[var[iv + 1]] = 1;
    hIndAllMult(pn, Npure + 1, rn, rad0, var, iv);
    pn[var[iv + 1]] = 0;
    b = rad0;
    c = Nrad;
    hElimR(rn, &rad0, b, c, var, iv);
    hPure(rn, b, &c, var, iv, pn, &x);
    hLex2R(rn, rad0, b, c, var, iv, hwork);
    rad0 += (c - b);
    hIndAllMult(pn, Npure + x, rn, rad0, var, iv);
  }
  else
  {
    hIndAllMult(pure, Npure, rad, Nrad, var, iv);
  }
}

lists scIndIndset(ideal S, BOOLEAN all, ideal Q)
{
  int i;
  indset save;
  lists res=(lists)Alloc0(sizeof(slists));

  hexist = hInit(S, Q, &hNexist);
  if ((hNexist == 0) || (hisModule!=0))
  {
    res->Init(0);
    return res;
  }
  save = ISet = (indset)Alloc0(sizeof(indlist));
  hMu = 0;
  hwork = (scfmon)Alloc(hNexist * sizeof(scmon));
  hvar = (varset)Alloc((pVariables + 1) * sizeof(int));
  hpure = (scmon)Alloc((1 + (pVariables * pVariables)) * sizeof(Exponent_t));
  hrad = hexist;
  hNrad = hNexist;
  radmem = hCreate(pVariables - 1);
  hCo = pVariables + 1;
  hNvar = pVariables;
  hRadical(hrad, &hNrad, hNvar);
  hSupp(hrad, hNrad, hvar, &hNvar);
  if (hNvar)
  {
    hCo = hNvar;
    memset(hpure, 0, (pVariables + 1) * sizeof(Exponent_t));
    hPure(hrad, 0, &hNrad, hvar, hNvar, hpure, &hNpure);
    hLexR(hrad, hNrad, hvar, hNvar);
    hDimSolve(hpure, hNpure, hrad, hNrad, hvar, hNvar);
  }
  if (hCo && (hCo < pVariables))
  {
    hIndMult(hpure, hNpure, hrad, hNrad, hvar, hNvar);
  }
  if (hMu!=0)
  {
    ISet = save;
    hMu2 = 0;
    if (all && (hCo+1 < pVariables))
    {
      JSet = (indset)Alloc0(sizeof(indlist));
      hIndAllMult(hpure, hNpure, hrad, hNrad, hvar, hNvar);
      i=hMu+hMu2;
      res->Init(i);
      if (hMu2 == 0)
      {
        Free((ADDRESS)JSet,sizeof(indlist));
      }
    }
    else
    {
      res->Init(hMu);
    }
    for (i=0;i<hMu;i++)
    {
      res->m[i].data = (void *)save->set;
      res->m[i].rtyp = INTVEC_CMD;
      ISet = save;
      save = save->nx;
      Free((ADDRESS)ISet,sizeof(indlist));
    }
    Free((ADDRESS)save,sizeof(indlist));
    if (hMu2 != 0)
    {
      save = JSet;
      for (i=hMu;i<hMu+hMu2;i++)
      {
        res->m[i].data = (void *)save->set;
        res->m[i].rtyp = INTVEC_CMD;
        JSet = save;
        save = save->nx;
        Free((ADDRESS)JSet,sizeof(indlist));
      }
      Free((ADDRESS)save,sizeof(indlist));
    }
  }
  else
  {
    res->Init(0);
    Free((ADDRESS)ISet, sizeof(indlist));
  }
  hKill(radmem, pVariables - 1);
  Free((ADDRESS)hpure, (1 + (pVariables * pVariables)) * sizeof(Exponent_t));
  Free((ADDRESS)hvar, (pVariables + 1) * sizeof(int));
  Free((ADDRESS)hwork, hNexist * sizeof(scmon));
  hDelete(hexist, hNexist);
  return res;
}

// multiplicity

static int hZeroMult(scmon pure, scfmon stc, int Nstc, varset var, int Nvar)
{
  int  iv = Nvar -1, sum, a, a0, a1, b, i;
  Exponent_t  x, x0;
  scmon pn;
  scfmon sn;
  if (!iv)
    return pure[var[1]];
  else if (!Nstc)
  {
    sum = 1;
    for (i = Nvar; i; i--)
      sum *= pure[var[i]];
    return sum;
  }
  x = a = 0;
  pn = hGetpure(pure);
  sn = hGetmem(Nstc, stc, stcmem[iv]);
  hStepS(sn, Nstc, var, Nvar, &a, &x);
  if (a == Nstc)
    return pure[var[Nvar]] * hZeroMult(pn, sn, a, var, iv);
  else
    sum = x * hZeroMult(pn, sn, a, var, iv);
  b = a;
  loop
  {
    a0 = a;
    x0 = x;
    hStepS(sn, Nstc, var, Nvar, &a, &x);
    hElimS(sn, &b, a0, a, var, iv);
    a1 = a;
    hPure(sn, a0, &a1, var, iv, pn, &i);
    hLex2S(sn, b, a0, a1, var, iv, hwork);
    b += (a1 - a0);
    if (a < Nstc)
    {
      sum += (x - x0) * hZeroMult(pn, sn, b, var, iv);
    }
    else
    {
      sum += (pure[var[Nvar]] - x0) * hZeroMult(pn, sn, b, var, iv);
      return sum;
    }
  }
}

static void hProject(scmon pure, varset sel)
{
  int  i, i0, k;
  i0 = 0;
  for (i = 1; i <= pVariables; i++)
  {
    if (pure[i])
    {
      i0++;
      sel[i0] = i;
    }
  }
  i = hNstc;
  memcpy(hwork, hstc, i * sizeof(scmon));
  hStaircase(hwork, &i, sel, i0);
  if ((i0 > 2) && (i > 10))
    hOrdSupp(hwork, i, sel, i0);
  memset(hpur0, 0, (pVariables + 1) * sizeof(Exponent_t));
  hPure(hwork, 0, &i, sel, i0, hpur0, &k);
  hLexS(hwork, i, sel, i0);
  hMu += hZeroMult(hpur0, hwork, i, sel, i0);
}

static void hDimMult(scmon pure, int Npure, scfmon rad, int Nrad,
 varset var, int Nvar)
{
  int  dn, iv, rad0, b, c, x;
  scmon pn;
  scfmon rn;
  if (Nrad < 2)
  {
    dn = Npure + Nrad;
    if (dn == hCo)
    {
      if (!Nrad)
        hProject(pure, hsel);
      else
      {
        pn = *rad;
        for (iv = Nvar; iv; iv--)
        {
          x = var[iv];
          if (pn[x])
          {
            pure[x] = 1;
            hProject(pure, hsel);
            pure[x] = 0;
          }
        }
      }
    }
    return;
  }
  iv = Nvar;
  dn = Npure+1;
  if (dn >= hCo)
  {
    if (dn > hCo)
      return;
    loop
    {
      if(!pure[var[iv]])
      {
        if(hNotZero(rad, Nrad, var, iv))
        {
          pure[var[iv]] = 1;
          hProject(pure, hsel);
          pure[var[iv]] = 0;
        }
      }
      iv--;
      if (!iv)
        return;
    }
  }
  while(pure[var[iv]]) iv--;
  hStepR(rad, Nrad, var, iv, &rad0);
  iv--;
  if (rad0 < Nrad)
  {
    pn = hGetpure(pure);
    rn = hGetmem(Nrad, rad, radmem[iv]);
    pn[var[iv + 1]] = 1;
    hDimMult(pn, Npure + 1, rn, rad0, var, iv);
    pn[var[iv + 1]] = 0;
    b = rad0;
    c = Nrad;
    hElimR(rn, &rad0, b, c, var, iv);
    hPure(rn, b, &c, var, iv, pn, &x);
    hLex2R(rn, rad0, b, c, var, iv, hwork);
    rad0 += (c - b);
    hDimMult(pn, Npure + x, rn, rad0, var, iv);
  }
  else
  {
    hDimMult(pure, Npure, rad, Nrad, var, iv);
  }
}

static void hDegree(ideal S, ideal Q)
{
  int  di;
  Exponent_t  mc;
  hexist = hInit(S, Q, &hNexist);
  if (!hNexist)
  {
    hCo = 0;
    hMu = 1;
    return;
  }
  hwork = (scfmon)Alloc(hNexist * sizeof(scmon));
  hvar = (varset)Alloc((pVariables + 1) * sizeof(int));
  hsel = (varset)Alloc((pVariables + 1) * sizeof(int));
  hpure = (scmon)Alloc((1 + (pVariables * pVariables)) * sizeof(Exponent_t));
  hpur0 = (scmon)Alloc((1 + (pVariables * pVariables)) * sizeof(Exponent_t));
  mc = hisModule;
  hrad = (scfmon)Alloc(hNexist * sizeof(scmon));
  if (!mc)
  {
    memcpy(hrad, hexist, hNexist * sizeof(scmon));
    hstc = hexist;
    hNrad = hNstc = hNexist;
  }
  else
    hstc = (scfmon)Alloc(hNexist * sizeof(scmon));
  radmem = hCreate(pVariables - 1);
  stcmem = hCreate(pVariables - 1);
  hCo = pVariables + 1;
  di = hCo + 1;
  loop
  {
    if (mc)
    {
      hComp(hexist, hNexist, mc, hrad, &hNrad);
      hNstc = hNrad;
      memcpy(hstc, hrad, hNrad * sizeof(scmon));
    }
    if (hNrad)
    {
      hNvar = pVariables;
      hRadical(hrad, &hNrad, hNvar);
      hSupp(hrad, hNrad, hvar, &hNvar);
      if (hNvar)
      {
        hCo = hNvar;
        memset(hpure, 0, (pVariables + 1) * sizeof(Exponent_t));
        hPure(hrad, 0, &hNrad, hvar, hNvar, hpure, &hNpure);
        hLexR(hrad, hNrad, hvar, hNvar);
        hDimSolve(hpure, hNpure, hrad, hNrad, hvar, hNvar);
      }
    }
    else
    {
      hNvar = 1;
      hCo = 0;
    }
    if (hCo < di)
    {
      di = hCo;
      hMu = 0;
    }
    if (hNvar && (hCo == di))
    {
      if (di && (di < pVariables))
        hDimMult(hpure, hNpure, hrad, hNrad, hvar, hNvar);
      else if (!di)
        hMu++;
      else
      {
        hStaircase(hstc, &hNstc, hvar, hNvar);
        if ((hNvar > 2) && (hNstc > 10))
          hOrdSupp(hstc, hNstc, hvar, hNvar);
        memset(hpur0, 0, (pVariables + 1) * sizeof(Exponent_t));
        hPure(hstc, 0, &hNstc, hvar, hNvar, hpur0, &hNpure);
        hLexS(hstc, hNstc, hvar, hNvar);
        hMu += hZeroMult(hpur0, hstc, hNstc, hvar, hNvar);
      }
    }
    mc--;
    if (mc <= 0)
      break;
  }
  hCo = di;
  hKill(stcmem, pVariables - 1);
  hKill(radmem, pVariables - 1);
  Free((ADDRESS)hpur0, (1 + (pVariables * pVariables)) * sizeof(Exponent_t));
  Free((ADDRESS)hpure, (1 + (pVariables * pVariables)) * sizeof(Exponent_t));
  Free((ADDRESS)hsel, (pVariables + 1) * sizeof(int));
  Free((ADDRESS)hvar, (pVariables + 1) * sizeof(int));
  Free((ADDRESS)hwork, hNexist * sizeof(scmon));
  Free((ADDRESS)hrad, hNexist * sizeof(scmon));
  hDelete(hexist, hNexist);
  if (hisModule)
    Free((ADDRESS)hstc, hNexist * sizeof(scmon));
}

int  scMultInt(ideal S, ideal Q)
{
  hDegree(S, Q);
  return hMu;
}

void scDegree(ideal S, ideal Q)
{
  hDegree(S, Q);
  if (pOrdSgn == 1)
    Print("// codimension = %d\n// dimension   = %d\n// degree      = %d\n",
      hCo, pVariables - hCo, hMu);
  else
    Print("// codimension  = %d\n// dimension    = %d\n// multiplicity = %d\n",
      hCo, pVariables - hCo, hMu);
}

static void hDegree0(ideal S, ideal Q)
{
  Exponent_t  mc;
  hexist = hInit(S, Q, &hNexist);
  if (!hNexist)
  {
    hMu = -1;
    return;
  }
  else
    hMu = 0;
  hwork = (scfmon)Alloc(hNexist * sizeof(scmon));
  hvar = (varset)Alloc((pVariables + 1) * sizeof(int));
  hpur0 = (scmon)Alloc((1 + (pVariables * pVariables)) * sizeof(Exponent_t));
  mc = hisModule;
  if (!mc)
  {
    hstc = hexist;
    hNstc = hNexist;
  }
  else
    hstc = (scfmon)Alloc(hNexist * sizeof(scmon));
  stcmem = hCreate(pVariables - 1);
  loop
  {
    if (mc)
    {
      hComp(hexist, hNexist, mc, hstc, &hNstc);
      if (!hNstc)
      {
        hMu = -1;
        break;
      }
    }
    hNvar = pVariables;
    for (int i = hNvar; i; i--)
      hvar[i] = i;
    hStaircase(hstc, &hNstc, hvar, hNvar);
    hSupp(hstc, hNstc, hvar, &hNvar);
    if ((hNvar == pVariables) && (hNstc >= pVariables))
    {
      if ((hNvar > 2) && (hNstc > 10))
        hOrdSupp(hstc, hNstc, hvar, hNvar);
      memset(hpur0, 0, (pVariables + 1) * sizeof(Exponent_t));
      hPure(hstc, 0, &hNstc, hvar, hNvar, hpur0, &hNpure);
      if (hNpure == hNvar)
      {
        hLexS(hstc, hNstc, hvar, hNvar);
        hMu += hZeroMult(hpur0, hstc, hNstc, hvar, hNvar);
      }
      else
        hMu = -1;
    }
    else if (hNvar)
      hMu = -1;
    mc--;
    if (mc <= 0 || hMu < 0)
      break;
  }
  hKill(stcmem, pVariables - 1);
  Free((ADDRESS)hpur0, (1 + (pVariables * pVariables)) * sizeof(Exponent_t));
  Free((ADDRESS)hvar, (pVariables + 1) * sizeof(int));
  Free((ADDRESS)hwork, hNexist * sizeof(scmon));
  hDelete(hexist, hNexist);
  if (hisModule)
    Free((ADDRESS)hstc, hNexist * sizeof(scmon));
}

int  scMult0Int(ideal S, ideal Q)
{
  hDegree0(S, Q);
  return hMu;
}


// HC

static poly pWork;

static void hHedge(poly hEdge)
{
  pSetm(pWork);
  if (pComp0(pWork, hEdge) == pOrdSgn)
  {
    for (int i = hNvar; i>0; i--)
      pSetExp(hEdge,i, pGetExp(pWork,i));
    pGetOrder(hEdge) = pGetOrder(pWork);
  }
}


static void hHedgeStep(scmon pure, scfmon stc,
                       int Nstc, varset var, int Nvar,poly hEdge)
{
  int  iv = Nvar -1, k = var[Nvar], a, a0, a1, b, i;
  Exponent_t  x, x0;
  scmon pn;
  scfmon sn;
  if (iv==0)
  {
    pSetExp(pWork, k, pure[k]);
    hHedge(hEdge);
    return;
  }
  else if (Nstc==0)
  {
    for (i = Nvar; i>0; i--)
      pSetExp(pWork, var[i], pure[var[i]]);
    hHedge(hEdge);
    return;
  }
  x = a = 0;
  pn = hGetpure(pure);
  sn = hGetmem(Nstc, stc, stcmem[iv]);
  hStepS(sn, Nstc, var, Nvar, &a, &x);
  if (a == Nstc)
  {
    pSetExp(pWork, k, pure[k]);
    hHedgeStep(pn, sn, a, var, iv,hEdge);
    return;
  }
  else
  {
    pSetExp(pWork, k, x);
    hHedgeStep(pn, sn, a, var, iv,hEdge);
  }
  b = a;
  loop
  {
    a0 = a;
    x0 = x;
    hStepS(sn, Nstc, var, Nvar, &a, &x);
    hElimS(sn, &b, a0, a, var, iv);
    a1 = a;
    hPure(sn, a0, &a1, var, iv, pn, &i);
    hLex2S(sn, b, a0, a1, var, iv, hwork);
    b += (a1 - a0);
    if (a < Nstc)
    {
      pSetExp(pWork, k, x);
      hHedgeStep(pn, sn, b, var, iv,hEdge);
    }
    else
    {
      pSetExp(pWork, k, pure[k]);
      hHedgeStep(pn, sn, b, var, iv,hEdge);
      return;
    }
  }
}


void scComputeHC(ideal S, int ak, poly &hEdge)
{
  int  i;
  Exponent_t  k = ak;
  hNvar = pVariables;
  hexist = hInit(S, NULL, &hNexist);
  if (k!=0)
    hComp(hexist, hNexist, k, hexist, &hNstc);
  else
    hNstc = hNexist;
  hwork = (scfmon)Alloc(hNexist * sizeof(scmon));
  hvar = (varset)Alloc((hNvar + 1) * sizeof(int));
  hpure = (scmon)Alloc((1 + (hNvar * hNvar)) * sizeof(Exponent_t));
  stcmem = hCreate(hNvar - 1);
  for (i = hNvar; i>0; i--)
    hvar[i] = i;
  hStaircase(hexist, &hNstc, hvar, hNvar);
  if ((hNvar > 2) && (hNstc > 10))
    hOrdSupp(hexist, hNstc, hvar, hNvar);
  memset(hpure, 0, (hNvar + 1) * sizeof(Exponent_t));
  hPure(hexist, 0, &hNstc, hvar, hNvar, hpure, &hNpure);
  hLexS(hexist, hNstc, hvar, hNvar);
  if (hEdge!=NULL)
    pFree1(hEdge);
  hEdge = pInit();
  pWork = pInit();
  hHedgeStep(hpure, hexist, hNstc, hvar, hNvar,hEdge);
  pSetComp(hEdge,ak);
  hKill(stcmem, hNvar - 1);
  Free((ADDRESS)hwork, hNexist * sizeof(scmon));
  Free((ADDRESS)hvar, (hNvar + 1) * sizeof(int));
  Free((ADDRESS)hpure, (1 + (hNvar * hNvar)) * sizeof(Exponent_t));
  hDelete(hexist, hNexist);
  pFree1(pWork);
}


//  kbase

static poly last;
static scmon act;

static void scElKbase()
{
  poly q = pInit();
  pSetCoeff0(q,nInit(1));
  pSetExpV(q,act);
  pNext(q) = NULL;
  last = pNext(last) = q;
}

static Exponent_t scMax( int i, scfmon stc, int Nvar)
{
  Exponent_t x, y=stc[0][Nvar];
  for (; i;)
  {
    i--;
    x = stc[i][Nvar];
    if (x > y) y = x;
  }
  return y;
}

static Exponent_t scMin( int i, scfmon stc, int Nvar)
{
  Exponent_t x, y=stc[0][Nvar];
  for (; i;)
  {
    i--;
    x = stc[i][Nvar];
    if (x < y) y = x;
  }
  return y;
}

static Exponent_t scRestrict( int &Nstc, scfmon stc, int Nvar)
{
  Exponent_t x, y;
  int i, j, Istc = Nstc;

  y = MAX_EXPONENT;
  for (i=Nstc-1; i>=0; i--)
  {
    j = Nvar-1;
    loop
    {
      if(stc[i][j] != 0) break;
      j--;
      if (j == 0)
      {
        Istc--;
        x = stc[i][Nvar];
        if (x < y) y = x;
        stc[i] = NULL;
        break;
      }
    }
  }
  if (Istc < Nstc)
  {
    for (i=Nstc-1; i>=0; i--)
    {
      if (stc[i] && (stc[i][Nvar] >= y))
      {
	Istc--;
	stc[i] = NULL;
      }
    }
    j = 0;
    while (stc[j]) j++;
    i = j+1;
    for(; i<Nstc; i++)
    {
      if (stc[i])
      {
        stc[j] = stc[i];
        j++;
      }
    }
    Nstc = Istc;
    return y;
  }
  else
    return -1;
}

static void scAll( int Nvar, Exponent_t deg)
{
  int i;
  Exponent_t d = deg;
  if (d == 0)
  {
    for (i=Nvar; i; i--) act[i] = 0;
    scElKbase();
    return;
  }
  if (Nvar == 1)
  {
    act[1] = d;
    scElKbase();
    return;
  }
  do
  {
    act[Nvar] = d;
    scAll(Nvar-1, deg-d);
    d--;
  } while (d >= 0);
}

static void scAllKbase( int Nvar, Exponent_t ideg, Exponent_t deg)
{
  do
  {
    act[Nvar] = ideg;
    scAll(Nvar-1, deg-ideg);
    ideg--;
  } while (ideg >= 0);
}

static void scDegKbase( scfmon stc, int Nstc, int Nvar, Exponent_t deg)
{
  int  Ivar, Istc, i, j;
  scfmon sn;
  Exponent_t x, ideg;

  if (deg == 0)
  {
    for (i=Nvar; i; i--) act[i] = 0;
    scElKbase();
    return;
  }
  if (Nvar == 1)
  {
    for (i=Nstc-1; i>=0; i--) if(deg >= stc[i][1]) return;
    act[1] = deg;
    scElKbase();
    return;
  }
  Ivar = Nvar-1;
  sn = hGetmem(Nstc, stc, stcmem[Ivar]);
  x = scRestrict(Nstc, sn, Nvar);
  if (x <= 0)
  {
    if (x == 0) return;
    ideg = deg;
  }
  else
  {
    if (deg < x) ideg = deg;
    else ideg = x-1;
    if (Nstc == 0)
    {
      scAllKbase(Nvar, ideg, deg);
      return;
    }
  }
  loop
  {
    x = scMax(Nstc, sn, Nvar);
    while (ideg >= x)
    {
      act[Nvar] = ideg;
      scDegKbase(sn, Nstc, Ivar, deg-ideg);
      ideg--;
    }
    if (ideg < 0) return;
    Istc = Nstc;
    for (i=Nstc-1; i>=0; i--)
    {
      if (ideg < sn[i][Nvar])
      {
        Istc--;
	sn[i] = NULL;
      }
    }
    if (Istc == 0)
    {
      scAllKbase(Nvar, ideg, deg);
      return;
    }
    j = 0;
    while (sn[j]) j++;
    i = j+1;
    for (; i<Nstc; i++)
    {
      if (sn[i])
      {
	sn[j] = sn[i];
	j++;
      }
    }
    Nstc = Istc;
  }
}

static void scInKbase( scfmon stc, int Nstc, int Nvar)
{
  int  Ivar, Istc, i, j;
  scfmon sn;
  Exponent_t x, ideg;

  if (Nvar == 1)
  {
    ideg = scMin(Nstc, stc, 1);
    while (ideg > 0)
    {
      ideg--;
      act[1] = ideg;
      scElKbase();
    }
    return;
  }
  Ivar = Nvar-1;
  sn = hGetmem(Nstc, stc, stcmem[Ivar]);
  x = scRestrict(Nstc, sn, Nvar);
  if (x == 0) return;
  ideg = x-1;
  loop
  {
    x = scMax(Nstc, sn, Nvar);
    while (ideg >= x)
    {
      act[Nvar] = ideg;
      scInKbase(sn, Nstc, Ivar);
      ideg--;
    }
    if (ideg < 0) return;
    Istc = Nstc;
    for (i=Nstc-1; i>=0; i--)
    {
      if (ideg < sn[i][Nvar])
      {
        Istc--;
	sn[i] = NULL;
      }
    }
    j = 0;
    while (sn[j]) j++;
    i = j+1;
    for (; i<Nstc; i++)
    {
      if (sn[i])
      {
	sn[j] = sn[i];
	j++;
      }
    }
    Nstc = Istc;
  }
}

static ideal scIdKbase()
{
  polyset mm;
  ideal res;
  poly p, q = last;
  int i = pLength(q);
  res = idInit(i,1);
  mm = res->m;
  i = 0;
  do
  {
    mm[i] = q;
    i++;
    p = pNext(q);
    pNext(q) = NULL;
    q = p;
  } while (q!=NULL);
  return res;
}

extern ideal scKBase(int deg, ideal s, ideal Q)
{
  int  i, di;
  poly p;

  if (deg < 0)
  {
    di = scDimInt(s, Q);
    if (di != 0)
    {
      Werror("KBase not finite");
      return idInit(1,1);
    }
  }
  stcmem = hCreate(pVariables - 1);
  hexist = hInit(s, Q, &hNexist);
  p = last = pInit();
  /*pNext(p) = NULL;*/
  act = (scmon)Alloc((pVariables + 1) * sizeof(Exponent_t));
  *act = 0;
  if (!hNexist)
  {
    scAll(pVariables, deg);
    goto ende;
  }
  if (!hisModule)
  {
    if (deg < 0) scInKbase(hexist, hNexist, pVariables);
    else scDegKbase(hexist, hNexist, pVariables, deg);
  }
  else
  {
    hstc = (scfmon)Alloc(hNexist * sizeof(scmon));
    for (i = 1; i <= hisModule; i++)
    {
      *act = i;
      hComp(hexist, hNexist, i, hstc, &hNstc);
      if (hNstc)
      {
        if (deg < 0) scInKbase(hstc, hNstc, pVariables);
        else scDegKbase(hstc, hNstc, pVariables, deg);
      }
      else
        scAll(pVariables, deg);
    }
    Free((ADDRESS)hstc, hNexist * sizeof(scmon));
  }
ende:
  hDelete(hexist, hNexist);
  Free((ADDRESS)act, (pVariables + 1) * sizeof(Exponent_t));
  hKill(stcmem, pVariables - 1);
  pDelete1(&p);
  if (p == NULL)
    return idInit(1,s->rank);
  else
  {
    last = p;
    ideal res=scIdKbase();
    res->rank=s->rank;
    return res;
  }
}


