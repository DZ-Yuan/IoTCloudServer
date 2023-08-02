#include "pch.h"

MessageSystem::MessageSystem(MServer *server, JobSystem *task_sys)
    : server_(server), status_(true), task_sys_(task_sys)
{
}

MessageSystem::~MessageSystem()
{
    task_sys_ = nullptr;
    // Clear();
    cout << "Clean Message system..." << endl;
}

void MessageSystem::OnMsgProc()
{
    cout << "Running Message system..." << endl;

    while (status_)
    {
        MsgPacket *m = PopMsg();

        if (!m)
            continue;

        // deliver
        task_sys_->PushMsg(m);
    }

    cout << "Terminate Message system..." << endl;
    Clear();
}

void MessageSystem::PushMsg(MsgPacket *m)
{
    if (!m)
    {
        return;
    }

    msg_queue_.push(m);
}

MsgPacket *MessageSystem::PopMsg()
{
    MsgPacket *m = msg_queue_.front();

    if (!m)
    {
        return nullptr;
    }

    msg_queue_.pop();
    return m;
}

void MessageSystem::Clear()
{
    while (!msg_queue_.empty())
    {
        MsgPacket *m = msg_queue_.front();

        if (!m)
            continue;

        msg_queue_.pop();
        SafeDelete(m);
    }
}
