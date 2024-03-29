#ifndef NODE_SYSTEM
#define NODE_SYSTEM

#include "common.h"
#include "interface_class.h"

#define TIMER_DELETE(t)  \
    if (t)               \
    {                    \
        timer_delete(t); \
        t = 0;           \
    }

class MServer;
class DataPacket;
class NodeSystem;
class TimerEvent;

// 0 - Hello/HeartBeatPacket
// 1 - LCC Advertisement
// 2 - Device Advertisement
// 3 - CONNECT
// 4 - COMFIRM
// 5 - REJECT
// 6 - CONTROL
enum CMD_NODE
{
    cmd_Hello = 0,
    cmd_Lcc_Advert = 1,
    cmd_Dev_Advert = 2,
    cmd_Connect = 3,
    cmd_Comfirm = 4,
    cmd_Reject = 5,
    cmd_Control = 6,
    cmd_ReqDevList = 7,
};

enum ERR_CODE
{
    err_Success = 0,
    err_Fail = 1,
};

struct NodeInfo
{
    uint8_t nid_;
    uint32_t name_;
    bool live_;
    uint32_t sock_;
    uint32_t ip_;
    uint32_t port_;
    uint32_t resp_time_;
    timer_t heartbeat_timer_id_;
    timer_t expire_timer_id_;
    std::list<TimerEvent *> timer_list_;
    // maybe we can use resp_time_ instead of create new value
    bool is_expire_;

    NodeSystem *node_system_;

    void *pending_action_;
    void *mq_;

    NodeInfo(uint8_t id, NodeSystem *node_sys_)
    {
        memset(this, 0, sizeof(*this));
        nid_ = id;
        node_system_ = node_sys_;

        is_expire_ = false;
    }

    ~NodeInfo();

    void GetDevName(char *name, int size);
    void GetDevIPString(char *ip, int size){};

    uint32_t GetDevIP() { return ip_; }

    void StartHeartBeatTimer();
    void StartExpireTimer();
    void CleanTimer(timer_t);
    void CleanAllTimer();
};

class NodeSystem : public MThread, SystemBase_Interface, NetMsgHdl_Interface
{
public:
    NodeSystem(MServer *, NetworkSystem *);
    ~NodeSystem();

public:
    virtual void run()
    {
        status_ = true;
        onRun();
    }

    void onRun();
    void RecvNetMsg(DataPacket *);
    void Acknowledge(int sock);

    int GetDevlist(std::vector<NodeInfo *> &dev_arr);
    NodeInfo *GetDevInfo(int node_id);

    bool DevInfo2Byte(NodeInfo *node, char *arr, int size);
    void SendSockData(int sock, char *data, int size);
    void CloseSock(int sock);
    void BreakOffDev(int node_id);
    void DeleteDev(int node_id);

    // cmd implement
    int DoConnect(DataPacket *);
    int DoComfirm(DataPacket *);
    int DoReqDevList(DataPacket *);

    // interface
    virtual int ReceiveNetPacket_IF(DataPacket *);

    virtual void SetStatus(int s) { status_ = s ? true : false; }
    virtual int GetStatus() { return status_ ? 1 : 0; };
    virtual void Terminate() { status_ = false; };

private:
    MServer *server_;
    // TODO：考虑将发送数据接口放在MServer中
    NetworkSystem *network_sys_;
    bool status_;
    int sys_id_;
    void *task_deliver_cb_;
    std::queue<DataPacket *> dp_queue_;

    std::unordered_map<uint8_t, NodeInfo *> node_map_;
};

#endif