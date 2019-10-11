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
#include "CachedValue.h"
#include "DomainType.h"

class dptf_export UserPreferredCache
{
public:
	UserPreferredCache();
	~UserPreferredCache();

	// Display Cache
	UIntN getUserPreferredDisplayCacheValue(std::string participantScope, DomainType::Type domainType);
	void setUserPreferredDisplayCacheValue(
		std::string participantScope,
		DomainType::Type domainType,
		UIntN userPreferredDisplayIndex);
	void invalidateUserPreferredDisplayCache(std::string participantScope, DomainType::Type domainType);
	Bool isUserPreferredDisplayCacheValid(std::string participantScope, DomainType::Type domainType);

private:
	std::map<std::pair<std::string, DomainType::Type>, CachedValue<UIntN>> m_userPreferredDisplayCacheMap;
};
