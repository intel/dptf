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
#include "EsifDataBinaryRfProfileDataPackage.h"

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
	struct EsifDataBinaryRfProfileDataPackage* currentRow =
		reinterpret_cast<struct EsifDataBinaryRfProfileDataPackage*>(data);

	if (buffer.size() == 0)
	{
		throw dptf_exception("Received empty PPSS buffer.");
	}

	UIntN rows = buffer.size() / sizeof(EsifDataBinaryRfProfileDataPackage);
	data = reinterpret_cast<UInt8*>(buffer.get());
	currentRow = reinterpret_cast<struct EsifDataBinaryRfProfileDataPackage*>(data);

	for (UIntN i = 0; i < rows; i++)
	{
		auto centerFrequency = Frequency(static_cast<UInt32>(currentRow->centerFrequency.integer.value));
		auto leftFrequencySpread = Frequency(static_cast<UInt32>(currentRow->leftFrequencySpread.integer.value));
		auto rightFrequencySpread = Frequency(static_cast<UInt32>(currentRow->rightFrequencySpread.integer.value));
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
			centerFrequency, leftFrequencySpread, rightFrequencySpread, rfProfileSupplementalData);

		rfProfileDataSet.insert(rfProfileDataSet.end(), rfProfileData);

		data += sizeof(struct EsifDataBinaryRfProfileDataPackage);
		currentRow = reinterpret_cast<struct EsifDataBinaryRfProfileDataPackage*>(data);
	}

	return RfProfileDataSet(rfProfileDataSet);
}

DptfBuffer RfProfileDataSet::toRfProfileDataBinary() const
{
	vector<EsifDataBinaryRfProfileDataPackage> packages;

	for (auto rfProfileData = m_rfProfileDataSet.begin(); rfProfileData != m_rfProfileDataSet.end(); rfProfileData++)
	{
		EsifDataBinaryRfProfileDataPackage entryPackage;

		// Center Frequency
		entryPackage.centerFrequency.integer.type = esif_data_type::ESIF_DATA_UINT64;
		entryPackage.centerFrequency.integer.value = (UInt64)((*rfProfileData).getCenterFrequency());

		// Left Frequency Spread
		entryPackage.leftFrequencySpread.integer.type = esif_data_type::ESIF_DATA_UINT64;
		entryPackage.leftFrequencySpread.integer.value = (UInt64)((*rfProfileData).getLeftFrequencySpread());

		// Right Frequency Spread
		entryPackage.rightFrequencySpread.integer.type = esif_data_type::ESIF_DATA_UINT64;
		entryPackage.rightFrequencySpread.integer.value = (UInt64)((*rfProfileData).getRightFrequencySpread());

		// Channel Number
		entryPackage.channelNumber.integer.type = esif_data_type::ESIF_DATA_UINT64;
		entryPackage.channelNumber.integer.value = (UInt64)((*rfProfileData).getSupplementalData().getChannelNumber());

		// Noise Power
		entryPackage.noisePower.integer.type = esif_data_type::ESIF_DATA_UINT64;
		entryPackage.noisePower.integer.value = (UInt64)((*rfProfileData).getSupplementalData().getNoisePower());

		// signalToNoiseRatio
		entryPackage.signalToNoiseRatio.integer.type = esif_data_type::ESIF_DATA_UINT64;
		entryPackage.signalToNoiseRatio.integer.value =
			(UInt64)((*rfProfileData).getSupplementalData().getSignalToNoiseRatio());

		// rssi
		entryPackage.rssi.integer.type = esif_data_type::ESIF_DATA_UINT64;
		entryPackage.rssi.integer.value = (UInt64)((*rfProfileData).getSupplementalData().getRssi());

		// uint32Value
		entryPackage.uint32Value.integer.type = esif_data_type::ESIF_DATA_UINT64;
		entryPackage.uint32Value.integer.value =
			(UInt64)((*rfProfileData).getSupplementalData().getRadioConnectionStatus());

		// bitError
		entryPackage.bitError.integer.type = esif_data_type::ESIF_DATA_UINT64;
		entryPackage.bitError.integer.value = (UInt64)((*rfProfileData).getSupplementalData().getBitError());

		packages.push_back(entryPackage);
	}

	UInt32 sizeOfPackages = (UInt32)packages.size() * sizeof(EsifDataBinaryRfProfileDataPackage);
	DptfBuffer buffer(sizeOfPackages);
	buffer.put(0, (UInt8*)packages.data(), sizeOfPackages);
	return buffer;
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