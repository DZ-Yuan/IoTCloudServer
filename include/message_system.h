#ifndef MESSAGE_SYS
#define MESSAGE_SYS

#include <queue>
#include "msg_def.h"
#include "_thread.h"

class MServer;
class JobsSystem;

enum class MESSAGESYSTEMSTATUS
{
    emsg_Err = -1,
    emsg_None = 0,
    emsg_Running = 1,
    emsg_Terminated = 2,
};

class MessageSystem : public _Thread
{
public:
    MessageSystem(MServer *server, JobsSystem *);
    ~MessageSystem();

public:
    void run() { OnMsgProc(); }

    // 处理
    void OnMsgProc();
    void Dispatch(){};
    // msg push
    void PushMsg(MsgPacket *m);
    // msg pop
    MsgPacket *PopMsg();

    void Terminate() { status_ = false; }
    // clear
    void Clear();

public:
private:
    //
    bool status_;
    // 优先队列
    // 消息队列
    std::queue<MsgPacket *> msg_queue_;
    JobsSystem *task_sys_;

    MServer *server_;
};

#endif