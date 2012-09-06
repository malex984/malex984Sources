"ndebug?: ", system("with", "ndebug");
"om_ndebug?: ", system("with", "om_ndebug");

listvar(Top);

if( system("with", "ndebug") )
{
  "Loading the Release version!";
  LIB("syzextra.so");
  listvar(Syzextra);
}
else
{
  "Loading the Debug version!";
  LIB("syzextra_g.so");
  listvar(Syzextra_g);
}


noop();

ring r;

DetailedPrint(r);

poly p = x +y +z;

DetailedPrint(p, 2);

ideal I = maxideal(1);

DetailedPrint(I, 2);

vector v =3 *xyz * [0, 0, 1, x, y, z];
v;
DetailedPrint(v, 2);

leadmonom(v);leadcomp(v);
leadrawexp(v);




def l = res(I, 0); DetailedPrint(l);




ring R = 0, (x), dp;

// // clearcontent:

proc TestClearContent(def i, number c, def o)
{
  "";   
  "Test: ClearContent(", i, " --?-> ", o, " / => ", c, "): ";
  int pass = 1;
  number @c = ClearContent(i);

  if( leadcoef(i) < 0 )
  {
    "ERROR: negative leading coeff. after clearing  content: ", leadcoef(i), " instead of ", leadcoef(o);
    pass = 0;
  }
  if( @c != c )
  {
    "ERROR: wrong content: ", @c, " instead of ", c;
    pass = 0;
  }  
  if( i != o )
  {
    "ERROR: wrong element after clearing content: ", i, " instead of ", o;
    pass = 0;
  }

  if( pass )
  {
    "[TestClearContent -- PASSED]";
  } else
  {
    ERROR("[TestClearContent -- FAILED]");
  }
  ""; 
}


proc TestClearDenominators(def i, number c, def o)
{
  def ii = cleardenom(i);
  "";   
  "Test: ClearDenominators(", i, " --?-> ", o, " / => ", c, "): ";
  int pass = 1;
  number @c = ClearDenominators(i);
  if( leadcoef(i) < 0 )
  {
    "ERROR: negative leading coeff. after clearing denominators: ", leadcoef(i), " instead of ", leadcoef(o);
    pass = 0;
  }
  if( @c != c )
  {
    "ERROR: wrong multiplier: ", @c, " instead of ", c;
    pass = 0;
  }
  if( i != o )
  {
    "ERROR: wrong element after clearing denominators: ", i, " instead of ", o;
    pass = 0;
  }

  number cntnt = ClearContent(i); // cleardenom seems to run clearcontent on its own...
  if( i != ii )
  {    
    "ERROR: result of clearing denominators: ", i, " is inconsistent with cleardenom(): ", ii;
    pass = 0;
  }


  if( pass )
  {
    "[TestClearDenominators -- PASSED]";
  } else
  {
    ERROR("[TestClearDenominators -- FAILED]");
  }
  ""; 
}



// with polynomials in 'x'
TestClearContent(poly(1), number(1), poly(1)); // {1} -> {1}, c=1
TestClearContent(poly(2), number(2), poly(1)); // {2} -> {1}, c=2
TestClearContent(poly(222222222222x + 2), number(2), poly(111111111111x + 1)); // {222222222222, 2 } -> { 111111111111, 1} c=2
TestClearContent(poly(2x + 222222222222), number(2), poly(1x + 111111111111)); // {2, 222222222222 } -> { 1, 111111111111} c=2

// use vector instead:
TestClearContent(vector(1), number(1), vector(1)); // {1} -> {1}, c=1
TestClearContent(vector(2), number(2), vector(1)); // {2} -> {1}, c=2
TestClearContent(vector([222222222222, 2]), number(2), vector([111111111111, 1])); // {222222222222, 2 } -> { 111111111111, 1} c=2
TestClearContent(vector([2, 222222222222]), number(2), vector([1, 111111111111])); // {2, 222222222222 } -> { 1, 111111111111} c=2


// with negative leading coeff!
TestClearContent(-poly(1), -number(1), poly(1)); // {1} -> {1}, c=1
TestClearContent(-poly(2), -number(2), poly(1)); // {2} -> {1}, c=2
TestClearContent(-poly(222222222222x + 2), -number(2), poly(111111111111x + 1)); // {222222222222, 2 } -> { 111111111111, 1} c=2
TestClearContent(-poly(2x + 222222222222), -number(2), poly(1x + 111111111111)); // {2, 222222222222 } -> { 1, 111111111111} c=2

