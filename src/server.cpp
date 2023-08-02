#include "pch.h"

MServer::MServer()
    : curr_state_(0), NetMsgRecvCallbackFunc{nullptr}
{
    network_sys_ = new NetworkSystem(this);
    jobs_sys_ = new JobSystem(this);
    msg_sys_ = new MessageSystem(this, jobs_sys_);
    node_sys_ = new NodeSystem(this, network_sys_);
}

MServer::~MServer()
{
    // SafeDelete(network_sys_);
    SafeDelete(msg_sys_);
    SafeDelete(jobs_sys_);
}

void MServer::Init()
{
}

void MServer::Launch()
{
    Init();
    // 设置服务器状态
    curr_state_ = sRunnig;
    SerPrint("Server Lanuch Successfully");
}

void MServer::Run()
{
    network_sys_->SetupConfig(SER_IP, SER_TCP_PORT, SER_UDP_PORT);
    network_sys_->Start();
    msg_sys_->start();
    jobs_sys_->start();
    node_sys_->start();

    while (1)
    {
        char buf[10];
        cin >> buf;

        if (strcmp(buf, "quit") == 0)
        {
            cout << "Stopping Server..." << endl;
            break;
        }

        usleep(1000);
        // test_case_msg();
        // thism_thread::sleep_for(1s);
    }

    Terminate();
}

void MServer::Terminate()
{
    curr_state_ = sTerminated;

    // 关闭其他系统
    network_sys_->Terminate();
    msg_sys_->Terminate();
    jobs_sys_->Terminate();

    // todo 等待子线程结束
    // thism_thread::sleep_for(10s);
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

void MServer::RegNetMsgRecvCallback(int sys_id, NetMsgHdl_Interface *p)
{
    if (!p || sys_id <= 0 || sys_id >= sidEnd)
        return;

    NetMsgRecvCallbackFunc[sys_id] = p;
}

int MServer::CallNetMsgRecvCallback(int sys_id, DataPacket *dp)
{
    if (sys_id <= sidNull || sys_id >= sidEnd)
    {
        return -1;
    }

    NetMsgHdl_Interface *p = NetMsgRecvCallbackFunc[sys_id];

    if (!p)
        return -1;

    p->ReceiveNetPacket_IF(dp);

    return 0;
}

void MServer::test_case_msg()
{
    MsgPacket *msg = new MsgPacket();

    msg->id_ = 1;
    msg->msg_type_ = em_Generaic;
    msg->ftype_ = ef_AddFunc;

    D1 *d = new D1();
    d->a = 10;

    msg->data_ = (char *)d;

    msg_sys_->PushMsg(msg);

    cout << "Create a message" << endl;
}
