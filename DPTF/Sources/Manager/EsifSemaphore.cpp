/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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

#include "EsifSemaphore.h"

EsifSemaphore::EsifSemaphore(void)
{
	m_semaphore = new esif_ccb_sem_t;
	esif_ccb_sem_init(m_semaphore);
}

EsifSemaphore::~EsifSemaphore(void)
{
	esif_ccb_sem_uninit(m_semaphore);
	DELETE_MEMORY_TC(m_semaphore);
}

void EsifSemaphore::wait(void)
{
	esif_ccb_sem_down(m_semaphore);
}

void EsifSemaphore::signal(void)
{
	esif_ccb_sem_up(m_semaphore);
}
