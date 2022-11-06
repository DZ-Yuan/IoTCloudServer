#include "../include/pch.h"

SockData::SockData(int sock)
{
    memset(this, 0, sizeof(*this));

    sock_ = sock;
    int total_size_ = PACK_DEFAULT_SIZE;
    //
    mem_ptr_ = (char *)malloc(total_size_);
    offset_ = mem_ptr_;
    end_ptr_ = mem_ptr_ + total_size_;
}

SockData::~SockData()
{
    if (mem_ptr_)
    {
        delete mem_ptr_;
        mem_ptr_ = nullptr;
    }
}

void SockData::Resize(int need)
{
    //申请
    int total = GetTotalSize() + need - GetAvailable();
    char *n_mem = (char *)malloc(total);
    memset(n_mem, 0, total);
    //拷贝原数据
    int offset_size = GetOffsetSize();
    memcpy(n_mem, mem_ptr_, offset_size);
    delete mem_ptr_;
    //更新指针
    mem_ptr_ = n_mem;
    offset_ = mem_ptr_ + offset_size;
    end_ptr_ = mem_ptr_ + total;
}

void SockData::Append(const char *src, int size)
{
    if (!src || size <= 0)
    {
        return;
    }
    //比较下剩余大小
    if (size > GetAvailable())
    {
        Resize(size);
    }

    // warning：线程不安全
    memcpy(offset_, src, size);
    offset_ += size;
}

SockDataReader::SockDataReader(int sock)
    : SockData(sock)
{
}

SockDataReader::~SockDataReader()
{
}

void SockDataReader::ParseData(void *pack_data, int &pack_size)
{
    pack_data = nullptr;
    pack_size = 0;

    //解析头
    int ret = ParseDataLen();
    if (ret < 0)
    {
        return;
    }

    int offset_size = GetOffsetSize();
    //判断剩余长度
    if (data_len_ > offset_size - parsed_)
    {
        return;
    }

    //申请空间
    char *d = (char *)malloc(data_len_);
    memset(d, 0, data_len_);
    //复制数据
    memcpy(d, mem_ptr_, data_len_);
    parsed_ += data_len_;

    //数据包内存地址
    pack_data = d;
    pack_size = data_len_;

    //调整剩余数据位置
    int s = offset_size - parsed_;
    if (s <= 0)
    {
        memset(mem_ptr_, 0, GetTotalSize());
        offset_ = mem_ptr_;
        return;
    }

    //拷贝剩余数据到前头
    memcpy(mem_ptr_, mem_ptr_ + parsed_, s);
    offset_ = mem_ptr_ + s;
    memset(offset_, 0, GetAvailable());
}

int SockDataReader::ParseDataLen()
{
    if (data_len_ > 0)
    {
        return data_len_;
    }

    if (GetOffsetSize() < sizeof(ushort))
    {
        return -1;
    }

    data_len_ = *(ushort *)mem_ptr_;
    parsed_ += sizeof(ushort);

    return data_len_;
}

void SockDataReader::Adjust()
{
    // int n_size = PACK_DEFAULT_SIZE;
    // if (used_ - parsed_ > PACK_DEFAULT_SIZE)
    // {
    //     n_size = used_ - parsed_;
    // }

    // //检查buf大小
    // if (total_size_ > 2 * PACK_DEFAULT_SIZE)
    // {
    // }

    // int rlen = used_ - parsed_;
    // char *new_buf = (char *)malloc(rlen);
    // memset(new_buf, 0, rlen);
    // memcpy(new_buf, data_ + parsed_, rlen);
}

NetworkSystem::NetworkSystem(MServer *server)
    : status_(sInActive), ser_sock_(0), address_("127.0.0.1"), port_(1668), server_(server)
{
    sock_map_.clear();
}

NetworkSystem::~NetworkSystem()
{
    ClearSockDataQueue();
    ClearSockContainer();
    ClearPacketQueue();
    DisconnectSock(ser_sock_);
    // todo 清理epoll
}

void NetworkSystem::SetupConfig(string addr, int port)
{
    address_ = addr;
    port_ = port;
}

void NetworkSystem::InitSerSock()
{
    ser_sock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // set host address
    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(address_.c_str());
    server_addr.sin_port = htons(port_);

    bind(ser_sock_, (sockaddr *)&server_addr, sizeof(server_addr));
}

void NetworkSystem::Start()
{
    // 初始化sock
    InitSerSock();
    //创建epoll
    epfd_ = epoll_create(20);

    // 监听线程
    listen_thread_ = thread((OnListenThread()));
    listen_thread_.detach();
    // epoll事件线程
    epoll_thread_ = thread((OnEpollEventThread()));
    epoll_thread_.detach();
    // 数据处理线程
    data_thread_ = thread((OnProcSockDataThread()));
    data_thread_.detach();

    SetStatus(sRunning);
}

function<void()> NetworkSystem::OnListenThread()
{
    return [&]() -> void
    {
        this->OnListen();
    };
}

function<void()> NetworkSystem::OnEpollEventThread()
{
    return [&]() -> void
    {
        this->OnEpollEvent();
    };
}

function<void()> NetworkSystem::OnProcSockDataThread()
{
    return [&]() -> void
    {
        this->OnProcSockData();
    };
}

