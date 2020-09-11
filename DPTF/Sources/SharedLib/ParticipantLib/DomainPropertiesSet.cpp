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

#include "DomainPropertiesSet.h"

DomainPropertiesSet::DomainPropertiesSet(std::vector<DomainProperties> domainProperties)
	: m_domainProperties(domainProperties)
{
}

DomainPropertiesSet::DomainPropertiesSet(DomainProperties domainProperties)
{
	m_domainProperties.push_back(domainProperties);
}

DomainProperties DomainPropertiesSet::getDomainProperties(UIntN domainIndex) const
{
	for (auto properties = m_domainProperties.begin(); properties != m_domainProperties.end(); properties++)
	{
		if (properties->getDomainIndex() == domainIndex)
		{
			return *properties;
		}
	}

	throw dptf_exception("Domain properties for domain index of " + std::to_string(domainIndex) + " does not exist.");
}

UIntN DomainPropertiesSet::getDomainCount(void) const
{
	return static_cast<UIntN>(m_domainProperties.size());
}

const DomainProperties DomainPropertiesSet::operator[](UIntN index) const
{
	return m_domainProperties.at(index);
}

UIntN DomainPropertiesSet::getDomainIndexFromDomainType(DomainType::Type domainType) const
{
	for (UIntN index = 0; index < m_domainProperties.size(); index++)
	{
		if (m_domainProperties[index].getDomainType() == domainType)
		{
			return index;
		}
	}

	throw dptf_exception("No domain of type " + DomainType::toString(domainType) + ".");
}
