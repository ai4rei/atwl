#include <new>

#include "object.h"
#include "shape.h"
#include "triangle.h"

int main()
{
    CTriangle* T = new CTriangle;

    T->Acquire();
    T->Release();
    T->Release();

    return 0;
}
