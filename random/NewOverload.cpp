#include <cstdio>
#include <cstdlib>

struct CTest
{
    CTest()
    {
        puts("CTest");
    }

    ~CTest()
    {
        puts("~CTest");
    }

    void* operator new(size_t uSize)
    {
        puts("operator new - Enter");

        printf("%u, %u\n", uSize, sizeof(CTest));

        void* lpMem = malloc(uSize);

        puts("operator new - Leave");

        return lpMem;
    }

    void operator delete(void* lpPtr)
    {
        puts("operator delete - Enter");

        free(lpPtr);

        puts("operator delete - Leave");
    }
};

struct CTestNull
{
    void* operator new(size_t uSize)
    {
        puts("operator new");

        return NULL;
    }

    void operator delete(void* lpPtr)
    {
        puts("operator delete");
    }
};

int main()
{
    puts("On Stack - Enter");
    {
        CTest Test;
    }
    puts("On Stack - Leave");

    puts("In Memory - Enter");
    {
        CTest* lpTest = new CTest;

        delete lpTest;
    }
    puts("In Memory - Leave");

    puts("NULL Memory - Enter");
    {
        CTestNull* lpTest = new CTestNull;

        if(lpTest)
        {
            delete lpTest;
        }
        else
        {
            puts("Is NULL");
        }
    }
    puts("NULL Memory - Leave");

    return 0;
}
