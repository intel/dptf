/******************************************************************************
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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
#include "DptfManager.h"

class dptf_export SupportedPolicyList
{
public:
	SupportedPolicyList(DptfManagerInterface* dptfManager);

	UIntN getCount(void) const;
	Guid operator[](UIntN index) const;
	Bool isPolicySupported(const Guid& guid) const;
	void update(void);

private:
	DptfManagerInterface* m_dptfManager;
	std::vector<Guid> m_guid;
	Bool isBufferValid(const DptfBuffer& buffer) const;
	std::vector<Guid> parseBufferForPolicyGuids(const DptfBuffer& buffer);
	void postMessageWithSupportedGuids() const;
};
