#ifndef IPCONVERT_H
#define IPCONVERT_H
/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/*
* ABSTRACT: interpreter: converting types
*/
#include <kernel/structs.h>

int iiTestConvert (int inputType, int outputType);
BOOLEAN iiConvert (int inputType, int outputType, int index,
                   leftv input, leftv output);
#endif

