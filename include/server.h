#ifndef M_SERVER
#define M_SERVER

#include <string>
#include <vector>
#include <unordered_map>
#include <list>
#include <thread>
#include <functional>
#include <sys/epoll.h>

#include "common.h"
#include "interface_class.h"

class MessageSystem;
class NetworkSystem;
class JobSystem;
class NodeSystem;

// typedef void (*accept_cb)(void *, void *);

enum SYSTEM_ID
{
    sidNull = 0,
    sidNodeSystem = 1,
    sidEnd,
};

// #define SerPrint()
/*
    threads：

*/
class MServer
{
public:
    MServer();
    ~MServer();

    enum ServerState
    {
        sUnInitalized = 0,
        sRunnig,
        sTerminated,
    };

public:
    // main
    void Init();
    void Launch();

    void Run();
    void Terminate();

    // common
    int GetCurrentState()
    {
        return curr_state_;
    }

    // 消息发送接收 unused
    bool SendMsg(int sock, char *msg, int size);

    void SerPrint(std::string m)
    {
        std::cout << "[Server] : " << m << std::endl;
    }

    void PostMsg();

    uint64_t GetTime(){};
    uint64_t GetTick(){};

    // cb
    void RegNetMsgRecvCallback(int sys_id, NetMsgHdl_Interface *cb);
    int CallNetMsgRecvCallback(int sys_id, DataPacket *dp);

    // test case
    void test_case_msg();

public:
    int curr_state_;
    // 网络系统
    NetworkSystem *network_sys_;
    // 消息处理系统
    MessageSystem *msg_sys_;
    // Jobs System
    JobSystem *jobs_sys_;
    // Node System
    NodeSystem *node_sys_;

    NetMsgHdl_Interface *NetMsgRecvCallbackFunc[sidEnd];
};

#endif