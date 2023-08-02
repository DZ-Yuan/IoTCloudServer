#ifndef WORKER
#define WORKER

#include <iostream>
#include "m_thread.h"

enum WORKSTAT
{
    ewk_None = 0,
    ewk_Working = 1,
    ewk_Wating = 2,
    ewk_Retire,
};

class Worker : public MThread
{
public:
    Worker(JobSystem *, int);
    ~Worker();

public:
    virtual void run() override { OnWork(); }

    void OnWork();
    void AcceptJob(MsgPacket *);
    void Done();

    int GetWorkerId()
    {
        return self_id_;
    }

    void Retire() { status_ = ewk_Retire; }
    void Clear();

private:
    int self_id_;
    int status_;

    int job_id_;

    JobSystem *jobs_sys_;
    MsgPacket *msg_;
};

#endif