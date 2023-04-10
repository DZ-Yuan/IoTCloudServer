#include "pch.h"
#include "worker.h"

Worker::Worker(JobsSystem *t, int id)
    : jobs_sys_(t), self_id_(id), status_(ewk_None)
{
}

Worker::~Worker()
{
    cout << "Bye" << endl;
}

void Worker::OnWork()
{
    this->status_ = ewk_Wating;

    cout << "Worker " << self_id_ << " waiting..." << endl;

    while (this->status_ != ewk_Retire)
    {
        if (this->status_ == ewk_Wating || !this->msg_)
        {
            continue;
        }

        // Change data type
        // Get func from the function map
        ProcFunc f = f_map[msg_->ftype_];
        // Do
        cout << "Worker " << self_id_ << " get a job..." << endl;
        f((FDATA *)msg_->data_);
        // Finish Job
        this->Done();
    }

    Clear();

    return;
}

void Worker::AcceptJob(MsgPacket *msg_packet)
{
    if (!msg_packet)
    {
        return;
    }

    // Change the status
    this->msg_ = msg_packet;
    this->status_ = ewk_Working;
}

void Worker::Done()
{
    // feedback to task system
    jobs_sys_->JobFeedback(this->self_id_, this->job_id_);
    // destory the msg packet
    SafeDelete(msg_);
    this->status_ = ewk_Wating;
}

void Worker::Clear()
{
    SafeDelete(msg_);
    jobs_sys_ = nullptr;
}