// use vector instead:
TestClearContent(-vector(1), -number(1), vector(1)); // {1} -> {1}, c=1
TestClearContent(-vector(2), -number(2), vector(1)); // {2} -> {1}, c=2
TestClearContent(-vector([222222222222, 2]), -number(2), vector([111111111111, 1])); // {222222222222, 2 } -> { 111111111111, 1} c=2
TestClearContent(-vector([2, 222222222222]), -number(2), vector([1, 111111111111])); // {2, 222222222222 } -> { 1, 111111111111} c=2




TestClearDenominators(poly(1), number(1), poly(1)); // {1} -> {1}, c=1
TestClearDenominators(poly(2), number(1), poly(2)); // {2} -> {2}, c=1
TestClearDenominators(poly(x + (1/2)), number(2), poly(2x + 1)); // {1, 1/2 } -> {2, 1}, c=2
TestClearDenominators(poly(1/2x + 1), number(2), poly(x + 2)); // {1/2, 1} -> {1, 2}, c=2
TestClearDenominators(poly(1/3x3+1/4x2+1/6x+1), number(12), poly(4x3+3x2+2x+12)); // {1/3, 1/4, 1/6, 1 } -> {4, 3, 2, 12}, c=12
TestClearDenominators(poly(1/2x3+1/4x2+3/2x+111111111111), number(4), poly(2x3+1x2+6x+444444444444)); // {1/2, 1/4, 3/2, 111111111111 } -> {2, 1, 6, 444444444444} , c=4



TestClearDenominators(vector([1]), number(1), vector([1])); // {1} -> {1}, c=1
TestClearDenominators(vector([2]), number(1), vector([2])); // {2} -> {2}, c=1
TestClearDenominators(vector([1, 1/2]), number(2), vector([2, 1])); // {1, 1/2} -> {2, 1}, c=2
TestClearDenominators(vector([1/2, 1]), number(2), vector([1, 2])); // {1/2, 1} -> {1, 2}, c=2
TestClearDenominators(vector([1/3,1/4,1/6,1]), number(12), vector([4,3,2,12])); // {1/3, 1/4, 1/6, 1 } -> {4, 3, 2, 12}, c=12
TestClearDenominators(vector([1/2,1/4,3/2,111111111111]), number(4), vector([2,1,6,444444444444])); // {1/2, 1/4, 3/2, 111111111111 } -> {2, 1, 6, 444444444444} , c=4


TestClearDenominators(-poly(1), -number(1), poly(1)); // {1} -> {1}, c=1
TestClearDenominators(-poly(2), -number(1), poly(2)); // {2} -> {2}, c=1
TestClearDenominators(-poly(x + (1/2)), -number(2), poly(2x + 1)); // {1, 1/2 } -> {2, 1}, c=2
TestClearDenominators(-poly(1/2x + 1), -number(2), poly(x + 2)); // {1/2, 1} -> {1, 2}, c=2
TestClearDenominators(-poly(1/3x3+1/4x2+1/6x+1), -number(12), poly(4x3+3x2+2x+12)); // {1/3, 1/4, 1/6, 1 } -> {4, 3, 2, 12}, c=12
TestClearDenominators(-poly(1/2x3+1/4x2+3/2x+111111111111), -number(4), poly(2x3+1x2+6x+444444444444)); // {1/2, 1/4, 3/2, 111111111111 } -> {2, 1, 6, 444444444444} , c=4



TestClearDenominators(-vector([1]), -number(1), vector([1])); // {1} -> {1}, c=1
TestClearDenominators(-vector([2]), -number(1), vector([2])); // {2} -> {2}, c=1
TestClearDenominators(-vector([1, 1/2]), -number(2), vector([2, 1])); // {1, 1/2} -> {2, 1}, c=2
TestClearDenominators(-vector([1/2, 1]), -number(2), vector([1, 2])); // {1/2, 1} -> {1, 2}, c=2
TestClearDenominators(-vector([1/3,1/4,1/6,1]), -number(12), vector([4,3,2,12])); // {1/3, 1/4, 1/6, 1 } -> {4, 3, 2, 12}, c=12
TestClearDenominators(-vector([1/2,1/4,3/2,111111111111]), -number(4), vector([2,1,6,444444444444])); // {1/2, 1/4, 3/2, 111111111111 } -> {2, 1, 6, 444444444444} , c=4



ring R = 0, (x, y, z), dp;

