#include <time.h>

int msglint_main(int argc, char** argv);

int __cdecl main(int argc, char** argv)
{
    WSADATA Wd;
    int result;

    if(WSAStartup(MAKEWORD(2,2), &Wd)==0)
    {
        result = msglint_main(argc, argv);

        WSACleanup();
    }
    else
    {
        result = -1;
    }

    return result;
}

struct tm* gmtime_r(const time_t* timep, struct tm* result)
{
    struct tm* t = gmtime(timep);

    memcpy(result, t, sizeof(result[0]));

    return result;
}

struct tm* localtime_r(const time_t* timep, struct tm* result)
{
    struct tm* t = localtime(timep);

    memcpy(result, t, sizeof(result[0]));

    return result;
}
