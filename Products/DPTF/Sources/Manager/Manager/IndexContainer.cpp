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

#include "IndexContainer.h"
#include "EsifMutexHelper.h"

// Do not throw exceptions from within this file

IndexContainer::IndexContainer(UIntN initialCount)
{
    for (UIntN i = 0; i < initialCount; i++)
    {
        getIndexPtr(i);
    }
}

IndexContainer::~IndexContainer(void)
{
    EsifMutexHelper esifMutexHelper(&m_mutex);
    esifMutexHelper.lock();

    for (UIntN i = 0; i < m_vectorIndexStructPtr.size(); i++)
    {
        DELETE_MEMORY_TC(m_vectorIndexStructPtr[i]);
    }

    m_vectorIndexStructPtr.clear();

    esifMutexHelper.unlock();
}

IndexStructPtr IndexContainer::getIndexPtr(UIntN index)
{
    IndexStructPtr indexPtr = nullptr;
    UInt64 currentVectorSize = m_vectorIndexStructPtr.size();

    EsifMutexHelper esifMutexHelper(&m_mutex);
    esifMutexHelper.lock();

    if ((index == Constants::Esif::NoParticipant) ||
        (index == Constants::Esif::NoDomain) ||
        (index == Constants::Invalid) ||
        (index > currentVectorSize))
    {
        indexPtr = nullptr;
    }
    else if (index < currentVectorSize)
    {
        indexPtr = m_vectorIndexStructPtr[index];
    }
    else if (index == currentVectorSize)
    {
        IndexStructPtr indexStructPtr = new IndexStruct;
        indexStructPtr->index = index;
        m_vectorIndexStructPtr.push_back(indexStructPtr);
        indexPtr = m_vectorIndexStructPtr[index];
    }

    esifMutexHelper.unlock();

    return indexPtr;
}

UIntN IndexContainer::getIndex(IndexStructPtr indexStructPtr)
{
    //FIXME:  consider using a hash table.  However, the number of items in the vector will be short as it will
    //        be the number of participants loaded.  It may not be worth the conversion.  Should run
    //        performance tests before changing.

    UIntN index = Constants::Invalid;

    EsifMutexHelper esifMutexHelper(&m_mutex);
    esifMutexHelper.lock();

    if (indexStructPtr != nullptr)
    {
        UInt64 currentVectorSize = m_vectorIndexStructPtr.size();
        for (UIntN i = 0; i < currentVectorSize; i++)
        {
            if (m_vectorIndexStructPtr[i] == indexStructPtr)
            {
                index = i;
                break;
            }
        }
    }

    esifMutexHelper.unlock();

    return index;
}