void NetworkSystem::OnListen()
{
    listen(this->ser_sock_, 20);
    char outmsg[20];
    sprintf(outmsg, "Server IP %s : %d", address_.c_str(), port_);
    OutputPrint(outmsg);
    OutputPrint("Listening...");

    while (this->GetStatus() != sTerminated)
    {
        // client sock
        sockaddr_in client_addr;
        socklen_t addr_size = sizeof(client_addr);
        int client_sock = accept(ser_sock_, (sockaddr *)&client_addr, &addr_size);

        if (client_sock < 0)
        {
            OutputPrint("Socket Error");
            continue;
        }

        //添加监听客户端的socket
        if (!this->AddEpollEvent(client_sock))
        {
            continue;
        }

        AddNewClientSock(client_sock);

        string res = SERVER_CONN_RESPONES;
        send(client_sock, res.c_str(), sizeof(res.c_str()), 0);
        OutputPrint("New Client Connect Successfully !");
    }

    OutputPrint("Listen thread close...");
}

void NetworkSystem::OnEpollEvent()
{
    while (GetStatus() != sTerminated)
    {
        // todo 先设定timeout时间，后面再做wait唤醒
        int e_size = epoll_wait(this->epfd_, this->ep_events, SERVER_MAX_EPOLL_EVENTS, 5000);
        if (e_size <= 0)
        {
            continue;
        }

        cout << "Epoll Event - count: " << e_size << endl;
        for (int i = 0; i < e_size; ++i)
        {
            RecvSockData(ep_events[i].data.fd);
        }
    }

    OutputPrint("Epoll thread close...");
}

void NetworkSystem::OnProcSockData()
{
    while (GetStatus() != sTerminated)
    {
        SockData *sd = sock_buffer_queue_.front();
        if (!sd)
        {
            continue;
        }

        sock_buffer_queue_.pop();

        //查找sock容器
        auto iter = sock_map_.find(sd->sock_);
        if (iter == sock_map_.end())
        {
            delete sd;
            continue;
        }

        SockDataReader *reader = iter->second;
        //追加数据到指定sock中
        reader->Append(sd->GetMemPtr(), sd->GetOffsetSize());
        delete sd;
        sd = nullptr;

        //尝试解析数据
        char *data = nullptr;
        int size = 0;
        reader->ParseData(data, size);

        if (data)
        {
            //打包数据
            PackageInDataPacket(data, size);
        }
    }

    OutputPrint("Sock Data Proc thread close...");
}

void NetworkSystem::OnSendSockData()
{
    while (GetStatus() != sTerminated)
    {
        auto *sd = dp_send_queue_.front();
        if (!sd)
        {
            continue;
        }

        sock_buffer_queue_.pop();

        //SendSockData(sd->sock_, sd->GetMemPtr(), sd->GetOffsetSize());
    }
}

void NetworkSystem::AddNewClientSock(int sock)
{
    SockDataReader *sdr = new SockDataReader(sock);
    sock_map_[sock] = sdr;
}

bool NetworkSystem::AddEpollEvent(int fd)
{
    if (fd <= 0)
    {
        return false;
    }

    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = fd;

    bool res = epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev);
    if (res == -1)
    {
        OutputPrint("Add epoll event fail !");
    }

    return res >= 0;
}

bool NetworkSystem::DelEpollEvent(int fd)
{
    if (fd <= 0)
    {
        return false;
    }

    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = fd;

    bool res = epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, &ev);
    if (res == -1)
    {
        OutputPrint("Del epoll event fail !");
    }

    return res > 0;
}

void NetworkSystem::RecvSockData(int sock)
{
    SockData *sd = new SockData(sock);

    //循环读取
    while (1)
    {
        int ret = RecvOnce(sock, buffer_, PACK_DEFAULT_SIZE);
        if (ret < 0)
        {
            //对端断开连接
            CloseOneSock(sock);
            SafeDelete(sd);
            break;
        }
        //无数据可读
        if (ret == 0)
        {
            break;
        }

        // copy到sock缓存数据中
        sd->Append(buffer_, ret);
        memset(buffer_, 0, BUFFER_SEIZ);
    }

    sock_buffer_queue_.push(sd);
}

int NetworkSystem::RecvOnce(int sock, char *buf, int size)
{
    int t = size;
    while (t > 0)
    {
        int rs = recv(sock, buf + (size - t), t, 0);

        if (rs <= -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
        {
            return size - t;
        }
        else if (rs == 0)
        {
            return -1;
        }

        t -= rs;
    }

    return size - t;
}

void NetworkSystem::SendSockData(int sock, char *data, int size)
{
    int s = size;
    while (size > 0)
    {
        int ss = send(sock, data + (size - s), s, 0);
        if (ss <= -1)
        {
            return;
        }

        s -= ss;
    }

    return;
}

void NetworkSystem::CloseOneSock(int sock)
{
    auto it = sock_map_.find(sock);
    if (it != sock_map_.end())
    {
        delete it->second;
    }

    DisconnectSock(sock);
}

void NetworkSystem::ClearSockDataQueue()
{
    SockData *sd = nullptr;

    while (sd = sock_buffer_queue_.front())
    {
        delete sd;
        sd = nullptr;
        sock_buffer_queue_.pop();
    }
}

void NetworkSystem::ClearSockContainer()
{
    for (auto it = sock_map_.begin(); it != sock_map_.end();)
    {
        DisconnectSock(it->first);
        delete it->second;
        sock_map_.erase(it++);
    }
}

void NetworkSystem::ClearPacketQueue()
{
    DataPacket *dp = nullptr;

    while (dp = packet_queue_.front())
    {
        delete dp;
        dp = nullptr;
        packet_queue_.pop();
    }
}

void NetworkSystem::PackageInDataPacket(void *data, int size)
{
    if (!data || size <= 0)
    {
        return;
    }

    DataPacket *dp = new DataPacket(static_cast<char *>(data), size);
    packet_queue_.push(dp);
}
