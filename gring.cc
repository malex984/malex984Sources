/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/***************************************************************
 *  File:    gring.cc
 *  Purpose: p_Mult family of procedures
 *  Author:  levandov (Viktor Levandovsky)
 *  Created: 8/00 - 11/00
 *  Version: $Id: gring.cc,v 1.14 2002-06-06 16:02:10 levandov Exp $
 *******************************************************************/
#include "mod2.h"
#ifdef HAVE_PLURAL
#include "gring.h"
#include "febase.h"
#include "ring.h"
#include "polys.h"
#include "numbers.h"
#include "ideals.h"
#include "matpol.h"
#include "kbuckets.h"

/* global nc_macros : */
#define freeT(A,v) omFreeSize((ADDRESS)A,(v+1)*sizeof(Exponent_t))
#define freeN(A,k) omFreeSize((ADDRESS)A,k*sizeof(number))

/* poly functions defined in p_Procs : */
poly nc_pp_Mult_mm(poly p, const poly m, const ring r, poly &last)
{
  return(nc_p_Mult_mm(p_Copy(p,r),m,r));
}

/* poly nc_p_Mult_mm(poly p, poly m, const ring r); defined below */
poly nc_p_Minus_mm_Mult_qq(poly p, const poly m, poly q, const ring r)
{
  number minus1=n_Init(-1,r);
  poly mc=p_Mult_nn(p_Copy(m,r),minus1,r);
  poly mmc=nc_mm_Mult_p(mc,p_Copy(q,r),r);
  p_Delete(&mc,r);
  p=p_Add_q(p,mmc,r);
  n_Delete(&minus1,r);
  return(p);
}

//----------- auxiliary routines--------------------------
poly _nc_p_Mult_q(poly p, poly q, const int copy, const ring r)
  /* destroy p,q */
{
  poly res=NULL;
  poly ghost=NULL;
  while (q!=NULL)
    {
      res=p_Add_q(res,nc_pp_Mult_mm(p,p_Head(q,r),r,ghost),r);
      q=p_LmDeleteAndNext(q,r);
    }
  p_Delete(&p,r);
  return(res);
}

poly  nc_p_Mult_mm(poly p, const poly m, const ring r)
/* p is poly, m is mono with coeff, p killed after */
{
  if ((p==NULL) || (m==NULL)) return(NULL);
  /*  if (pNext(p)==NULL) return(nc_mm_Mult_nn(p,pCopy(m),r)); */
  /* excluded  - the cycle will do it anyway - OK. */
  if (p_IsConstant(m,r)) return(p_Mult_nn(p,p_GetCoeff(m,r),r));

#ifdef PDEBUG
  p_Test(p,r);
  p_Test(m,r);
#endif
  poly v=NULL;
  poly out=NULL;
  Exponent_t *P=(Exponent_t *)omAlloc0((r->N+1)*sizeof(Exponent_t));
  Exponent_t *M=(Exponent_t *)omAlloc0((r->N+1)*sizeof(Exponent_t));
  /* coefficients: */
  number cP,cM,cOut;
  p_GetExpV(m,M,r);
  cM=p_GetCoeff(m,r);
  /* components:*/
  const Exponent_t expM=p_GetComp(m,r);
  Exponent_t expP=0;
  Exponent_t expOut=0;
  
  while (p!=NULL)
  {
    v=p_Head(p,r);
    p_Test(v,r);
    p_Test(p,r);
    
    expP=p_GetComp(v,r);
    if (expP==0)
    {
      if (expM==0)
      {
        expOut=0;
      }
      else
      {
        expOut=expM;
      }      
    }
    else
    {
      if (expM==0)
      {
        expOut=expP;
      }
      else
      {
        /* REPORT_ERROR */
	Print("nc_p_Mult_mm: exponent mismatch %d and %d\n",expP,expM);
        expOut=0;
      }
    }
    
    p_GetExpV(v,P,r);
    cP=p_GetCoeff(v,r);
    v= nc_mm_Mult_nn(P,M,r);
    cOut=n_Mult(cP,cM,r);
    v=p_Mult_nn(v,cOut,r);
    p_SetCompP(v,expOut,r);
    out = p_Add_q(out,v,r);
    p=p_LmDeleteAndNext(p,r);
  }
  freeT(P,r->N);
  freeT(M,r->N);
  p_Test(out,r);
  return(out);
}

