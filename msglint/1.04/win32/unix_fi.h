#include <winsock2.h>  // gethostname in msglint.c

/*
    not supported
*/
#define popen(COMMANDLINE,MODE) (NULL)
#define pclose(HANDLE) (0)

/*
    aliasing
*/
#define getpid() GetCurrentProcessId()
#define strncasecmp strnicmp

struct tm* gmtime_r(const time_t* timep, struct tm* result);
struct tm* localtime_r(const time_t* timep, struct tm* result);
