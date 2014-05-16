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

#include "EsifMutex.h"

EsifMutex::EsifMutex(void)
{
    esif_ccb_mutex_init(&m_mutex);
}

EsifMutex::~EsifMutex(void)
{
    esif_ccb_mutex_uninit(&m_mutex);
}

void EsifMutex::lock(void)
{
    esif_ccb_mutex_lock(&m_mutex);
}

void EsifMutex::unlock(void)
{
    esif_ccb_mutex_unlock(&m_mutex);
}