poly nc_mm_Mult_p(const poly m, poly p, const ring r)
/* p is poly, m is mono with coeff, p killed after */
/* former pMultT2 */
{
  if ((p==NULL) || (m==NULL)) return(NULL);
  /*  if (pNext(p)==NULL) return(nc_mm_Mult_nn(p,pCopy(m),r)); */
  /* excluded  - the cycle will do it anyway - OK.*/
  if (p_IsConstant(m,r)) return(p_Mult_nn(p,p_GetCoeff(m,r),r));

#ifdef PDEBUG
  p_Test(p,r);
  p_Test(m,r);
#endif
  poly v=NULL;
  poly out=NULL;
  Exponent_t *P=(Exponent_t *)omAlloc0((r->N+1)*sizeof(Exponent_t));
  Exponent_t *M=(Exponent_t *)omAlloc0((r->N+1)*sizeof(Exponent_t));
  /* coefficients: */
  number cP,cM,cOut;
  p_GetExpV(m,M,r);
  cM=p_GetCoeff(m,r);
  /* components:*/
  const Exponent_t expM=p_GetComp(m,r);
  Exponent_t expP=0;
  Exponent_t expOut=0;
  
  while (p!=NULL)
  {
    v=p_Head(p,r);
    p_Test(v,r);
    p_Test(p,r);
    
    expP=p_GetComp(v,r);
    if (expP==0)
    {
      if (expM==0)
      {
        expOut=0;
      }
      else
      {
        expOut=expM;
      }      
    }
    else
    {
      if (expM==0)
      {
        expOut=expP;
      }
      else
      {
        /* REPORT_ERROR */
	Print("nc_mm_Mult_p: exponent mismatch %d and %d\n",expP,expM);
        expOut=0;
      }
    }
    
    p_GetExpV(v,P,r);
    cP=p_GetCoeff(v,r);
    v= nc_mm_Mult_nn(M,P,r);
    cOut=n_Mult(cP,cM,r);
    v=p_Mult_nn(v,cOut,r);
    p_SetCompP(v,expOut,r);
    out = p_Add_q(out,v,r);
    p_DeleteLm(&p,r);
  }
  freeT(P,r->N);
  freeT(M,r->N);
  return(out);
}

