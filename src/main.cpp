#include "../include/pch.h"

int cli_mod = 0;
int port = 0;
unsigned int addr[4] = {0, 0, 0, 0};

void test_case_client();
void test_case_server();

int main(int argc, char *argv[])
{
    int opt;
    opterr = 0;

    while ((opt = getopt(argc, argv, "+csp:B:")) != -1)
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
        test_case_client();
    }
    else
    {
        test_case_server();
    }

    cout << "Demo done" << endl;
    return 0;
}
