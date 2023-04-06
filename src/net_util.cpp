#include "pch.h"

#define DEBUG 0


// Tool
void _split_addr(char* addr, char* ip, int* port)
{
    if(!addr || !ip || !port)
    {
        return;
    }

    // addr: "192.168.11.1:1234"
    sscanf(addr, "%[^:]:%d", &ip, &port);
    
#if DEBUG
    printf("[%s] Check input addr: %s:%d\n", __FUNCTION__, addr, port);
    printf("[%s] Check output addr: %s:%d\n", __FUNCTION__, addr, port);
#endif
}


int CreateUDPSock(char* addr)
{
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    return sock;
}

int CreateTCPSock(char* addr)
{
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    return sock;
}

int SendMsg(int sock, char* buf, int size)
{
   int t = size;

   while(t > 0)
   {
        int res = send(sock, buf, t, 0);

        if(res < 0)
        {
            if(errno == EAGAIN || errno == EWOULDBLOCK)
                continue;

            return size - t;    
        }

        buf += res;
        t -= res;
   }

   return 0;
}

int RecvMsg(int sock, char* buf, int size)
{

    int t = size;

    while(t > 0)
    {
        int res = recv(sock, buf, t, MSG_DONTWAIT);

        //recv return -1 for error(nothing to read, EAGAIN), 0 for the peer has performed an orderly shutdown
        if(res == 0)
        {
            return -1;
        }
        else if(res < 0)
        {
            break;
        }

        buf += res;
        t -= res;
    }

    return t;
}

// peer: "192.168.11.1:1234"
int SendMsgTo(int sock, char* buf, int size, char* peer)
{
    struct sockaddr_in peer_addr;

    char addr[24] = {0};
    int port = 0;

    sscanf(peer, "%[^:]:%d", &addr, &port);
    printf("[%s] Check peer addr: %s:%d\n", __FUNCTION__, addr, port);
    
    bzero(&peer_addr, sizeof(peer_addr));
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_addr.s_addr = inet_addr(addr);
    peer_addr.sin_port = htons(port);

    int res = sendto(sock, buf, size, 0, (struct sockaddr*)&peer_addr, sizeof(peer_addr));

    if(res <= 0)
    {
        printf("[%s] UDP send: send err\n", __FUNCTION__);
    }

    return 0;
}

int RecvMsgFrom(int sock, char* buf, int size, char* local, char* peer)
{
    struct sockaddr_in peer_addr, local_addr;

    char laddr[32] = {0}, paddr[32] = {0};
    int lport = 0, pport=0;

    sscanf(local, "%[^:]:%d", &laddr, &lport);
    printf("[%s] Check local addr: %s:%d\n", __FUNCTION__, laddr, lport);
    sscanf(peer, "%[^:]:%d", &paddr, &pport);
    printf("[%s] Check peer addr: %s:%d\n", __FUNCTION__, paddr, pport);
    
    bzero(&local_addr, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = inet_addr(laddr);
    local_addr.sin_port = htons(lport);

    bzero(&peer_addr, sizeof(peer_addr));
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_addr.s_addr = inet_addr(paddr);
    peer_addr.sin_port = htons(pport);

    //socklen_t == unsigned int
    socklen_t addr_len = sizeof(peer_addr);

    if(bind(sock, (struct sockaddr *) &local_addr, sizeof(local_addr)) < 0)
    {
        perror("bind() error");
        exit(1);
    }

    int res = recvfrom(sock, buf, size, 0, (struct sockaddr*)&peer_addr, &addr_len);

    if(res <= 0)
    {
        printf("[%s] UDP recv err!\n", __FUNCTION__);
    }

    printf("Recv: %s", buf);

    return 0;
}


void test_case_client()
{
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    if(sock < 0)
        return;

    char buf[128] = {0};
    char cli_addr[64] = {0}, ser_addr[64] = {0};

    // snprintf(cli_addr, sizeof(cli_addr), "%s:%d", CLI_IP, CLI_PORT);
    snprintf(ser_addr, sizeof(ser_addr), "%s:%d", SER_IP, SER_PORT);

    //RecvMsgFrom(sock, buf, sizeof(buf), cli_addr, ser_addr);
    snprintf(buf, sizeof(buf), "This data is send via UDP Protocol");
    SendMsgTo(sock, buf, sizeof(buf), ser_addr);

    close(sock);
}

void test_case_server()
{
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    if(sock < 0)
        return;

    char buf[128] = {0};
    char cli_addr[64] = {0}, ser_addr[64] = {0};

    snprintf(cli_addr, sizeof(cli_addr), "%s:%d", CLI_IP, CLI_PORT);
    snprintf(ser_addr, sizeof(ser_addr), "%s:%d", SER_IP, SER_PORT);

    RecvMsgFrom(sock, buf, sizeof(buf), ser_addr, cli_addr);

    close(sock);
}