TestClearContent(poly(9x2y2z-18xyz2-18xyz+18z2+18z), number(9), poly(x2y2z-2xyz2-2xyz+2z2+2z)); // I[6]: Manual/Generalized_Hilbert_Syzygy_Theorem.tst
TestClearContent(-poly(9x2y2z-18xyz2-18xyz+18z2+18z), -number(9), poly(x2y2z-2xyz2-2xyz+2z2+2z)); // -_


TestClearContent(poly(4x3+2xy3), number(2), poly(2x3+xy3)); // j[1]: Old/err3.tst
TestClearContent(-poly(4x3+2xy3), -number(2), poly(2x3+xy3)); // j[1]: Old/err3.tst

TestClearContent(poly(2xy), number(2), poly(xy)); // _[2]: Manual/Delta.tst	
TestClearContent(poly(6x2z+2y2z), number(2), poly(3x2z+y2z)); // _[3]: Manual/Delta.tst	




ring r=0,(x,y),dp;
TestClearDenominators(poly(1/2x2 + 1/3y), number(6), poly(6*(1/2x2 + 1/3y)));


poly g = 1/2x2 + 1/3y; cleardenom(g);





// 
// [(1/(2t)), 1] -> [1, (2t)], (2t)
// trans. [((1/3)/(2t)), 1] -> [(1/3), (2t)], (2t) ???
// alg:  [((1/3)/(2t)), 1] ->[1, (6t)], (6t) 




/*
< _[1]=8x4l*gen(3)-16x3l2*gen(1)+8x2l3*gen(2)-8x*gen(2)+8y*gen(1)-8z*gen(3)
< _[2]=8x5*gen(3)-16x4l*gen(1)+8x3l2*gen(2)+8y*gen(2)-8z*gen(1)
---
> _[1]=33554432x4l*gen(3)-67108864x3l2*gen(1)+33554432x2l3*gen(2)-33554432x*gen(2)+33554432y*gen(1)-33554432z*gen(3)
> _[2]=33554432x5*gen(3)-67108864x4l*gen(1)+33554432x3l2*gen(2)+33554432y*gen(2)-33554432z*gen(1)


< j[3]=11x4y2+9x5+9x7
---
> j[3]=55x4y2+45x5+45x7



< qsat[1]=12zt+3z-10
< qsat[2]=5z2+12xt+3x
< qsat[3]=144xt2+72xt+9x+50z
---
> qsat[1]=21600zt+5400z-18000
> qsat[2]=9000z2+21600xt+5400x
> qsat[3]=259200xt2+129600xt+16200x+90000z




< q[1]=12zty+3zy-10y2
< q[2]=60z2t-36xty-9xy-50zy
< q[3]=12xty2+5z2y+3xy2
< q[4]=z3y+2xy3
---
> q[1]=216zty+54zy-180y2
> q[2]=1080z2t-648xty-162xy-900zy
> q[3]=648xty2+270z2y+162xy2
> q[4]=270z3y+540xy3



< qsat[1]=12zt+3z-10y
< qsat[2]=12xty+5z2+3xy
< qsat[3]=144xt2+72xt+9x+50z
< qsat[4]=z3+2xy2
---
> qsat[1]=23328000zt+5832000z-19440000y
> qsat[2]=233280000xty+97200000z2+58320000xy
> qsat[3]=279936000xt2+139968000xt+17496000x+97200000z
> qsat[4]=97200000z3+194400000xy2



*/


/*
experiments master Singular (together with Claus / exts. of Q):

> ring R = (0,t), x, dp;
> cleardenom ( [1/(2t), 1] );
(2t)*gen(2)+gen(1)
> cleardenom ( [(1/3)/(2t), 1] );
(6t)*gen(2)+gen(1)
> number a = 1/3;
> a;
1/3
> poly p = (a / (2t)) * x + 1;
> p;
1/(6t)*x+1
> poly p = ((a + t) / (2t+1)) * x + 1;
// ** redefining p **
> p;
(3t+1)/(6t+3)*x+1
> ^Z



 ring R = (0,t), x, dp;cleardenom ( [(1/3)/(2t), 1] );
(6t)*gen(2)+gen(1)
> ring R = (0,t), x, dp; minpoly = t2 + 1; cleardenom ( [(1/3)/(2t), 1] );
6*gen(2)+(-t)*gen(1)
>  [(1/3)/(2t), 1];
gen(2)+(-1/6t)*gen(1)



*/

$$$

