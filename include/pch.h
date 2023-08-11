#ifndef PCH_H
#define PCH_H

// ------- sys -------
#include <iostream>
#include <string.h>
#include <stdint.h>
// #include <thread>
//  #include <stdlib.h>
#include <error.h>
#include <errno.h>
#include <unistd.h>
#include <ctime>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/random.h>
// socket
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <sys/epoll.h>

// ------- usr --------
#include "sock_unit.h"
#include "server.h"
#include "message_system.h"
#include "msg_def.h"
#include "common.h"
#include "interface_class.h"
#include "DataPacket.h"
#include "network_system.h"
#include "fun.h"
#include "proc_func_table.h"
#include "config.h"
#include "job_system.h"
#include "worker.h"
#include "m_thread.h"
#include "m_timer.h"
#include "node_system.h"
#include "utillib.h"

using namespace std;

#define DEBUG 1

#endif
