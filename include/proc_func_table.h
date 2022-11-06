#ifndef PROC_FUNC_TAB
#define PROC_FUNC_TAB

#include "msg_def.h"
// class MsgPacket;

#define MAX_FUNC 128

// typedef void(*ProcFunc)(MsgPacket*);
using ProcFunc = void(*)(MsgPacket*);

// void testFunc(MsgPacket *m){};

#endif