#ifndef TASK_SYSTEM
#define TASK_SYSTEM

#include <queue>
#include "m_thread.h"

#define WORKER_MAX 20
#define WORKER_NUM 3

class Worker;

enum JOBSYSTEMSTATUS
{
    ejob_Err = -1,
    ejob_None = 0,
    ejob_Running = 1,
    ejob_Terminated = 2,
};

struct WorkerStat
{
    Worker *worker_;
    int status_;

    WorkerStat() : worker_(nullptr), status_(ejob_None) {}
};

class JobSystem : public MThread
{
public:
    JobSystem(MServer *);
    ~JobSystem();

public:
    void run();

    void Assign();
    Worker *GetIdleWorker();

    void RegisterWorker(Worker *);
    void AddWorker();

    void JobFeedback(int, int);

    MsgPacket *PopMsg();
    void PushMsg(MsgPacket *);

    void Terminate() { status_ = ejob_Terminated; }
    void Clear();

private:
    int status_;

    int worker_num_;
    WorkerStat worker_list_[WORKER_MAX];

    std::queue<MsgPacket *> msg_queue_;

    MServer *server_;
};

#endif