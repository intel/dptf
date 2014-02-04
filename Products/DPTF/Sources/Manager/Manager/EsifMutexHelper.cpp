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

#include "EsifMutexHelper.h"

EsifMutexHelper::EsifMutexHelper(EsifMutex* esifMutex) : m_esifMutex(esifMutex), m_mutexLocked(false)
{
}

EsifMutexHelper::~EsifMutexHelper(void)
{
    if (m_mutexLocked == true)
    {
        unlock();
    }
}

void EsifMutexHelper::lock(void)
{
    m_esifMutex->lock();
    m_mutexLocked = true;
}

void EsifMutexHelper::unlock(void)
{
    m_esifMutex->unlock();
    m_mutexLocked = false;
}