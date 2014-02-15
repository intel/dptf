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

#include "DomainRfProfileStatus_002.h"

//
// version 002 is for wireless
//

DomainRfProfileStatus_002::DomainRfProfileStatus_002(ParticipantServicesInterface* participantServicesInterface) :
    m_participantServicesInterface(participantServicesInterface)
{
}

DomainRfProfileStatus_002::~DomainRfProfileStatus_002(void)
{
}

RfProfileData DomainRfProfileStatus_002::getRfProfileData(UIntN participantIndex, UIntN domainIndex)
{
    // FIXME:  can any of this be cached?  If so how do we know when the data changes?

    //
    // If any of these primitive calls fail we allow the exception to throw back to the requesting policy.
    //

    Frequency centerFrequency = m_participantServicesInterface->primitiveExecuteGetAsFrequency(
        esif_primitive_type::GET_RFPROFILE_CENTER_FREQUENCY, domainIndex);

    Frequency leftFrequencySpread = m_participantServicesInterface->primitiveExecuteGetAsFrequency(
        esif_primitive_type::GET_RFPROFILE_FREQUENCY_SPREAD_LEFT, domainIndex);

    Frequency rightFrequencySpread = m_participantServicesInterface->primitiveExecuteGetAsFrequency(
        esif_primitive_type::GET_RFPROFILE_FREQUENCY_SPREAD_RIGHT, domainIndex);

    UInt32 channelNumber = m_participantServicesInterface->primitiveExecuteGetAsUInt32(
        esif_primitive_type::GET_RFPROFILE_CHANNEL_NUMBER, domainIndex);

    UInt32 noisePower = m_participantServicesInterface->primitiveExecuteGetAsUInt32(
        esif_primitive_type::GET_RFPROFILE_NOISE_POWER, domainIndex);

    UInt32 signalToNoiseRatio = m_participantServicesInterface->primitiveExecuteGetAsUInt32(
        esif_primitive_type::GET_RFPROFILE_SIGNAL_TO_NOISE_RATIO, domainIndex);

    UInt32 rssi = m_participantServicesInterface->primitiveExecuteGetAsUInt32(
        esif_primitive_type::GET_RFPROFILE_RSSI, domainIndex);

    UInt8 uint8Value = m_participantServicesInterface->primitiveExecuteGetAsUInt8(
        esif_primitive_type::GET_RFPROFILE_CONNECTION_STATUS, domainIndex);
    RadioConnectionStatus::Type radioConnectionStatus = static_cast<RadioConnectionStatus::Type>(uint8Value);

    UInt32 bitError = m_participantServicesInterface->primitiveExecuteGetAsUInt32(
        esif_primitive_type::GET_RFPROFILE_BIT_ERROR, domainIndex);

    RfProfileSupplementalData rfProfileSupplementalData(channelNumber, noisePower, signalToNoiseRatio, rssi,
        radioConnectionStatus, bitError);

    RfProfileData rfProfileData(centerFrequency, leftFrequencySpread, rightFrequencySpread, rfProfileSupplementalData);

    return rfProfileData;
}

void DomainRfProfileStatus_002::clearCachedData(void)
{
    // For now nothing is cached
}

XmlNode* DomainRfProfileStatus_002::getXml(UIntN domainIndex)
{
    // FIXME
    throw implement_me();
}