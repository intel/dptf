/******************************************************************************
** Copyright (c) 2014 Intel Corporation All Rights Reserved
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

#include "RfProfileSupplementalData.h"
#include "XmlNode.h"
#include "StatusFormat.h"
using namespace StatusFormat;

RfProfileSupplementalData::RfProfileSupplementalData(UInt32 channelNumber, UInt32 noisePower,
    UInt32 signalToNoiseRatio, UInt32 rssi, RadioConnectionStatus::Type radioConnectionStatus, UInt32 bitError) :
    m_channelNumber(channelNumber),
    m_noisePower(noisePower),
    m_signalToNoiseRatio(signalToNoiseRatio),
    m_rssi(rssi),
    m_radioConnectionStatus(radioConnectionStatus),
    m_bitError(bitError)
{
}

UInt32 RfProfileSupplementalData::getChannelNumber(void) const
{
    return m_channelNumber;
}

UInt32 RfProfileSupplementalData::getNoisePower(void) const
{
    return m_noisePower;
}

UInt32 RfProfileSupplementalData::getSignalToNoiseRatio(void) const
{
    return m_signalToNoiseRatio;
}

UInt32 RfProfileSupplementalData::getRssi(void) const
{
    return m_rssi;
}

RadioConnectionStatus::Type RfProfileSupplementalData::getRadioConnectionStatus(void) const
{
    return m_radioConnectionStatus;
}

UInt32 RfProfileSupplementalData::getBitError(void) const
{
    return m_bitError;
}

Bool RfProfileSupplementalData::operator==(const RfProfileSupplementalData& rhs) const
{
    return
        ((m_channelNumber == rhs.m_channelNumber) &&
         (m_noisePower == rhs.m_noisePower) &&
         (m_signalToNoiseRatio == rhs.m_signalToNoiseRatio) &&
         (m_rssi == rhs.m_rssi) &&
         (m_radioConnectionStatus == rhs.m_radioConnectionStatus) &&
         (m_bitError == rhs.m_bitError));
}

Bool RfProfileSupplementalData::operator!=(const RfProfileSupplementalData& rhs) const
{
    return !(*this == rhs);
}

XmlNode* RfProfileSupplementalData::getXml(void) const
{
    XmlNode* supplementalData = XmlNode::createWrapperElement("radio_frequency_supplemental_data");
    supplementalData->addChild(XmlNode::createDataElement("channel_number", friendlyValue(m_channelNumber)));
    supplementalData->addChild(XmlNode::createDataElement("noise_power", friendlyValue(m_noisePower)));
    supplementalData->addChild(XmlNode::createDataElement("signal_to_noise_ratio", friendlyValue(m_signalToNoiseRatio)));
    supplementalData->addChild(XmlNode::createDataElement("rssi", friendlyValue(m_rssi)));
    supplementalData->addChild(XmlNode::createDataElement("radio_connection_status", 
        RadioConnectionStatus::ToString(m_radioConnectionStatus)));
    supplementalData->addChild(XmlNode::createDataElement("bit_error", friendlyValue(m_bitError)));
    return supplementalData;
}