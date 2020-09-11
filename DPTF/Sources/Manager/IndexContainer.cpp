/******************************************************************************
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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

IndexContainer::IndexContainer()
{
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

void IndexContainer::insertHandle(UIntN participantIndex,
	UIntN domainIndex,
	esif_handle_t participantHandle,
	esif_handle_t domainHandle
	)
{
	EsifMutexHelper esifMutexHelper(&m_mutex);
	esifMutexHelper.lock();

	IndexStructPtr indexStructPtr = new IndexStruct;

	indexStructPtr->participantIndex = participantIndex;
	indexStructPtr->domainIndex = domainIndex;
	indexStructPtr->participantHandle = participantHandle;
	indexStructPtr->domainHandle = domainHandle;

	m_vectorIndexStructPtr.push_back(indexStructPtr);

	esifMutexHelper.unlock();
}

void IndexContainer::removeHandle(esif_handle_t participantHandle, esif_handle_t domainHandle)
{
	EsifMutexHelper esifMutexHelper(&m_mutex);
	esifMutexHelper.lock();

	for (UIntN i = 0; i < m_vectorIndexStructPtr.size(); i++)
	{
		if ((m_vectorIndexStructPtr[i]->participantHandle == participantHandle) &&
			(m_vectorIndexStructPtr[i]->domainHandle == domainHandle))
		{
			DELETE_MEMORY_TC(m_vectorIndexStructPtr[i]);
			m_vectorIndexStructPtr.erase(m_vectorIndexStructPtr.begin() + i);
			break;
		}
	}

	esifMutexHelper.unlock();
}

esif_handle_t IndexContainer::getParticipantHandle(UIntN participantIndex)
{
	esif_handle_t participantHandle = ESIF_INVALID_HANDLE;
	UInt64 currentVectorSize = 0;

	EsifMutexHelper esifMutexHelper(&m_mutex);
	esifMutexHelper.lock();

	currentVectorSize = m_vectorIndexStructPtr.size();
	for (UIntN i = 0; i < currentVectorSize; i++)
	{
		if (m_vectorIndexStructPtr[i]->participantIndex == participantIndex)
		{
			participantHandle = m_vectorIndexStructPtr[i]->participantHandle;
			break;
		}
	}
	esifMutexHelper.unlock();

	return participantHandle;
}

esif_handle_t IndexContainer::getDomainHandle(UIntN participantIndex, UIntN domainIndex)
{
	esif_handle_t domainHandle = ESIF_INVALID_HANDLE;
	UInt64 currentVectorSize = 0;

	EsifMutexHelper esifMutexHelper(&m_mutex);
	esifMutexHelper.lock();

	currentVectorSize = m_vectorIndexStructPtr.size();

	for (UIntN i = 0; i < currentVectorSize; i++)
	{
		if ((m_vectorIndexStructPtr[i]->participantIndex == participantIndex) &&
			(m_vectorIndexStructPtr[i]->domainIndex == domainIndex))
		{
			domainHandle = m_vectorIndexStructPtr[i]->domainHandle;
			break;
		}
	}
	esifMutexHelper.unlock();

	return domainHandle;
}

UIntN IndexContainer::getParticipantIndex(esif_handle_t participantHandle)
{
	UIntN participantIndex = Constants::Invalid;
	UInt64 currentVectorSize = 0;

	EsifMutexHelper esifMutexHelper(&m_mutex);
	esifMutexHelper.lock();

	currentVectorSize = m_vectorIndexStructPtr.size();

	for (UIntN i = 0; i < currentVectorSize; i++)
	{
		if (m_vectorIndexStructPtr[i]->participantHandle == participantHandle)
		{
			participantIndex = m_vectorIndexStructPtr[i]->participantIndex;
			break;
		}
	}
	esifMutexHelper.unlock();

	return participantIndex;
}

UIntN IndexContainer::getDomainIndex(esif_handle_t participantHandle, esif_handle_t domainHandle)
{
	UIntN domainIndex = Constants::Invalid;
	UInt64 currentVectorSize = 0;

	EsifMutexHelper esifMutexHelper(&m_mutex);
	esifMutexHelper.lock();

	currentVectorSize = m_vectorIndexStructPtr.size();

	for (UIntN i = 0; i < currentVectorSize; i++)
	{
		if ((m_vectorIndexStructPtr[i]->participantHandle == participantHandle) &&
			(m_vectorIndexStructPtr[i]->domainHandle == domainHandle))
		{
			domainIndex = m_vectorIndexStructPtr[i]->domainIndex;
			break;
		}
	}
	esifMutexHelper.unlock();

	return domainIndex;
}