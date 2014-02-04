/******************************************************************************
** Copyright (c) 2014 Intel Corporation All Rights Reserved
**
** Licensed under the Apache License, Version 2.0 (the "License"); you may not
** use this file except in compliance with the License.
**
** You may obtain a copy of the License at
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
** WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**
** See the License for the specific language governing permissions and
** limitations under the License.
**
******************************************************************************/

#include "DptfMemory.h"
#include "esif_ccb_memory.h"

DptfMemory::DptfMemory(void)
{
    initialize();
}

DptfMemory::DptfMemory(UIntN sizeBytes)
{
    initialize();
    allocate(sizeBytes, true);
}

DptfMemory::DptfMemory(UIntN sizeBytes, Bool automaticallyFreeOnDestruction)
{
    initialize();
    allocate(sizeBytes, automaticallyFreeOnDestruction);
}

DptfMemory::~DptfMemory(void)
{
    if (m_automaticallyFreeOnDestruction == true)
    {
        deallocate();
    }
}

void* DptfMemory::allocate(UIntN sizeBytes, Bool automaticallyFreeOnDestruction)
{
    if (m_memoryPtr != nullptr)
    {
        throw dptf_exception("Memory has already been allocated.");
    }

    if (sizeBytes == 0)
    {
        throw dptf_exception("Cannot allocate 0 bytes.");
    }

    m_memoryPtr = esif_ccb_malloc(sizeBytes);
    if (m_memoryPtr == nullptr)
    {
        std::stringstream stream;
        stream << "Failed to allocate [" << sizeBytes << "] bytes of memory.";
        throw memory_allocation_failure(stream.str());
    }

    m_sizeBytes = sizeBytes;
    m_automaticallyFreeOnDestruction = automaticallyFreeOnDestruction;

    return m_memoryPtr;
}

void DptfMemory::deallocate(void)
{
    if (m_memoryPtr != nullptr)
    {
        esif_ccb_free(m_memoryPtr);
        initialize();
    }
}

void DptfMemory::initialize(void)
{
    m_memoryPtr = nullptr;
    m_sizeBytes = 0;
    m_automaticallyFreeOnDestruction = false;
}

void* DptfMemory::getPtr(void) const
{
    if (m_memoryPtr == nullptr)
    {
        throw dptf_exception("Memory has not been allocated.");
    }

    return m_memoryPtr;
}

UIntN DptfMemory::getSize(void) const
{
    if (m_memoryPtr == nullptr)
    {
        throw dptf_exception("Memory has not been allocated.");
    }

    return m_sizeBytes;
}

DptfMemory::operator void*(void)
{
    return getPtr();
}