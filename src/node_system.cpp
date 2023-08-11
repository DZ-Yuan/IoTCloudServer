#include "pch.h"

NodeInfo::~NodeInfo()
{
    CleanTimer(this->heartbeat_timer_id_);
    CleanTimer(this->expire_timer_id_);
}

void NodeInfo::GetDevName(char *name, int size)
{
    // need 5 byte
    if (size < sizeof(name_) + 1)
    {
        return;
    }

    memcpy(name, &name_, sizeof(name_));
    name[5] = '\0';
}

void NodeInfo::StartHeartBeatTimer()
{
    union sigval v;
    v.sival_ptr = this;

    void (*cb)(union sigval v) = [](union sigval v)
    {
        NodeInfo *info = (NodeInfo *)v.sival_ptr;

        // msg head
        char msg[DATAPACKET_SIZE] = {0};
        msg[0] = (uint8_t)cmd_Hello;
        *(uint32_t *)(msg + 1) = (uint32_t)CLOUD_ID;

        info->node_system_->SendSockData(info->sock_, msg, sizeof(msg));

#if DEBUG
        printf("Send Heartbeat to dev %d\n", info->nid_);
#endif
    };

    this->heartbeat_timer_id_ = SetInterval(cb, 20, v);
}

void NodeInfo::StartExpireTimer()
{
    union sigval v;
    v.sival_ptr = this;

    void (*cb)(union sigval v) = [](union sigval v)
    {
        NodeInfo *info = (NodeInfo *)v.sival_ptr;

        if (info->is_expire_)
        {
            printf("Dev node is expired...\n");
            info->node_system_->BreakOffDev(info->nid_);
            return;
        }

        info->is_expire_ = true;
#if DEBUG
        printf("Update expire flag\n");
#endif
    };

    this->expire_timer_id_ = SetInterval(cb, 60, v);
}

void NodeInfo::CleanTimer(timer_t t)
{
    if (t)
    {
        timer_delete(t);
        t = nullptr;
    }
}

NodeSystem::NodeSystem(MServer *ser, NetworkSystem *net)
    : server_(ser), network_sys_(net), sys_id_(sidNodeSystem)
{
    // Reg Net msg callback
    ser->RegNetMsgRecvCallback(sys_id_, this);
}

NodeSystem::~NodeSystem()
{
}

void NodeSystem::onRun()
{
    while (status_)
    {
        auto *dp = dp_queue_.front();

        if (!dp)
            continue;

        dp_queue_.pop();

        // parse
        // cmd
        uint8_t cmd = dp->ReadOnce<uint8_t>();

        int err = -1;
        switch (cmd)
        {
        case cmd_Connect:
            printf("Recv node connet request!\n");
            err = DoConnect(dp);
            break;

        case cmd_Comfirm:
            printf("Recv node comfirm reply!\n");
            err = DoComfirm(dp);
            break;

        case cmd_ReqDevList:
            printf("Recv LCC req devlist request!\n");
            err = DoReqDevList(dp);
            break;

        default:
            printf("[%s]unknow cmd recv!\n", __FUNCTION__);
            break;
        }

        printf("NodeSystem cmd run result: %d\n", err);
        // send response

        SafeDelete(dp);
    }
}

void NodeSystem::RecvNetMsg(DataPacket *dp)
{
    dp_queue_.push(dp);
}

void NodeSystem::Acknowledge(int sock)
{
    char msg[DATAPACKET_SIZE] = {0};
    // cmd
    msg[0] = (uint16_t)DATAPACKET_SIZE;
    msg[2] = cmd_Comfirm;

    network_sys_->SendSockData(sock, msg, sizeof(msg));
}

int NodeSystem::GetDevlist(std::vector<NodeInfo *> &dev_arr)
{
    for (auto it = node_map_.begin(); it != node_map_.end(); ++it)
    {
        dev_arr.push_back(it->second);
    }

    return node_map_.size();
}

NodeInfo *NodeSystem::GetDevInfo(int node_id)
{
    auto it = node_map_.find(node_id);

    return it == node_map_.end() ? nullptr : it->second;
}

bool NodeSystem::DevInfo2Byte(NodeInfo *node, char *arr, int size)
{
    if (size < NODE_INFO_BYTE_ARR_SIZE)
    {
        return false;
    }

    char *p = arr;

    int len = sizeof(node->nid_);
    memcpy(p, &node->nid_, len);
    p += len;

    len = sizeof(node->name_);
    memcpy(p, &node->name_, len);
    p += len;

    len = sizeof(node->live_);
    memcpy(p, &node->live_, len);

    return true;
}