poly nc_mm_Mult_nn(Exponent_t *F0, Exponent_t *G0, const ring r)
/* destroys nothing, no coeffs and exps */
{
  poly out=NULL;
  int i;
  int iF,jG,iG;
  int ExpSize=(r->N+1)*sizeof(Exponent_t);

  Exponent_t *F=(Exponent_t *)omAlloc0(ExpSize);
  Exponent_t *G=(Exponent_t *)omAlloc0(ExpSize);

  memcpy(F, F0,(r->N+1)*sizeof(Exponent_t));
  // pExpVectorCopy(F,F0);
  memcpy(G, G0,(r->N+1)*sizeof(Exponent_t));
  //  pExpVectorCopy(G,G0);
  F[0]=0;
  G[0]=0;
  
  iF=r->N;
  while ((F[iF]==0)&&(iF>=1)) iF--; /* last exp_num of F */
  jG=1;
  while ((G[jG]==0)&&(jG<=r->N)) jG++;  /* first exp_num of G */
  iG=r->N;
  while ((G[iG]==0)&&(iG>=1)) iG--;  /* last exp_num of G */

  if (iF<=jG)
    /* i.e. no mixed exp_num , MERGE case */
  {
    out=pOne();
    for (i=1;i<=r->N;i++)
    {
      F[i]=F[i]+G[i];
    }  
    p_SetExpV(out,F,r);
    p_Setm(out,r);
    freeT(F,r->N);
    freeT(G,r->N);
    return(out);
  }

  if (iG==jG)
    /* g is univariate monomial */
  {
    /*    if (ri->nc->type==nc_skew) -- postpone to TU */
    out=nc_mm_Mult_uu(F,jG,G[jG],r);
    freeT(F,r->N);
    freeT(G,r->N);
    return(out);
  }
  
  number n1=n_Init(1,r);
  Exponent_t *Prv=(Exponent_t *)omAlloc0(ExpSize);
  Exponent_t *Nxt=(Exponent_t *)omAlloc0(ExpSize);

  int *log=(int *)omAlloc0((r->N+1)*sizeof(int));
  int cnt=0; int cnf=0;

  /* splitting F wrt jG */
  for (i=1;i<=jG;i++)
  {
    Prv[i]=F[i]; Nxt[i]=0; /* mult at the very end */
    if (F[i]!=0) cnf++;
  }

  if (cnf==0) freeT(Prv,r->N);

  for (i=jG+1;i<=r->N;i++)
  {
    Nxt[i]=F[i];
    /*    if (cnf!=0)  Prv[i]=0; */
    if (F[i]!=0)
    {
      cnt++;
    }              /* effective part for F */
  }
  freeT(F,r->N);
  cnt=0;

  for (i=1;i<=r->N;i++)
  {
    if (G[i]!=0)
    {
     cnt++;
     log[cnt]=i;
     }               /* lG for G */
   }

/* ---------------------- A C T I O N ------------------------ */
  poly D=NULL;
  poly Rout=NULL;
  number *c=(number *)omAlloc0((r->N+1)*sizeof(number));
  c[0]=n_Init(1,r);

  Exponent_t *Op=Nxt;
  Exponent_t *On=G;
  Exponent_t *U=(Exponent_t *)omAlloc0(ExpSize);

  for (i=jG;i<=r->N;i++) U[i]=Nxt[i]+G[i];  /* make leadterm */
  Nxt=NULL;
  G=NULL;
  cnt=1;
  int t=0;
  poly w=NULL;
  poly Pn=pOne();
  p_SetExpV(Pn,On,r);
  p_Setm(Pn,r);

  while (On[iG]!=0)
  {
     t=log[cnt];

     w=nc_mm_Mult_uu(Op,t,On[t],r);
     c[cnt]=n_Mult(c[cnt-1],p_GetCoeff(w,r),r);
     D = pNext(w);  /* getting coef and rest D */
     p_DeleteLm(&w,r);
     w=NULL;

     Op[t] += On[t];   /* update exp_vectors */
     On[t] = 0;

     if (t!=iG)    /* not the last step */
     {
       p_SetExpV(Pn,On,r);
       p_Setm(Pn,r);
       p_Test(Pn,r);
       
//       if (pNext(D)==0)
// is D a monomial? could be postponed higher
//       {
//       Rout=nc_mm_Mult_nn(D,Pn,r);
//       }
//       else
//       {
       Rout=nc_p_Mult_mm(D,Pn,r); 
//       }
     }
     else
     {
       Rout=D;
       D=NULL;
     }
     
     if (Rout!=NULL)
     {
       Rout=p_Mult_nn(Rout,c[cnt-1],r); /* Rest is ready */
       out=p_Add_q(out,Rout,r);
       Rout=NULL;
     }
     cnt++;
  }
  freeT(On,r->N);
  freeT(Op,r->N);
  p_Delete(&Pn,r);
  omFreeSize((ADDRESS)log,(r->N+1)*sizeof(int));

  /* leadterm and Prv-part */

  Rout=pOne();
  /* U is lead.monomial */
  U[0]=0;
  p_SetExpV(Rout,U,r);
  p_Setm(Rout,r);  /* use again this name Rout */
  p_Test(Rout,r);
  p_SetCoeff(Rout,c[cnt-1],r);
  out=p_Add_q(out,Rout,r);
  freeT(U,r->N);
  freeN(c,r->N+1);
  if (cnf!=0)  /* Prv is non-zero vector */
  {
    Rout=pOne();
    Prv[0]=0;
    p_SetExpV(Rout,Prv,r);
    p_Setm(Rout,r);
    p_Test(Rout,r);
    out=nc_mm_Mult_p(Rout,out,r); /* getting the final result */
    freeT(Prv,r->N);
    p_Delete(&Rout,r);
  }
  return (out);
}


