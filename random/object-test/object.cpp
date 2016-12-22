#include <cstdio>

#include "object.h"

CObject::CObject()
{
    printf("CObject::CObject\n");

    this->m_ulReferences = 1UL;
}

CObject::~CObject()
{
    printf("CObject::~CObject\n");
}

void CObject::Acquire()
{
    printf("CObject::Acquire\n");

    this->m_ulReferences++;
}

void CObject::Release()
{
    printf("CObject::Release\n");

    this->m_ulReferences--;
    
    if(!this->m_ulReferences)
    {
        delete this;
    }
}
