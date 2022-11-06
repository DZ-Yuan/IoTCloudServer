#include "../include/pch.h"

MessageSystem::MessageSystem(MServer *server)
    : server_(server), status_(true)
{
}

MessageSystem::~MessageSystem()
{
}

void MessageSystem::OnMsgProc()
{
    while (status_)
    {
    }
}

void MessageSystem::PushMsg(MsgPacket *m)
{
    msg_queue_.push(m);
}

MsgPacket *MessageSystem::PopMsg()
{
    MsgPacket *m = msg_queue_.front();
    msg_queue_.pop();
    return m;
}

void MessageSystem::Clear()
{
    while (!msg_queue_.empty())
    {
        delete msg_queue_.front();
        msg_queue_.pop();
    }
}