poly nc_mm_Mult_uu(Exponent_t *F,int jG,int bG, const ring r)
/* f=mono(F),g=(x_iG)^bG */
{
  poly out=NULL;
  int i;
  number num=NULL;
  
  int iF=r->N;
  while ((F[iF]==0)&&(iF>0)) iF-- ;   /* last exponent_num of F */

  if (iF==0)  /* F==zero vector in other words */
  {
   out=pOne();
   p_SetExp(out,jG,bG,r);
   p_Setm(out,r);
   return(out);
  }

  int jF=1;
  while ((F[jF]==0)&&(jF<=r->N)) jF++;  /* first exp of F */

  if (iF<=jG)                       /* i.e. no mixed exp_num */
  {
    out=pOne();
    F[jG]=F[jG]+bG;
    p_SetExpV(out,F,r);
    p_Setm(out,r);
    return(out);
  }

  if (iF==jF)              /* uni times uni */
  {
   out=nc_uu_Mult_ww(iF,F[iF],jG,bG,r);
   return(out);
  }
  
  Exponent_t *Prv=(Exponent_t*)omAlloc0((r->N+1)*sizeof(Exponent_t));
  Exponent_t *Nxt=(Exponent_t*)omAlloc0((r->N+1)*sizeof(Exponent_t));
  int *lF=(int *)omAlloc0((r->N+1)*sizeof(int));
  int cnt=0; int cnf=0;
  /* splitting F wrt jG */
  for (i=1;i<=jG;i++) /* mult at the very end */
  {
    Prv[i]=F[i]; Nxt[i]=0;
    if (F[i]!=0) cnf++;
  }
  if (cnf==0)  freeT(Prv,r->N);
  for (i=jG+1;i<=r->N;i++)
  {
    Nxt[i]=F[i];
    if (cnf!=0) { Prv[i]=0;}
    if (F[i]!=0)
    {
     cnt++;
     lF[cnt]=i;
     }                 /* eff_part,lF_for_F */
  }

  if (cnt==1) /* Nxt consists of 1 nonzero el-t only */
  {
    int q=lF[1];
    poly Rout=pOne();
    out=nc_uu_Mult_ww(q,Nxt[q],jG,bG,r);
    freeT(Nxt,r->N);

    if (cnf!=0)
    {
       Prv[0]=0;
       p_SetExpV(Rout,Prv,r);
       p_Setm(Rout,r);
       p_Test(Rout,r);
       freeT(Prv,r->N);
       out=nc_mm_Mult_p(Rout,out,r); /* getting the final result */
    }

    omFreeSize((ADDRESS)lF,(r->N+1)*sizeof(int));
    p_Delete(&Rout,r);
    return (out);
  }
/* -------------------- MAIN ACTION --------------------- */

  poly D=NULL;
  poly Rout=NULL;
  number *c=(number *)omAlloc0((cnt+2)*sizeof(number));
  c[cnt+1]=n_Init(1,r);
  i=cnt+2; 	/* later in freeN */
  Exponent_t *Op=Nxt;
  Exponent_t *On=(Exponent_t *)omAlloc0((r->N+1)*sizeof(Exponent_t));
  Exponent_t *U=(Exponent_t *)omAlloc0((r->N+1)*sizeof(Exponent_t));


  //  pExpVectorCopy(U,Nxt);
  memcpy(U, Nxt,(r->N+1)*sizeof(Exponent_t));
  U[jG] = U[jG] + bG;

  /* Op=Nxt and initial On=(0); */
  Nxt=NULL;

  poly Pp;
  poly Pn;
  int t=0;
  int first=lF[1];
  int nlast=lF[cnt];
  int kk=0;
  /*  cnt--;   */
  /* now lF[cnt] should be <=iF-1 */

  while (Op[first]!=0)
  {
     t=lF[cnt];   /* cnt as it was computed */

     poly w=nc_uu_Mult_ww(t,Op[t],jG,bG,r);
     c[cnt]=n_Copy(p_GetCoeff(w,r),r);
     D = pNext(w);  /* getting coef and rest D */
     p_DeleteLm(&w,r);
     w=NULL;

     Op[t]= 0;
     Pp=pOne();
     p_SetExpV(Pp,Op,r);
     p_Setm(Pp,r);

     if (t<nlast)
     {
       kk=lF[cnt+1];
       On[kk]=F[kk];
       
       Pn=pOne();
       p_SetExpV(Pn,On,r);
       p_Setm(Pn,r);

       if (t!=first)   /* typical expr */
       {
         w=nc_p_Mult_mm(D,Pn,r);
         Rout=nc_mm_Mult_p(Pp,w,r);
         w=NULL;
       }
       else                   /* last step */
       {
         On[t]=0;
         p_SetExpV(Pn,On,r);
         p_Setm(Pn,r);
         Rout=nc_p_Mult_mm(D,Pn,r);
       }
       p_Test(Pp,r);
       p_Delete(&Pn,r);
     }
     else                     /* first step */
     {
       Rout=nc_mm_Mult_p(Pp,D,r);
     }

     p_Test(Pp,r);
     p_Delete(&Pp,r);
     num=n_Mult(c[cnt+1],c[cnt],r);
     n_Delete(&c[cnt],r);
     c[cnt]=num;
     Rout=p_Mult_nn(Rout,c[cnt+1],r); /* Rest is ready */
     out=p_Add_q(out,Rout,r);
     Pp=NULL;
     cnt--;
  }
  /* only to feel safe:*/
  Pn=Pp=NULL;
  freeT(On,r->N);
  freeT(Op,r->N);

/* leadterm and Prv-part with coef 1 */
/*  U[0]=exp; */
/*  U[jG]=U[jG]+bG;  */
/* make leadterm */
/* ??????????? we have done it already :-0 */
  Rout=pOne();
  p_SetExpV(Rout,U,r);
  p_Setm(Rout,r);  /* use again this name */
  p_SetCoeff(Rout,c[cnt+1],r);  /* last computed coef */
  out=p_Add_q(out,Rout,r);
  Rout=NULL;
  freeT(U,r->N);
  freeN(c,i);
  omFreeSize((ADDRESS)lF,(r->N+1)*sizeof(int));

  if (cnf!=0)
  {
    Rout=pOne();
    p_SetExpV(Rout,Prv,r);
    p_Setm(Rout,r);
    freeT(Prv,r->N);
    out=nc_mm_Mult_p(Rout,out,r); /* getting the final result */
    p_Delete(&Rout,r);
  }
  return (out);
}

