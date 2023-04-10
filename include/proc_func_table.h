#ifndef PROC_FUNC_TAB
#define PROC_FUNC_TAB

#include "msg_def.h"
// class MsgPacket;

#define MAX_FUNC 128

enum FUNCTYPE
{
    ef_None = 0,
    ef_AddFunc = 1,
};

struct D1
{
    int a;
};

typedef union fdata
{
    D1 d1;
} FDATA;

// type cast
// typedef void(*ProcFunc)(MsgPacket*);
// using ProcFunc = void (*)(void *);

// use union
using ProcFunc = void (*)(FDATA *);

/* use template
template <typename T>
using AddFunc = void (*)(T *);
*/

void NoneFun(FDATA *d);
void AddFun(FDATA *d);

const ProcFunc f_map[MAX_FUNC] = {
    [FUNCTYPE::ef_None] = NoneFun,
    [FUNCTYPE::ef_AddFunc] = AddFun,
};

#endif