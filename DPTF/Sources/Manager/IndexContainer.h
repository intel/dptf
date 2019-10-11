/******************************************************************************
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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

#pragma once

#include "Dptf.h"
#include "EsifMutex.h"
#include "IndexContainerInterface.h"

class IndexContainer : public IndexContainerInterface
{
public:
	IndexContainer();
	~IndexContainer(void);

	virtual void insertHandle(UIntN participantIndex,
		UIntN domainIndex,
		esif_handle_t participantHandle,
		esif_handle_t domainHandle) override;

	virtual void removeHandle(esif_handle_t participantHandle, esif_handle_t domainHandle) override;

	virtual esif_handle_t getParticipantHandle(UIntN participantIndex) override;
	virtual UIntN getParticipantIndex(esif_handle_t participantHandle) override;

	virtual esif_handle_t getDomainHandle(UIntN participantIndex, UIntN domainIndex) override;
	virtual UIntN getDomainIndex(esif_handle_t participantHandle, esif_handle_t domainHandle) override;
private:
	// hide the copy constructor and assignment operator.
	IndexContainer(const IndexContainer& rhs);
	IndexContainer& operator=(const IndexContainer& rhs);

	EsifMutex m_mutex;

	std::vector<IndexStructPtr> m_vectorIndexStructPtr;
};
