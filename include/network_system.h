#ifndef NETWORK_SYS
#define NETWORK_SYS

#include <string>
#include <queue>
#include <unordered_map>
#include <thread>
#include <functional>
#include <sys/epoll.h>
#include "sock_unit.h"
#include "common.h"

#define BUFFER_SEIZ 1024
#define PACK_DEFAULT_SIZE 128

#define SERVER_MAX_EPOLL_EVENTS 20
#define SERVER_CONN_RESPONES "Server is online..."

class MServer;
class DataPacket;

struct SockData
{
    int sock_;
    //
    char *mem_ptr_;
    char *offset_;
    char *end_ptr_;

    SockData(int sock);
    virtual ~SockData();

    void Resize(int need);
    void Reset(){};
    void Append(const char *src, int size);

    int GetAvailable()
    {
        return end_ptr_ - offset_;
    }
    int GetOffsetSize()
    {
        return offset_ - mem_ptr_;
    }
    int GetTotalSize()
    {
        return end_ptr_ - mem_ptr_;
    }
    char *GetMemPtr()
    {
        return mem_ptr_;
    }
};

struct SockDataReader : SockData
{
    ushort data_len_;
    //已解析长度
    int parsed_;

    SockDataReader(int sock);
    virtual ~SockDataReader();

    void ParseData(void *pack_data, int &size);
    int ParseDataLen();
    // unused
    void Adjust();
};

struct SockDataWriter
{
    char *w_offset_;
    DataPacket *dp_;

    SockDataWriter(DataPacket *dp)
        : dp_(dp), w_offset_(dp->GetMemPtr())
    {
    }
    ~SockDataWriter()
    {
        if (dp_)
        {
            delete dp_;
            dp_ = nullptr;
        }
    }
};

/*
    threads：
    1 监听；1 epoll响应；1 sock数据处理
*/
//网络系统
class NetworkSystem
{
public:
    NetworkSystem(MServer *server);
    ~NetworkSystem();

    enum StatusType
    {
        sInActive = 0,
        sRunning,
        sTerminated,
    };

public:
    void SetupConfig(std::string addr, int port);
    //创建服务器sock
    void InitSerSock();
    void Start();

    // thread
    std::function<void()> OnListenThread();
    std::function<void()> OnEpollEventThread();
    std::function<void()> OnProcSockDataThread();
    //监听
    void OnListen();
    //处理epoll事件
    void OnEpollEvent();
    //处理Sock数据
    void OnProcSockData();
    // todo 发送sock数据
    void OnSendSockData();

    // func
    void SetStatus(int f)
    {
        status_ = f;
    }
    int GetStatus()
    {
        return status_;
    }
    void Terminate()
    {
        SetStatus(sTerminated);
    }

    void AddNewClientSock(int sock);

    // epoll事件
    bool AddEpollEvent(int fd);
    bool DelEpollEvent(int fd);

    //接收sock数据
    void RecvSockData(int sock);
    int RecvOnce(int sock, char *buf, int size);
    //发送sock数据
    void SendSockData(int sock, char *data, int size);

    //关闭一个sock
    void CloseOneSock(int sock);
    //清理
    void ClearSockDataQueue();
    void ClearSockContainer();
    void ClearPacketQueue();

    //打包sock数据
    void PackageInDataPacket(void *data, int size);
    //获取可用数据
    DataPacket *AcquireDataPacket()
    {
        DataPacket *dp = packet_queue_.front();
        if (dp)
        {
            packet_queue_.pop();
        }

        return dp;
    }

    DataPacket *AllocDataPacket();

    // util
    void OutputPrint(std::string m)
    {
        std::cout << "[ NetworkSystem ] : " << m << std::endl;
    }

    void DisconnectSock(int s)
    {
        DelEpollEvent(s);
        shutdown(s, SHUT_RDWR);
        close(s);
    }

public:
    int status_;
    char buffer_[BUFFER_SEIZ];
    // server sock
    int ser_sock_;
    std::string address_;
    int port_;

    // epoll
    int epfd_;
    epoll_event ep_events[SERVER_MAX_EPOLL_EVENTS];

    // sock数据接收缓冲
    std::queue<SockData *> sock_buffer_queue_;
    // sock容器
    std::unordered_map<int, SockDataReader *> sock_map_;
    // DataPacket数据发送队列
    std::queue<SockDataWriter *> dp_send_queue_;

    // 已解释数据包队列
    std::queue<DataPacket *> packet_queue_;

    // MServer
    MServer *server_;

    //监听线程
    std::thread listen_thread_;
    // epoll处理线程
    std::thread epoll_thread_;
    // sock数据处理线程
    std::thread data_thread_;
};

#endif