poly nc_uu_Mult_ww (int i, int a, int j, int b, const ring r)
{
  poly out=pOne();
  number tmp_number=NULL;
  
  /* Now check zero exeptions, commutativity and should we do something at all?  */
  if (i==j)
  {
    p_SetExp(out,j,a+b,r);
  }
  else
  {
    p_SetExp(out,j,b,r);
    p_SetExp(out,i,a,r);
  }
  p_Setm(out,r);
  if ((a==0)||(b==0)||(i<=j)) return(out);/* zero exeptions and usual case */
  
  if (MATELEM(r->nc->COM,j,i)!=NULL)
    /* commutative or quasicommutative case */
  {
    if (r->cf->nIsOne(p_GetCoeff(MATELEM(r->nc->COM,j,i),r))) /* commutative case */
    {
      return(out);
    }     
    else
    {
      tmp_number=p_GetCoeff(MATELEM(r->nc->COM,j,i),r); /* quasicommutative case */
      nPower(tmp_number,a*b,&tmp_number);
      p_SetCoeff(out,tmp_number,r);
      return(out);
    }
  }/* end_of commutative or quasicommutative case */

  /* we are here if  i>j and variables do not commute or quasicommute */
  /* in fact, now a>=1 and b>=1; and j<i */
  /* now check wether the polynom is alredy computed */
  int vik = UPMATELEM(j,i,r->N);
  matrix cMT=r->nc->MT[vik];
  int cMTsize=r->nc->MTsize[vik];

  if (((a<cMTsize)&&(b<cMTsize))&&(MATELEM(cMT,a,b)!=NULL))
  {
    out=p_Copy(MATELEM(cMT,a,b),r);
    return (out);
  }
  
  /*  poly C=MATELEM(r->nc->C,j,i);               */
  /*  number c=p_GetCoeff(C,r); //coeff           */
  /*  p_Delete(&C,r); */
      
  int newcMTsize=0;
  int k,m;
  p_Delete(&out,r);

  if (a>=b) {newcMTsize=a;} else {newcMTsize=b;}
  if (newcMTsize>cMTsize)
  {
     newcMTsize = newcMTsize+cMTsize;
     matrix tmp = mpNew(newcMTsize,newcMTsize);
     
     for (k=1;k<=cMTsize;k++)
     {
        for (m=1;m<=cMTsize;m++)
        {
           MATELEM(tmp,k,m) = MATELEM(r->nc->MT[UPMATELEM(j,i,r->N)],k,m);
	   //	   omCheckAddr(tmp->m);
           MATELEM(r->nc->MT[UPMATELEM(j,i,r->N)],k,m)=NULL;
	   //	   omCheckAddr(r->nc->MT[UPMATELEM(j,i,r->N)]->m);
        }
     }
     id_Delete((ideal *)&(r->nc->MT[UPMATELEM(j,i,r->N)]),r);
     r->nc->MT[UPMATELEM(j,i,r->N)] = tmp;
     r->nc->MTsize[UPMATELEM(j,i,r->N)] = newcMTsize;
  }  /* The update of multiplication matrix is finished */

  cMT=r->nc->MT[UPMATELEM(j,i,r->N)];         /* cMT=current MT */

  poly x=pOne();p_SetExp(x,j,1,r);p_Setm(x,r);p_Test(x,r);/* var(j); */
  poly y=pOne();p_SetExp(y,i,1,r);p_Setm(y,r);p_Test(y,r);/*var(i);  for convenience */
  
  poly t=NULL;
/* ------------ Main Cycles ----------------------------*/

  for (k=2;k<=a;k++)
  {
     t=MATELEM(cMT,k,1);

     if (t==NULL)   /* not computed yet */
     {
        t=p_Copy(MATELEM(cMT,k-1,1),r);
        t = nc_mm_Mult_p(y,t,r);
        MATELEM(cMT,k,1) = t;
	//	omCheckAddr(cMT->m);
     }
     t=NULL;
  }
  
  for (m=2;m<=b;m++)
  {
     t=MATELEM(cMT,a,m);
     if (t==NULL)   //not computed yet
     {
        t=p_Copy(MATELEM(cMT,a,m-1),r);
        t = nc_p_Mult_mm(t,x,r);
        MATELEM(cMT,a,m) = t;
	//	omCheckAddr(cMT->m);
     }
     t=NULL;
  }
  p_Delete(&x,r);
  p_Delete(&y,r);
  t=MATELEM(cMT,a,b);
  return(p_Copy(t,r));  /* as the last computed element was cMT[a,b] */
}


