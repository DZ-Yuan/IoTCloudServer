#include "../include/pch.h"

int msg_mod = 0;
int cli_mod = 0;
int test_main_mod = 0;
int port = 0;
unsigned int addr[4] = {0, 0, 0, 0};

void test_case_client_tcp();
void test_case_client();
void test_case_server();
void test_case_msgsys();

void test_main()
{
    std::list<int> list_;

    list_.clear();
    int a = list_.front();
    list_.pop_front();
    cout << a << endl;
    cout << "t_main Done" << endl;
}

int main(int argc, char *argv[])
{
    int opt;
    opterr = 0;

    while ((opt = getopt(argc, argv, "+csp:B:MT")) != -1)
    {
        switch (opt)
        {
        case 's':
            printf("Server Mode...\n");
            break;

        case 'c':
            printf("Client Mode...\n");
            cli_mod = 1;
            break;

        case 'p':
            printf("Set Port: %s\n", optarg);
            sscanf(optarg, "%d", &port);
            break;

        case 'B':
            printf("Set Address: %s\n", optarg);
            sscanf(optarg, "%d.%d.%d.%d", &addr[0], &addr[1], &addr[2], &addr[3]);
            break;

        case 'M':
            msg_mod = 1;
            printf("Message Mod: %s\n", optarg);
            break;

        case 'T':
            test_main_mod = 1;
            printf("test_main Mod: %s\n", optarg);
            break;

        case ':':
            printf("%c option miss args\n", optopt);
            break;

        case '?':
        default:
            printf("unknow option: %c, args: %s\n", optopt, optarg);
            return 0;
        }
    }

    if (cli_mod)
    {
        // test_case_client();
        test_case_client_tcp();
    }
    else if (msg_mod)
    {
        MServer ser;
        ser.Launch();
        ser.Run();
    }
    else if (test_main_mod)
    {
        test_main();
    }
    else
    {
        // test_case_server();
        MServer ser;
        ser.Launch();
        ser.Run();
    }

    cout << "\nDemo done" << endl;
    return 0;
}
