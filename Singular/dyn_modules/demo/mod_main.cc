#include <kernel/mod2.h>

#include <reporter/reporter.h>
#include <coeffs/coeffs.h>

#include <kernel/polys.h>

#include <Singular/mod_lib.h>
#include <Singular/ipid.h>
#include <Singular/tok.h>


/// sum of squares from \ref Singular/extra.cc
static BOOLEAN _myAdd(leftv res, leftv h)
{
  PrintS("inside _myAdd...\n");
   
  res->rtyp = NONE;
  res->data = NULL;

  const char *usage = "'my_sq_poly_add' (_myAdd) needs two polynomial arguments...";

  if( h == NULL )
  {
    WarnS(usage);
    return TRUE;
  }

  assume( h != NULL );

  // 1st argument: of type polynomial
  if( !( h->Typ() == POLY_CMD ) )
  {
    WarnS(usage);
    return TRUE;
  }
  const poly p1 = static_cast<poly>(h->Data());   

  // try to get the next (2nd) argument
  if( h->next == NULL )
  {
    WarnS(usage);
    return TRUE;
  }

  h = h->next; assume( h != NULL );

  // 2nd argument: of type polynomial
  if( !( h->Typ() == POLY_CMD ) )
  {
    WarnS(usage);
    return TRUE;
  }   
  const poly p2 = static_cast<poly>(h->Data());
   
  // no further arguments to this call
  assume( h->next == NULL );

  // result should be also polynomial: sum of squares of inputs
  res->data=(void *)pAdd(ppMult_qq(p1, p1), ppMult_qq(p2, p2));
  res->rtyp=POLY_CMD; 

  return FALSE;
}

/// Standard module initialization
extern "C" int SI_MOD_INIT(demo)(SModulFunctions* psModulFunctions)
{
  PrintS("initializing demo module\n");
   
#define ADD(C,D,E) \
  psModulFunctions->iiAddCproc((currPack->libname? currPack->libname: ""), (char*)C, D, E);

  // register _myAdd under the name `myadd_dm` in the current module namespace (Demo) upon loading this module into Singular interpreter
  ADD("my_sq_poly_add", FALSE, _myAdd);
//  ADD("myadd", FALSE, _myAdd); // This is not allowed if `myadd` has been made a reserved name for the interpreter (`cmdnames cmds[]` in Singular/table)
   
  return (MAX_TOK); // something like version of Singular interporeter
}