void NodeSystem::SendSockData(int sock, char *data, int size)
{
    this->network_sys_->SendSockData(sock, data, size);
}

void NodeSystem::CloseSock(int sock)
{
    this->network_sys_->CloseOneSock(sock);
}

void NodeSystem::BreakOffDev(int node_id)
{
    auto iter = node_map_.find(node_id);
    if (iter == node_map_.end())
    {
        return;
    }

    NodeInfo *info = iter->second;
    info->live_ = false;

    info->CleanTimer(info->heartbeat_timer_id_);
    info->CleanTimer(info->expire_timer_id_);

    CloseSock(info->sock_);
    info->sock_ = -1;
}

int NodeSystem::DoConnect(DataPacket *dp)
{
    if (!dp)
    {
        return err_Fail;
    }

    NodeInfo *info = nullptr;
    // dev id
    uint8_t id = dp->ReadOnce<uint8_t>();

    // find if already exist
    auto iter = node_map_.find(id);
    if (iter == node_map_.end())
    {
        info = new NodeInfo(id, this);
        node_map_[id] = info;
    }
    else
    {
        info = iter->second;
    }

    // 检查上次连接是否已经断开
    if (info->live_)
    {
        printf("Break off last connect...\n");
        BreakOffDev(info->nid_);
    }

    // dev name
    uint32_t name = dp->ReadOnce<uint32_t>();

    // update info
    info->name_ = name;
    info->sock_ = dp->sock_;
    info->ip_ = server_->network_sys_->GetSockIP(dp->sock_);
    info->live_ = true;
    info->is_expire_ = false;

    info->StartHeartBeatTimer();
    info->StartExpireTimer();

#if DEBUG
    printf("Dev id: %d\n", info->nid_);

    char dev_name[5] = {0};
    info->GetDevName(dev_name, sizeof(dev_name));
    printf("Dev name: %s\n", dev_name);

    in_addr addr;
    addr.s_addr = info->GetDevIP();
    printf("Dev ip: %s\n", inet_ntoa(addr));

    printf("Dev status: %d\n", info->live_ ? 1 : 0);
#endif

    Acknowledge(info->sock_);
    return err_Success;
}

int NodeSystem::DoComfirm(DataPacket *dp)
{
    if (!dp)
    {
        return err_Fail;
    }
    // exec errCode
    uint8_t err_code = dp->ReadOnce<uint8_t>();
    // cmd of peer want to comfirm
    uint8_t comfire_cmd = dp->ReadOnce<uint8_t>();
    // dev id
    uint8_t nid = dp->ReadOnce<uint8_t>();

    NodeInfo *info = GetDevInfo(nid);
    if (!info)
    {
        printf("Can't find the dev info, dev id is: %d\n", nid);
        return err_Fail;
    }

    switch (comfire_cmd)
    {
    case cmd_Hello:
        info->is_expire_ = false;
        break;

    default:
        printf("Invaild CMD: %d, dev id is: %d\n", comfire_cmd, nid);
        return err_Fail;
    }

    return err_Success;
}

int NodeSystem::DoReqDevList(DataPacket *dp)
{
    if (!dp)
    {
        return err_Fail;
    }

    std::vector<NodeInfo *> node_arr;
    // ?
    node_arr.clear();

    GetDevlist(node_arr);

    int dev_count = node_arr.size();
    const int total_size = (dev_count * NODE_INFO_BYTE_ARR_SIZE) + 1;
    char *buf = new char[total_size];

    memset(buf, 0, total_size);
    // memcpy(buf, &dev_count, sizeof(uint8_t));
    buf[0] = (uint8_t)dev_count;

    char node_byte[NODE_INFO_BYTE_ARR_SIZE] = {0};
    char *p = buf + 1;
    for (int i = 0; i < dev_count; i++)
    {
        int len = sizeof(node_byte);
        DevInfo2Byte(node_arr[i], node_byte, len);
        memcpy(p, node_byte, len);
        p += len;

        memset(node_byte, 0, len);
    }

    // msg head
    char msg[DATAPACKET_SIZE] = {0};
    msg[0] = (uint8_t)cmd_Comfirm;
    msg[1] = (uint8_t)err_Success;
    msg[2] = (uint8_t)cmd_ReqDevList;

    memcpy(msg + 3, buf, total_size);

    // send dev list respond
    network_sys_->SendSockData(dp->sock_, msg, sizeof(msg));

    return err_Success;
}

int NodeSystem::ReceiveNetPacket_IF(DataPacket *dp)
{
    printf("New DataPacket receive!\n");
    this->RecvNetMsg(dp);
    return 0;
}
