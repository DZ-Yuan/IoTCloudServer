#ifndef MSG_DEF
#define MSG_DEF

struct MsgPacket;

#include "proc_func_table.h"

// typedef void (*MsgProc)();
//using ProcFunc = void(*)(MsgPacket* m); declare in proc_func_table.h

enum MSGTYPE
{
    mNone = 0,
    mCustom = 1,
};

struct MsgPacket
{
    int id_;
    void *data_;

    MsgPacket() : id_(0), data_(nullptr)
    {
    }
};



#endif
