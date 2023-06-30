#include "pch.h"

JobSystem::JobSystem(MServer *ser)
    : server_(ser), worker_num_(0), status_(ejob_None)
{
}

JobSystem::~JobSystem()
{
    // clean worker_list
    for (int i = 0; i < WORKER_MAX; i++)
    {
        WorkerStat &ws = worker_list_[i];
        if (ws.worker_)
        {
            ws.worker_->Retire();
            delete ws.worker_;
            ws.worker_ = nullptr;
            ws.status_ = ewk_Retire;
        }
    }

    cout << "Clean Jobs system..." << endl;
}

void JobSystem::run()
{
    status_ = ejob_Running;
    cout << "Running Job system..." << endl;
    AddWorker();

    while (status_ != ejob_Terminated)
    {
        Worker *w = GetIdleWorker();

        if (!w)
            continue;

        MsgPacket *m = PopMsg();

        if (!m)
            continue;

        w->AcceptJob(m);
    }

    cout << "Terminate Job system..." << endl;
    Clear();
}

Worker *JobSystem::GetIdleWorker()
{
    for (size_t i = 0; i < WORKER_MAX; i++)
    {
        WorkerStat &ws = worker_list_[i];

        if (ws.worker_ && ws.status_ == ewk_Wating)
        {
            return ws.worker_;
        }
    }

    return nullptr;
}

void JobSystem::RegisterWorker(Worker *w)
{
    for (int i = 0; i < WORKER_MAX; i++)
    {
        if (!worker_list_[i].worker_)
        {
            worker_list_[i].worker_ = w;
            worker_list_[i].status_ = ewk_Wating;
            break;
        }
    }
}

void JobSystem::AddWorker()
{
    for (int i = 0; i < WORKER_NUM; i++)
    {
        Worker *wk = new Worker(this, i);
        this->RegisterWorker(wk);

        wk->start();
    }
}

void JobSystem::JobFeedback(int worker_id, int job_id)
{
    if (worker_id > WORKER_MAX || worker_id < 0)
        return;

    WorkerStat &ws = worker_list_[worker_id];

    if (!ws.worker_)
        return;

    ws.status_ = ewk_Wating;
}

MsgPacket *JobSystem::PopMsg()
{
    MsgPacket *m = msg_queue_.front();

    if (!m)
    {
        return nullptr;
    }

    msg_queue_.pop();
    return m;
}

void JobSystem::PushMsg(MsgPacket *m)
{
    msg_queue_.push(m);
}

void JobSystem::Clear()
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