/* ----------------------------- Syzygies ---------------------- */

/*2
* reduction of p2 with p1
* do not destroy p1, but p2
* p1 divides p2 -> for use in NF algorithm
*/

poly nc_spGSpolyRed(poly p1, poly p2,poly spNoether, const ring r)
{
  if (p_GetComp(p1,r)!=p_GetComp(p2,r))
  {
    Print("nc_spGSpolyRed: different components");    
    return(NULL);
  }
  poly m=pOne();
  p_ExpVectorDiff(m,p2,p1,r);
  p_Setm(m,r); 
  p_Test(m,r);
  /* pSetComp(m,r)=0? */
  poly N=nc_mm_Mult_p(m,p_Head(p1,r),r);
  number C=n_Copy(p_GetCoeff(N,r),r);
  number cF=n_Copy(p_GetCoeff(p2,r),r);
  p2=p_Mult_nn(p2,C,r);
  poly out=nc_mm_Mult_p(m,p_LmDeleteAndNext(p_Copy(p1,r),r),r);
  N=p_Add_q(N,out,r);
  number MinusOne=n_Init(-1,r);
  if (!n_Equal(cF,MinusOne,r)) 
  {
    cF=n_Neg(cF,r);
    N=p_Mult_nn(N,cF,r);
  }
  out=p_Add_q(p2,N,r);
  p_Delete(&m,r);
  n_Delete(&cF,r);
  n_Delete(&C,r);
  n_Delete(&MinusOne,r);
  return(out);
}


/*3
* reduction of p2 with p1
* do not destroy p1 and p2
* p1 divides p2 -> for use in NF algorithm
*/
poly nc_spGSpolyRedNew(poly p1, poly p2,poly spNoether, const ring r)
{
  return(nc_spGSpolyRed(p1,p_Copy(p2,r),spNoether,r));
}

/*4
* creates the S-polynomial of p1 and p2
* do not destroy p1 and p2
*/
poly nc_spGSpolyCreate(poly p1, poly p2,poly spNoether, const ring r)
{
  if (p_GetComp(p1,r)!=p_GetComp(p2,r))
  {
    Print("nc_spGSpolyCreate : different components!");    
    return(NULL);
  }
  if ((r->nc->type==nc_lie) && pHasNotCF(p1,p2)) /* prod crit */
  {
    return(nc_p_Bracket_qq(pCopy(p2),p1));
  }
  poly pL=pOne();
  poly m1=pOne();
  poly m2=pOne();
  pLcm(p1,p2,pL);
  p_Setm(pL,r);
  p_Test(pL,r);
  p_ExpVectorDiff(m1,pL,p1,r);
  p_Setm(m1,r);
  p_Test(m1,r);
  p_ExpVectorDiff(m2,pL,p2,r);
  p_Setm(m2,r);
  p_Test(m2,r);
  p_Delete(&pL,r);
  /* zero exponents ! */
  poly M1=nc_mm_Mult_p(m1,p_Head(p1,r),r);
  number C1=n_Copy(p_GetCoeff(M1,r),r);
  poly M2=nc_mm_Mult_p(m2,p_Head(p2,r),r);
  number C2=n_Copy(p_GetCoeff(M2,r),r);
  M1=p_Mult_nn(M1,C2,r);
  p_SetCoeff(m1,C2,r);
  number MinusOne=n_Init(-1,r);
  if (n_Equal(C1,MinusOne,r))
  {
    M2=p_Add_q(M1,M2,r);
  }
  else
  {
    C1=n_Neg(C1,r);
    M2=p_Mult_nn(M2,C1,r);
    M2=p_Add_q(M1,M2,r);
    p_SetCoeff(m2,C1,r);
  }
  /* M1 is killed, M2=res = C2 M1 - C1 M2 */
  poly tmp=p_Copy(p1,r);
  tmp=p_LmDeleteAndNext(tmp,r);
  M1=nc_mm_Mult_p(m1,tmp,r);
  tmp=p_Copy(p2,r);
  tmp=p_LmDeleteAndNext(tmp,r);
  M2=p_Add_q(M2,M1,r);
  M1=nc_mm_Mult_p(m2,tmp,r);
  M2=p_Add_q(M2,M1,r);
  p_Delete(&m1,r);
  p_Delete(&m2,r);
  //  n_Delete(&C1,r);
  //  n_Delete(&C2,r);
  n_Delete(&MinusOne,r);
  return(M2);
}

