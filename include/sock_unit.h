#ifndef M_SOCKET
#define M_SOCKET

#include <iostream>
#include <string.h>
#include <queue>

#include "common.h"

class SockUnit
{
public:
    SockUnit(int sock) : sock_(sock)
    {
    }
    ~SockUnit()
    {
        Close();
    }

    enum SockStatus{
        sNone = 0,
        sConnected,
        sShutDownRead,
        sShutDownWrite,
        // sClientShutDownRead,
        // sClientShutDownWrite,
        sWaitForClose,
    };

public:
    int GetSock()
    {
        return sock_;
    }
    void Close();

    bool PeekRecv();
    void Recv();
    void Send();

    void AllocDataPacket();

public:
    int sock_;
    int status_;

    
};

#endif