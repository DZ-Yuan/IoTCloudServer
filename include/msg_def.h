#ifndef MSG_DEF
#define MSG_DEF

struct MsgPacket;

#include "common.h"
// #include "proc_func_table.h"

// typedef void (*MsgProc)();
// using ProcFunc = void(*)(MsgPacket* m); declare in proc_func_table.h

enum MSGTYPE
{
    em_None = 0,
    em_Custom = 1,
    em_Generaic = 2,
};

struct MsgPacket
{
    int id_;
    int msg_type_;
    int ftype_;
    // void* cb_;
    void *data_;

    MsgPacket() : id_(0), data_(nullptr), ftype_(0)
    {
    }

    ~MsgPacket()
    {
        SafeDelete(data_);
    }
};

#endif
