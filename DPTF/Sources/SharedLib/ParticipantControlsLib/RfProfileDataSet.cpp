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

RfProfileDataSet RfProfileDataSet::createRfProfileDataFromDptfBuffer(const DptfBuffer& buffer)
{
	std::vector<RfProfileData> rfProfileDataSet;
	RadioConnectionStatus::Type connectionStatus = RadioConnectionStatus::NotConnected;
	UInt8* data = reinterpret_cast<UInt8*>(buffer.get());
	struct esif_data_rfprofile* currentRow =
		reinterpret_cast<struct esif_data_rfprofile*>(data);

	if (buffer.size() == 0)
	{
		throw dptf_exception("Received empty PPSS buffer.");
	}

	UIntN rows = buffer.size() / sizeof(esif_data_rfprofile);

	for (UIntN i = 0; i < rows; i++)
	{
		auto centerFrequency = Frequency(static_cast<UInt64>(currentRow->centerFrequency.integer.value));
		auto frequencySpread = Frequency(static_cast<UInt64>(currentRow->frequencySpread.integer.value));
		auto channelNumber = static_cast<UInt32>(currentRow->channelNumber.integer.value);
		auto noisePower = static_cast<UInt32>(currentRow->noisePower.integer.value);
		auto signalToNoiseRatio = static_cast<UInt32>(currentRow->signalToNoiseRatio.integer.value);
		auto rssi = static_cast<UInt32>(currentRow->rssi.integer.value);
		auto uint32Value = static_cast<UInt32>(currentRow->uint32Value.integer.value);
		auto bitError = static_cast<UInt32>(currentRow->bitError.integer.value);
		if (RadioConnectionStatus::Connected == uint32Value)
		{
			connectionStatus = RadioConnectionStatus::Connected;
		}

		RfProfileSupplementalData rfProfileSupplementalData(
			channelNumber, noisePower, signalToNoiseRatio, rssi, connectionStatus, bitError);
		RfProfileData rfProfileData(
			centerFrequency, frequencySpread / 2, frequencySpread / 2, rfProfileSupplementalData);

		rfProfileDataSet.insert(rfProfileDataSet.end(), rfProfileData);

		data += sizeof(struct esif_data_rfprofile);
		currentRow = reinterpret_cast<struct esif_data_rfprofile*>(data);
	}

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
