#include "pch.h"

MServer::MServer() : curr_state_(0)
{
    network_sys_ = new NetworkSystem(this);
    msg_proc_ = new MessageSystem(this);
}

MServer::~MServer()
{
    SafeDelete(network_sys_);
    SafeDelete(msg_proc_);
}

void MServer::Init()
{
}

void MServer::Launch()
{
    Init();
    //设置服务器状态
    curr_state_ = sRunnig;
    SerPrint("Server Lanuch Successfully");
}

void MServer::Run()
{
    while (1)
    {
        char buf[10];
        cin >> buf;

        if (strcmp(buf, "quit") == 0)
        {
            cout << "Stopping Server..." << endl;
            break;
        }

        this_thread::sleep_for(1s);
    }

    Terminate();
}

void MServer::Terminate()
{
    curr_state_ = sTerminated;
    
    //关闭其他系统
    network_sys_->Terminate();
    msg_proc_->Clear();

    // todo 等待子线程结束
    this_thread::sleep_for(10s);
}

bool MServer::SendMsg(int sock, char *msg, int size)
{
    while (size > 0)
    {
        int ss = send(sock, msg, size, 0);
        if (ss <= -1)
        {
            return false;
        }

        size -= ss;
        msg += ss;
    }

    return true;
}

void MServer::PostMsg()
{
}