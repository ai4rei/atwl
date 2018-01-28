#include <cstdio>

#include "object.h"

CObject::CObject()
    : m_ulReferences(1UL)
{
    printf("CObject::CObject\n");
}

CObject::~CObject()
{
    printf("CObject::~CObject\n");
}

void CObject::Acquire()
{
    printf("CObject::Acquire\n");

    m_ulReferences++;
}

void CObject::Release()
{
    printf("CObject::Release\n");

    m_ulReferences--;

    if(!m_ulReferences)
    {
        delete this;
    }
}
