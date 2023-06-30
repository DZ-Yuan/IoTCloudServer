#ifndef Mm_thread
#define Mm_thread

#include <pthread.h>

class MThread
{
public:
    MThread() {}
    virtual ~MThread() { join(); }

    void start()
    {
        pthread_create(&thread, nullptr, MThread::entryPoint, this);
    }

    void join()
    {
        pthread_join(thread, nullptr);
    }

protected:
    virtual void run() = 0;

private:
    static void *entryPoint(void *pthis)
    {
        MThread *pt = static_cast<MThread *>(pthis);
        pt->run();
        return nullptr;
    }

    pthread_t thread;
};

#endif