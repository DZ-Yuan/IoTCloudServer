#include "../include/pch.h"

int main()
{
    MServer server;
    server.Launch();
    server.Run();

    cout << "Demo done" << endl;
    return 0;
}
