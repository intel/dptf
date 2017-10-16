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

#include "RfProfileCapabilities.h"

RfProfileCapabilities::RfProfileCapabilities(
	Frequency defaultCenterFrequency,
	Frequency centerFrequency,
	Frequency frequencyAdjustResolution,
	Frequency minFrequency,
	Frequency maxFrequency,
	Percentage ssc)
	: m_defaultCenterFrequency(defaultCenterFrequency)
	, m_centerFrequency(centerFrequency)
	, m_frequencyAdjustResolution(frequencyAdjustResolution)
	, m_minFrequency(minFrequency)
	, m_maxFrequency(maxFrequency)
	, m_ssc(ssc)
{
}

Frequency RfProfileCapabilities::getDefaultCenterFrequency(void) const
{
	return m_defaultCenterFrequency;
}

Frequency RfProfileCapabilities::getCenterFrequency(void) const
{
	return m_centerFrequency;
}

Frequency RfProfileCapabilities::getFrequencyAdjustResolution(void) const
{
	return m_frequencyAdjustResolution;
}

Frequency RfProfileCapabilities::getMinFrequency(void) const
{
	return m_minFrequency;
}

Frequency RfProfileCapabilities::getMaxFrequency(void) const
{
	return m_maxFrequency;
}

Percentage RfProfileCapabilities::getSsc(void) const
{
	return m_ssc;
}

Bool RfProfileCapabilities::operator==(const RfProfileCapabilities& rhs) const
{
	return (
		(m_defaultCenterFrequency == rhs.m_defaultCenterFrequency) && (m_centerFrequency == rhs.m_centerFrequency)
		&& (m_frequencyAdjustResolution == rhs.m_frequencyAdjustResolution) && (m_minFrequency == rhs.m_minFrequency) 
		&& (m_maxFrequency == rhs.m_maxFrequency) && (m_ssc == rhs.m_ssc));
}

Bool RfProfileCapabilities::operator!=(const RfProfileCapabilities& rhs) const
{
	return !(*this == rhs);
}

std::shared_ptr<XmlNode> RfProfileCapabilities::getXml(void)
{
	// FIXME
	throw implement_me();
}
