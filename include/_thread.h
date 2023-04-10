#ifndef _THREAD
#define _THREAD

#include <pthread.h>

class _Thread
{
public:
    _Thread() {}
    virtual ~_Thread() { join(); }

    void start()
    {
        pthread_create(&thread, nullptr, _Thread::entryPoint, this);
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
        _Thread *pt = static_cast<_Thread *>(pthis);
        pt->run();
        return nullptr;
    }

    pthread_t thread;
};

#endif