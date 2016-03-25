/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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

DomainRfProfileStatus_002::DomainRfProfileStatus_002(UIntN participantIndex, UIntN domainIndex, 
    ParticipantServicesInterface* participantServicesInterface) :
    DomainRfProfileStatusBase(participantIndex, domainIndex, participantServicesInterface)
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

    Frequency centerFrequency = getParticipantServices()->primitiveExecuteGetAsFrequency(
        esif_primitive_type::GET_RFPROFILE_CENTER_FREQUENCY, domainIndex);

    Frequency leftFrequencySpread = getParticipantServices()->primitiveExecuteGetAsFrequency(
        esif_primitive_type::GET_RFPROFILE_FREQUENCY_SPREAD_LEFT, domainIndex);

    Frequency rightFrequencySpread = getParticipantServices()->primitiveExecuteGetAsFrequency(
        esif_primitive_type::GET_RFPROFILE_FREQUENCY_SPREAD_RIGHT, domainIndex);

    UInt32 channelNumber = getParticipantServices()->primitiveExecuteGetAsUInt32(
        esif_primitive_type::GET_RFPROFILE_CHANNEL_NUMBER, domainIndex);

    UInt32 noisePower = getParticipantServices()->primitiveExecuteGetAsUInt32(
        esif_primitive_type::GET_RFPROFILE_NOISE_POWER, domainIndex);

    UInt32 signalToNoiseRatio = getParticipantServices()->primitiveExecuteGetAsUInt32(
        esif_primitive_type::GET_RFPROFILE_SIGNAL_TO_NOISE_RATIO, domainIndex);

    UInt32 rssi = getParticipantServices()->primitiveExecuteGetAsUInt32(
        esif_primitive_type::GET_RFPROFILE_RSSI, domainIndex);

    UInt32 uint32Value = getParticipantServices()->primitiveExecuteGetAsUInt32(
        esif_primitive_type::GET_RFPROFILE_CONNECTION_STATUS, domainIndex);
    RadioConnectionStatus::Type radioConnectionStatus = static_cast<RadioConnectionStatus::Type>(uint32Value);

    UInt32 bitError = getParticipantServices()->primitiveExecuteGetAsUInt32(
        esif_primitive_type::GET_RFPROFILE_BIT_ERROR, domainIndex);

    RfProfileSupplementalData rfProfileSupplementalData(channelNumber, noisePower, signalToNoiseRatio, rssi,
        radioConnectionStatus, bitError);

    RfProfileData rfProfileData(centerFrequency, leftFrequencySpread, rightFrequencySpread, rfProfileSupplementalData);

    return rfProfileData;
}

void DomainRfProfileStatus_002::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
    try
    {
        if (isActivityLoggingEnabled() == true)
        {
            EsifCapabilityData capability;
            capability.type = Capability::RfProfileStatus;
            capability.size = sizeof(capability);

            getParticipantServices()->sendDptfEvent(ParticipantEvent::DptfParticipantControlAction,
                domainIndex, Capability::getEsifDataFromCapabilityData(&capability));
        }
    }
    catch (...)
    {
        // skip if there are any issue in sending log data
    }
}

void DomainRfProfileStatus_002::clearCachedData(void)
{
    // FIXME: do we clear the cache for this control?
}

std::shared_ptr<XmlNode> DomainRfProfileStatus_002::getXml(UIntN domainIndex)
{
    // FIXME
    throw implement_me();
}

std::string DomainRfProfileStatus_002::getName(void)
{
    return "RF Profile Status (Version 2)";
}
