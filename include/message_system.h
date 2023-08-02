#ifndef MESSAGE_SYS
#define MESSAGE_SYS

#include <queue>
#include "msg_def.h"
#include "m_thread.h"
#include "node_system.h"
#include "interface_class.h"

class MServer;
class JobSystem;

enum class MESSAGE_SYSTEM_STATUS
{
    emsg_Err = -1,
    emsg_None = 0,
    emsg_Running = 1,
    emsg_Terminated = 2,
};

class MessageSystem : public MThread, SystemBase_Interface
{
public:
    MessageSystem(MServer *server, JobSystem *);
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

    virtual void SetStatus(int s) { status_ = s ? true : false; }
    virtual int GetStatus() { return status_ ? 1 : 0; }
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
    // should I change the var name?
    JobSystem *task_sys_;

    MServer *server_;
};

#endif