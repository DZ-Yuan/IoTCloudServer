#ifndef MESSAGE_SYS
#define MESSAGE_SYS

#include <queue>
#include "msg_def.h"

class MServer;

class MessageSystem
{
public:
    MessageSystem(MServer* server);
    ~MessageSystem();

public:
    //处理
    void OnMsgProc();
    void Dispatch(){};
    // msg push
    void PushMsg(MsgPacket *m);
    // msg pop
    MsgPacket *PopMsg();
    // todo post msg
    // clear
    void Clear();

public:
private:
    //
    bool status_;
    //优先队列
    //消息队列
    std::queue<MsgPacket *> msg_queue_;

    MServer* server_;
};

#endif