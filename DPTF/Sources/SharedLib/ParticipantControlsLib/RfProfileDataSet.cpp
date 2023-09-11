/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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

#include "RfProfileDataSet.h"
#include "XmlNode.h"
#include "StatusFormat.h"
#include "esif_sdk_data_misc.h"

using namespace std;
RfProfileDataSet::RfProfileDataSet(const std::vector<RfProfileData>& rfProfileDataSet)
	: m_rfProfileDataSet(rfProfileDataSet)
{
}

RfProfileDataSet::RfProfileDataSet()
{
}

RfProfileDataSet RfProfileDataSet::createActiveRfProfileDataFromDptfBuffer(const DptfBuffer& buffer)
{
	throwIfBufferIsUnexpectedSize(buffer);

	const auto activeChannels = reinterpret_cast<struct ActiveRfChannels_s*>(buffer.get());
	auto numberOfChannels = activeChannels->numberOfChannels;
	std::vector<RfProfileData> rfProfileDataSet;
	Bool fillWithEmptyData = false;
	if (numberOfChannels == 0)
	{
		fillWithEmptyData = true;
	}
	if (numberOfChannels <= MAX_ACTIVE_RF_CHANNELS)
	{
		for (UIntN i = 0; i < numberOfChannels || fillWithEmptyData; i++)
		{
			const auto is5GDevice = activeChannels->rfChannels[i].is5G == 1 ? true : false;
			const auto channelPriority = activeChannels->rfChannels[i].channelPriority;
			const auto centerFrequency = Frequency(activeChannels->rfChannels[i].centerFrequency);
			const auto frequencySpread = Frequency((UInt64)activeChannels->rfChannels[i].frequencySpread);
			const auto connectStatus = activeChannels->rfChannels[i].connectStatus;
			const auto channelNumber = activeChannels->rfChannels[i].channelNumber;
			const auto band = activeChannels->rfChannels[i].band;
			const auto rssi = activeChannels->rfChannels[i].rssi;

			RfProfileSupplementalData rfProfileSupplementalData(RadioConnectionStatus::ToType(connectStatus));
			RfProfileData rfProfileData(
				is5GDevice,
				channelPriority,
				centerFrequency,
				frequencySpread / 2,
				frequencySpread / 2,
				Frequency(0),
				channelNumber,
				band,
				rfProfileSupplementalData,
				rssi);

			rfProfileDataSet.insert(rfProfileDataSet.end(), rfProfileData);
			fillWithEmptyData = false;
		}
	}

	return RfProfileDataSet(rfProfileDataSet);
}

RfProfileDataSet RfProfileDataSet::createActiveRfProfileDataFromEmptyData()
{
	std::vector<RfProfileData> rfProfileDataSet;

	const auto is5GDevice = false;
	const auto channelPriority = 0;
	const auto centerFrequency = Frequency(0);
	const auto frequencySpread = Frequency(0);
	const auto connectStatus = 0;
	const auto channelNumber = Constants::Invalid;
	const auto band = 0;
	const auto rssi = 0;

	RfProfileSupplementalData rfProfileSupplementalData(RadioConnectionStatus::ToType(connectStatus));
	RfProfileData rfProfileData(
		is5GDevice,
		channelPriority,
		centerFrequency,
		frequencySpread / 2,
		frequencySpread / 2,
		Frequency(0),
		channelNumber,
		band,
		rfProfileSupplementalData,
		rssi);

	rfProfileDataSet.insert(rfProfileDataSet.end(), rfProfileData);

	return RfProfileDataSet(rfProfileDataSet);
}

std::vector<RfProfileData> RfProfileDataSet::getRfProfileData() const
{
	return m_rfProfileDataSet;
}

Bool RfProfileDataSet::operator==(const RfProfileDataSet& rhs) const
{
	return (m_rfProfileDataSet == rhs.m_rfProfileDataSet);
}

RfProfileData RfProfileDataSet::operator[](UIntN index) const
{
	return m_rfProfileDataSet.at(index);
}

std::shared_ptr<XmlNode> RfProfileDataSet::getXml()
{
	auto root = XmlNode::createWrapperElement("radio_profile_data_set");

	for (UIntN i = 0; i < m_rfProfileDataSet.size(); i++)
	{
		root->addChild(m_rfProfileDataSet[i].getXml());
	}

	return root;
}

void RfProfileDataSet::throwIfBufferIsUnexpectedSize(const DptfBuffer& buffer)
{
	if (buffer.size() < sizeof(UInt8))
	{
		throw dptf_exception("Received Invalid Active Rf Channel number buffer.");
	}

	const auto activeChannels = reinterpret_cast<struct ActiveRfChannels_s*>(buffer.get());
	auto numberOfChannels = activeChannels->numberOfChannels;

	if (buffer.size() < (sizeof(UInt8) + numberOfChannels * sizeof(RfProfile)))
	{
		throw dptf_exception("Received Invalid Active Rf Channel Info buffer.");
	}

}