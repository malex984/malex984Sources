/*****************************************
*  Computer Algebra System SINGULAR      *
*****************************************/
/* $Id: eigenval.cc,v 1.11 2002-02-20 17:32:12 mschulze Exp $ */
/*
* ABSTRACT: eigenvalues of constant square matrices
*/

#include "mod2.h"

#ifdef HAVE_EIGENVAL

#include "febase.h"
#include "tok.h"
#include "ipid.h"
#include "intvec.h"
#include "numbers.h"
#include "polys.h"
#include "ideals.h"
#include "lists.h"
#include "matpol.h"
#include "clapsing.h"
#include "eigenval.h"


matrix evSwap(matrix M,int i,int j)
{
  if(i==j)
    return(M);

  for(int k=1;k<=MATROWS(M);k++)
  {
    poly p=MATELEM(M,i,k);
    MATELEM(M,i,k)=MATELEM(M,j,k);
    MATELEM(M,j,k)=p;
  }

  for(int k=1;k<=MATCOLS(M);k++)
  {
    poly p=MATELEM(M,k,i);
    MATELEM(M,k,i)=MATELEM(M,k,j);
    MATELEM(M,k,j)=p;
  }

  return(M);
}


BOOLEAN evSwap(leftv res,leftv h)
{
  if(currRingHdl)
  {
    if(h&&h->Typ()==MATRIX_CMD)
    {
      matrix M=(matrix)h->Data();
      h=h->next;
      if(h&&h->Typ()==INT_CMD)
      {
        int i=(int)h->Data();
        h=h->next;
        if(h&&h->Typ()==INT_CMD)
        {
          int j=(int)h->Data();
          res->rtyp=MATRIX_CMD;
          res->data=(void *)evSwap(mpCopy(M),i,j);
          return FALSE;
        }
      }
    }
    WerrorS("<matrix>,<int>,<int> expected");
    return TRUE;
  }
  WerrorS("no ring active");
  return TRUE;
}


matrix evRowElim(matrix M,int i,int j,int k)
{
  if(MATELEM(M,i,k)==NULL||MATELEM(M,j,k)==NULL)
    return(M);

  poly p=pNSet(nDiv(pGetCoeff(MATELEM(M,i,k)),pGetCoeff(MATELEM(M,j,k))));
  pNormalize(p);

  for(int l=1;l<=MATCOLS(M);l++)
  {
    MATELEM(M,i,l)=pSub(MATELEM(M,i,l),ppMult_qq(p,MATELEM(M,j,l)));
    pNormalize(MATELEM(M,i,l));
  }
  for(int l=1;l<=MATROWS(M);l++)
  {
    MATELEM(M,l,j)=pAdd(MATELEM(M,l,j),ppMult_qq(p,MATELEM(M,l,i)));
    pNormalize(MATELEM(M,l,j));
  }

  pDelete(&p);

  return(M);
}


BOOLEAN evRowElim(leftv res,leftv h)
{
  if(currRingHdl)
  {
    if(h&&h->Typ()==MATRIX_CMD)
    {
      matrix M=(matrix)h->Data();
      h=h->next;
      if(h&&h->Typ()==INT_CMD)
      {
        int i=(int)h->Data();
        h=h->next;
        if(h&&h->Typ()==INT_CMD)
        {
          int j=(int)h->Data();
          h=h->next;
          if(h&&h->Typ()==INT_CMD)
          {
            int k=(int)h->Data();
            res->rtyp=MATRIX_CMD;
            res->data=(void *)evRowElim(mpCopy(M),i,j,k);
            return FALSE;
	  }
        }
      }
    }
    WerrorS("<matrix>,<int>,<int>,<int> expected");
    return TRUE;
  }
  WerrorS("no ring active");
  return TRUE;
}


matrix evColElim(matrix M,int i,int j,int k)
{
  if(MATELEM(M,k,i)==0||MATELEM(M,k,j)==0)
    return(M);

  poly p=pNSet(nDiv(pGetCoeff(MATELEM(M,k,i)),pGetCoeff(MATELEM(M,k,j))));
  pNormalize(p);

  for(int l=1;l<=MATROWS(M);l++)
  {
    MATELEM(M,l,i)=pSub(MATELEM(M,l,i),ppMult_qq(p,MATELEM(M,l,j)));
    pNormalize(MATELEM(M,l,i));
  }
  for(int l=1;l<=MATCOLS(M);l++)
  {
    MATELEM(M,j,l)=pAdd(MATELEM(M,j,l),ppMult_qq(p,MATELEM(M,i,l)));
    pNormalize(MATELEM(M,j,l));
  }

  pDelete(&p);

  return(M);
}


BOOLEAN evColElim(leftv res,leftv h)
{
  if(currRingHdl)
  {
    if(h&&h->Typ()==MATRIX_CMD)
    {
      matrix M=(matrix)h->Data();
      h=h->next;
      if(h&&h->Typ()==INT_CMD)
      {
        int i=(int)h->Data();
        h=h->next;
        if(h&&h->Typ()==INT_CMD)
        {
          int j=(int)h->Data();
          h=h->next;
          if(h&&h->Typ()==INT_CMD)
          {
            int k=(int)h->Data();
            res->rtyp=MATRIX_CMD;
            res->data=(void *)evColElim(mpCopy(M),i,j,k);
            return FALSE;
	  }
        }
      }
    }
    WerrorS("<matrix>,<int>,<int>,<int> expected");
    return TRUE;
  }
  WerrorS("no ring active");
  return TRUE;
}


