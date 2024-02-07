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

#include "DttConfigurationQuery.h"
#include <sstream>
using namespace std;

DttConfigurationQuery::DttConfigurationQuery(const string& regexString)
	: m_regularExpression(regexString)
{
}

DttConfigurationQuery::DttConfigurationQuery(const list<string>& regexCollection)
{
	stringstream stream;
	for (const auto& r : regexCollection)
	{
		stream << r;
	}
	m_regularExpression = stream.str();
}

regex DttConfigurationQuery::toRegex() const
{
	return regex(m_regularExpression);
}

bool DttConfigurationQuery::operator==(const DttConfigurationQuery& other) const
{
	return m_regularExpression == other.m_regularExpression;
}

bool DttConfigurationQuery::operator<(const DttConfigurationQuery& other) const
{
	return m_regularExpression < other.m_regularExpression;
}

const 	string& DttConfigurationQuery::toString() const
{
	return m_regularExpression;
}