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

#include "UserPreferredCache.h"

UserPreferredCache::UserPreferredCache()
	: m_userPreferredDisplayCacheMap()
{
}

UserPreferredCache::~UserPreferredCache()
{
}

UIntN UserPreferredCache::getUserPreferredDisplayCacheValue(std::string participantScope, DomainType::Type domainType)
{
	std::pair<std::string, DomainType::Type> participantScopeDomainType;
	participantScopeDomainType.first = participantScope;
	participantScopeDomainType.second = domainType;

	auto userPreferredDisplayValue = m_userPreferredDisplayCacheMap.find(participantScopeDomainType);
	if (userPreferredDisplayValue != m_userPreferredDisplayCacheMap.end())
	{
		return m_userPreferredDisplayCacheMap[participantScopeDomainType].get();
	}

	throw dptf_exception("User Preferred Display Cache Value not found in map");
}

void UserPreferredCache::setUserPreferredDisplayCacheValue(
	std::string participantScope,
	DomainType::Type domainType,
	UIntN userPreferredDisplayIndex)
{
	CachedValue<UIntN> userPreferredCacheValue;
	std::pair<std::string, DomainType::Type> participantScopeDomainType;
	participantScopeDomainType.first = participantScope;
	participantScopeDomainType.second = domainType;

	auto userPreferredDisplayValue = m_userPreferredDisplayCacheMap.find(participantScopeDomainType);
	if (userPreferredDisplayValue != m_userPreferredDisplayCacheMap.end())
	{
		m_userPreferredDisplayCacheMap[participantScopeDomainType].set(userPreferredDisplayIndex);
	}
	else
	{
		userPreferredCacheValue.set(userPreferredDisplayIndex);
		m_userPreferredDisplayCacheMap.insert(std::make_pair(participantScopeDomainType, userPreferredCacheValue));
	}
}

void UserPreferredCache::invalidateUserPreferredDisplayCache(std::string participantScope, DomainType::Type domainType)
{
	std::pair<std::string, DomainType::Type> participantScopeDomainType;
	participantScopeDomainType.first = participantScope;
	participantScopeDomainType.second = domainType;

	auto userPreferredDisplayCacheValue = m_userPreferredDisplayCacheMap.find(participantScopeDomainType);
	if (userPreferredDisplayCacheValue != m_userPreferredDisplayCacheMap.end())
	{
		m_userPreferredDisplayCacheMap[participantScopeDomainType].invalidate();
	}
	else
	{
		throw dptf_exception("User Preferred Display Cache Value not found in map");
	}
}

Bool UserPreferredCache::isUserPreferredDisplayCacheValid(std::string participantScope, DomainType::Type domainType)
{
	std::pair<std::string, DomainType::Type> participantScopeDomainType;
	participantScopeDomainType.first = participantScope;
	participantScopeDomainType.second = domainType;

	auto userPreferredDisplayCacheValue = m_userPreferredDisplayCacheMap.find(participantScopeDomainType);

	if (userPreferredDisplayCacheValue != m_userPreferredDisplayCacheMap.end())
	{
		return m_userPreferredDisplayCacheMap[participantScopeDomainType].isValid();
	}
	else
	{
		return false;
	}
}