matrix evHessenberg(matrix M)
{
  int n=MATROWS(M);

  for(int k=1,j=2;k<n-1;k++,j=k+1)
  {
    while(j<=n&&MATELEM(M,j,k)==0)
      j++;

    if(j<=n)
    {
      M=evSwap(M,j,k+1);

      for(int i=j+1;i<=n;i++)
        M=evRowElim(M,i,k+1,k);
    }
  }

  return(M);
}


BOOLEAN evHessenberg(leftv res,leftv h)
{
  if(currRingHdl)
  {
    if(h&&h->Typ()==MATRIX_CMD)
    {
      matrix M=(matrix)h->Data();
      res->rtyp=MATRIX_CMD;
      res->data=(void *)evHessenberg(mpCopy(M));
      return FALSE;
    }
    WerrorS("<matrix> expected");
    return TRUE;
  }
  WerrorS("no ring active");
  return TRUE;
}


lists evEigenvals(matrix M)
{
  lists l=(lists)omAllocBin(slists_bin);
  if(MATROWS(M)!=MATCOLS(M))
  {
    l->Init(0);
    return(l);
  }

  M=evHessenberg((matrix)idJet((ideal)M,0));

  int n=MATROWS(M);
  ideal e=idInit(n,1);
  intvec *m=new intvec(n);

  poly t=pOne();
  pSetExp(t,1,1);
  pSetm(t);

  for(int j0=1,j=2,k=0;j<=n+1;j0=j,j++)
  {
    while(j<=n&&MATELEM(M,j,j-1)!=NULL)
      j++;
    if(j==j0+1)
    {
      e->m[k]=pHead(MATELEM(M,j0,j0));
      (*m)[k]=1;
      k++;
    }
    else
    {
      int n0=j-j0;
      matrix M0=mpNew(n0,n0);

      j0--;
      for(int i=1;i<=n0;i++)
        for(int j=1;j<=n0;j++)
          MATELEM(M0,i,j)=pCopy(MATELEM(M,j0+i,j0+j));
      for(int i=1;i<=n0;i++)
        MATELEM(M0,i,i)=pSub(MATELEM(M0,i,i),pCopy(t));

      intvec *m0;
      ideal e0=singclap_factorize(mpDetBareiss(M0),&m0,2);

      for(int i=0;i<IDELEMS(e0);i++)
      {
        if(pNext(e0->m[i])==NULL)
	{
          (*m)[k]=(*m0)[i];
          k++;
        }
        else
        if(pGetExp(e0->m[i],1)<2&&pGetExp(pNext(e0->m[i]),1)<2&&
           pNext(pNext(e0->m[i]))==NULL)
	{
          number e1=nNeg(pGetCoeff(e0->m[i]));
          if(pGetExp(pNext(e0->m[i]),1)==0)
            e->m[k]=pNSet(nDiv(pGetCoeff(pNext(e0->m[i])),e1));
          else
	    e->m[k]=pNSet(nDiv(e1,pGetCoeff(pNext(e0->m[i]))));
          nDelete(&e1);
          pNormalize(e->m[k]);
          (*m)[k]=(*m0)[i];
          k++;
        }
        else
	{
          e->m[k]=e0->m[i];
          pNormalize(e->m[k]);
          e0->m[i]=NULL;
          (*m)[k]=(*m0)[i];
          k++;
	}
      }

      delete(m0);
      idDelete(&e0);
    }
  }

  pDelete(&t);
  idDelete((ideal *)&M);

  for(int i0=0;i0<n-1;i0++)
  {
    for(int i1=i0+1;i1<n;i1++)
    {
      if(pEqualPolys(e->m[i0],e->m[i1]))
      {
        (*m)[i0]+=(*m)[i1];
        (*m)[i1]=0;
      }
      else
      {
        if(e->m[i0]==NULL&&!nGreaterZero(pGetCoeff(e->m[i1]))||
           e->m[i1]==NULL&&
	  (nGreaterZero(pGetCoeff(e->m[i0]))||pNext(e->m[i0])!=NULL)||
           e->m[i0]!=NULL&&e->m[i1]!=NULL&&
          (pNext(e->m[i0])!=NULL&&pNext(e->m[i1])==NULL||
           pNext(e->m[i0])==NULL&&pNext(e->m[i1])==NULL&&
           nGreater(pGetCoeff(e->m[i0]),pGetCoeff(e->m[i1]))))
        {
          poly e1=e->m[i0];
          e->m[i0]=e->m[i1];
          e->m[i1]=e1;
          int m1=(*m)[i0];
          (*m)[i0]=(*m)[i1];
          (*m)[i1]=m1;
        }
      }
    }
  }

  int n0=0;
  for(int i=0;i<n;i++)
    if((*m)[i]>0)
      n0++;

  ideal e0=idInit(n0,1);
  intvec *m0=new intvec(n0);

  for(int i=0,i0=0;i<n;i++)
    if((*m)[i]>0)
    {
      e0->m[i0]=e->m[i];
      e->m[i]=NULL;
      (*m0)[i0]=(*m)[i];
      i0++;
    }

  idDelete(&e);
  delete(m);

  l->Init(2);
  l->m[0].rtyp=IDEAL_CMD;
  l->m[0].data=e0;
  l->m[1].rtyp=INTVEC_CMD;
  l->m[1].data=m0;

  return(l);
}


BOOLEAN evEigenvals(leftv res,leftv h)
{
  if(currRingHdl)
  {
    if(h&&h->Typ()==MATRIX_CMD)
    {
      matrix M=(matrix)h->Data();
      res->rtyp=LIST_CMD;
      res->data=(void *)evEigenvals(mpCopy(M));
      return FALSE;
    }
    WerrorS("<matrix> expected");
    return TRUE;
  }
  WerrorS("no ring active");
  return TRUE;
}

#endif /* HAVE_EIGENVAL */