/*5
* reduction of tail(q) with p1
* lead(p1) divides lead(pNext(q2)) and pNext(q2) is reduced
* do not destroy p1, but tail(q)
*/
void nc_spGSpolyRedTail(poly p1, poly q, poly q2, poly spNoether, const ring r)
{
  poly a1=p_Head(p1,r);
  poly Q=pNext(q2);
  number cQ=p_GetCoeff(Q,r);
  poly m=pOne();  
  p_ExpVectorDiff(m,Q,p1,r);
  p_Setm(m,r); 
  p_Test(m,r);
  /* pSetComp(m,r)=0? */
  poly M=nc_mm_Mult_p(m,p_Copy(p1,r),r);
  number C=p_GetCoeff(M,r);
  M=p_Add_q(M,nc_mm_Mult_p(m,p_LmDeleteAndNext(p_Copy(p1,r),r),r),r);
  q=p_Mult_nn(q,C,r);
  number MinusOne=n_Init(-1,r);
  if (!n_Equal(cQ,MinusOne,r))
  {
    cQ=nNeg(cQ);
    M=p_Mult_nn(M,cQ,r);    
  }
  Q=p_Add_q(Q,M,r);
  pNext(q2)=Q;

  p_Delete(&m,r);
  n_Delete(&C,r);
  n_Delete(&cQ,r);
  n_Delete(&MinusOne,r);
  /*  return(q); */
}

/*6
* creates the commutative lcm(lm(p1),lm(p2))
* do not destroy p1 and p2
*/
poly nc_spShort(poly p1, poly p2, const ring r)
{
  if (p_GetComp(p1,r)!=p_GetComp(p2,r))
  {
    Print("spShort:exponent mismatch!");
    return(NULL);
  }
  poly m=pOne();
  pLcm(p1,p2,m);
  p_Setm(m,r);
  return(m);
}

void nc_kBucketPolyRed(kBucket_pt b, poly p)
{
  poly m=pOne();
  pExpVectorDiff(m,kBucketGetLm(b),p);
  pSetm(m);
  pTest(m);
  p=nc_mm_Mult_p(m,pCopy(p),currRing);
  pDelete(&m);
  number n=pGetCoeff(p);
  number MinusOne=nInit(-1);
  if (!nEqual(n,MinusOne))
  {
    n=nNeg(nInvers(n));    
  }
  n=nMult(n,pGetCoeff(kBucketGetLm(b)));
  p=p_Mult_nn(p,n,currRing);
  nDelete(&n);
  nDelete(&MinusOne);
  int l=pLength(p);
  kBucket_Add_q(b,p,&l);
}

poly nc_p_Bracket_qq(poly p, poly q)
  /* returns [p,q], destroys p */
{
  if (!rIsPluralRing(currRing)) return(NULL);
  if (pComparePolys(p,q)) return(NULL);
  /* Components !? */
  poly Q=NULL;
  number coef=NULL;
  poly res=NULL;
  poly pres=NULL;
  while (p!=NULL)
  {
    Q=q;
    while(Q!=NULL)
    {
      pres=nc_mm_Bracket_nn(p,Q); /* since no coeffs are taken into account there */
      if (pres!=NULL) 
      {
	coef=nMult(pGetCoeff(p),pGetCoeff(Q));
	if (!nIsOne(coef)) pres=p_Mult_nn(pres,coef,currRing);
	res=p_Add_q(res,pres,currRing);
        nDelete(&coef);
      }
      pIter(Q);
    }
    p=pLmDeleteAndNext(p);
  }
  return(res);
}

