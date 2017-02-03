/*
    workaround wprintf name collision
*/
#ifdef wprintf
    #undef wprintf
#endif
#define wprintf msglint_wprintf

/*
    md5.h need stdcall, md5.c not, but main must not be, so /Gz and redo main()
*/
#define main msglint_main
