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

class MessageSystem;
class NetworkSystem;
class JobsSystem;

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

    // test case
    void test_case_msg();

public:
    int curr_state_;
    // 网络系统
    NetworkSystem *network_sys_;
    // 消息处理系统
    MessageSystem *msg_sys_;
    // Jobs System
    JobsSystem *jobs_sys_;
};

#endif