poly nc_mm_Bracket_nn(poly m1, poly m2)
  /*returns [m1,m2] for two monoms, destroys nothing */
  /* without coeffs */
{
  if (pLmIsConstant(m1) || pLmIsConstant(m1)) return(NULL);
  if (pLmCmp(m1,m2)==0) return(NULL);
  Exponent_t *M1=(Exponent_t *)omAlloc0((currRing->N+1)*sizeof(Exponent_t));
  Exponent_t *M2=(Exponent_t *)omAlloc0((currRing->N+1)*sizeof(Exponent_t));
  Exponent_t *PREFIX=(Exponent_t *)omAlloc0((currRing->N+1)*sizeof(Exponent_t));
  Exponent_t *SUFFIX=(Exponent_t *)omAlloc0((currRing->N+1)*sizeof(Exponent_t));
  pGetExpV(m1,M1);
  pGetExpV(m2,M2);
  poly res=NULL;
  poly ares=NULL;
  poly bres=NULL;
  poly tmp=pOne();
  poly prefix=pOne();
  poly suffix=pOne();
  int nMin,nMax;
  number nTmp=NULL;
  number MinusOne=nInit(-1);
  int i,j,k;
  for (i=1;i<=currRing->N;i++)
  {
    if (M2[i]!=0)
    {
      ares=NULL;
      for (j=1;j<=currRing->N;j++)
      {
	if (M1[j]!=0) 
	{
	  bres=NULL;
	  /* compute [ x_j^M1[j],x_i^M2[i] ] */
	  if (i<j) {nMax=j;  nMin=i;} else {nMax=i;  nMin=j;}
	  if ( (i==j) || ((MATELEM(currRing->nc->COM,nMin,nMax)!=NULL) && nIsOne(pGetCoeff(MATELEM(currRing->nc->C,nMin,nMax))) )) /* not (the same exp. or commuting exps)*/
	  { bres=NULL; }
	  else
	  {
	    if (i<j) { bres=nc_uu_Mult_ww(j,M1[j],i,M2[i],currRing); }
	    else bres=nc_uu_Mult_ww(i,M2[i],j,M1[j],currRing);
	    if (nIsOne(pGetCoeff(bres)))
	    {
	      bres=pLmDeleteAndNext(bres);
	    }
	    else
	    {
	      nTmp=nInit(1);
	      nTmp=nSub(pGetCoeff(bres),nTmp);
	      pSetCoeff(bres,nTmp); /* only lc ! */
	    }
	    pTest(bres);
	    if (i>j)  bres=p_Mult_nn(bres, MinusOne,currRing);
	  }
	  if (bres!=NULL)
	  {
	    /* now mult (prefix, bres, suffix) */
	    memcpy(SUFFIX, M1,(currRing->N+1)*sizeof(Exponent_t));
	    memcpy(PREFIX, M1,(currRing->N+1)*sizeof(Exponent_t));
	    for (k=1;k<=j;k++) SUFFIX[k]=0;
	    for (k=j;k<=currRing->N;k++) PREFIX[k]=0;
	    SUFFIX[0]=0;
	    PREFIX[0]=0;
	    prefix=pOne();
	    suffix=pOne();
	    pSetExpV(prefix,PREFIX);
	    pSetm(prefix);
	    pSetExpV(suffix,SUFFIX);
	    pSetm(suffix);
	    if (!pLmIsConstant(prefix)) bres = nc_mm_Mult_p(prefix, bres,currRing);
	    if (!pLmIsConstant(suffix)) bres = nc_p_Mult_mm(bres, suffix,currRing);
	    ares=p_Add_q(ares, bres,currRing);
	    /* What to give free? */
	    pDelete(&prefix);
	    pDelete(&suffix);
	  }
	}
      }
      if (ares!=NULL)
      {
	/* now mult (prefix, bres, suffix) */
	memcpy(SUFFIX, M2,(currRing->N+1)*sizeof(Exponent_t));
	memcpy(PREFIX, M2,(currRing->N+1)*sizeof(Exponent_t));
	for (k=1;k<=i;k++) SUFFIX[k]=0;
	for (k=i;k<=currRing->N;k++) PREFIX[k]=0;
	SUFFIX[0]=0;
	PREFIX[0]=0;
	prefix=pOne();
	suffix=pOne();
	pSetExpV(prefix,PREFIX);
	pSetm(prefix);
	pSetExpV(suffix,SUFFIX);
	pSetm(suffix);
	bres=ares;
	if (!pLmIsConstant(prefix)) bres = nc_mm_Mult_p(prefix, bres,currRing);
	if (!pLmIsConstant(suffix)) bres = nc_p_Mult_mm(bres, suffix,currRing);
	res=p_Add_q(res, bres,currRing);
	pDelete(&prefix);
	pDelete(&suffix);
      }
    }
  }
  freeT(M1,currRing->N);
  freeT(M2,currRing->N);
  freeT(PREFIX, currRing->N);
  freeT(SUFFIX, currRing->N);
  return(res);
}

#endif
