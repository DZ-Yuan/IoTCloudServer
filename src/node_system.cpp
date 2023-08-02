#include "pch.h"
#include "node_system.h"

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
            printf("Recv node connetc request!\n");
            err = DoConnect(dp);
            break;

        case cmd_ReqDevList:
            printf("Recv LCC req devlist request!\n");
            err = DoReqDevList(dp);
            break;

        default:
            printf("[%s]unknow cmd recv!\n", __FUNCTION__);
            break;
        }

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
    //  cmd
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

NodeInfo *NodeSystem::GetDevDetail(int node_id)
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
        info = new NodeInfo(id);
        node_map_[id] = info;
    }
    else
    {
        info = iter->second;
    }

    // dev name
    uint32_t name = dp->ReadOnce<uint32_t>();

    // update info
    info->name_ = name;
    info->sock_ = dp->sock_;
    info->ip_ = server_->network_sys_->GetSockIP(dp->sock_);
    info->live_ = true;

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

int NodeSystem::DoReqDevList(DataPacket *dp)
{
    std::vector<NodeInfo *> node_arr;
    // ?
    node_arr.clear();

    GetDevlist(node_arr);

    const int total_size = node_arr.size() * NODE_INFO_BYTE_ARR_SIZE;
    char *buf = new char[total_size];

    memset(buf, 0, total_size);

    char node_byte[NODE_INFO_BYTE_ARR_SIZE] = {0};
    char *p = buf;
    for (int i = 0; i < node_arr.size(); i++)
    {
        int len = sizeof(node_byte);
        DevInfo2Byte(node_arr[i], node_byte, len);
        memcpy(buf, node_byte, len);
        p += len;

        memset(node_byte, 0, len);
    }

    // msg head
    char msg[DATAPACKET_SIZE] = {0};
    msg[0] = cmd_Comfirm;
    msg[1] = err_Success;
    msg[2] = cmd_ReqDevList;

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
