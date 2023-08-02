#ifndef M_TIMER
#define M_TIMER

struct MsgPacket;
class MessageSystem;
class MTimer;

struct Event
{
    MsgPacket *msg_ = nullptr;
    // void *f;
    // void *ctx;
    bool loop_;
    int repeat_;
};

struct EventSlot
{
    std::list<Event *> list_;

    EventSlot()
    {
        list_.clear();
    }

    void AppendEvent(Event *e);
    Event *GetEvent();
    bool IsEmpty() { return list_.empty(); }
    void Clean();
};

class TickTimer : public MThread
{
public:
    TickTimer(MTimer *mt);
    ~TickTimer() {}

public:
    virtual void run();

    void exit() { run_ = false; }

private:
    bool run_;
    MTimer *timer_;
};

class MTimer : public MThread
{
public:
    MTimer(MessageSystem *);
    ~MTimer();
    virtual void run();
    void TickUpdate();
    void ProcEvent();
    void AddTimer(uint32_t ms, MsgPacket *ne);
    // TODO use template
    // void AddTimer(uint32_t ms, MsgPacket *ne);
    void InsertEvent(uint32_t t, Event *ne);
    // void InsertEvent(uint32_t t, MsgPacket *ne);

    uint32_t GetTick() { return ms_tick_; }

private:
    bool status_;
    // uint64 time_wheel_ = 0;
    uint32_t ms_tick_;
    int event_count_;

    std::unordered_map<int, EventSlot> event_map_;
    std::list<EventSlot *> expire_event_;

    TickTimer *t_timer_;
    MessageSystem *msg_system_;
};

#endif
