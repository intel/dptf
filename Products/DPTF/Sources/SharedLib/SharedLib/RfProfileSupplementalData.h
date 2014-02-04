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

#pragma once

#include "Dptf.h"
#include "RadioConnectionStatus.h"

class XmlNode;

class RfProfileSupplementalData final
{
public:

    RfProfileSupplementalData(UInt32 channelNumber, UInt32 noisePower, UInt32 signalToNoiseRatio, UInt32 rssi,
        RadioConnectionStatus::Type radioConnectionStatus, UInt32 bitError);

    UInt32 getChannelNumber(void) const;
    UInt32 getNoisePower(void) const;
    UInt32 getSignalToNoiseRatio(void) const;
    UInt32 getRssi(void) const;
    RadioConnectionStatus::Type getRadioConnectionStatus(void) const;
    UInt32 getBitError(void) const;

    Bool operator==(const RfProfileSupplementalData& rhs) const;
    Bool operator!=(const RfProfileSupplementalData& rhs) const;
    XmlNode* getXml(void) const;

private:

    UInt32 m_channelNumber;
    UInt32 m_noisePower;
    UInt32 m_signalToNoiseRatio;
    UInt32 m_rssi;
    RadioConnectionStatus::Type m_radioConnectionStatus;
    UInt32 m_bitError;
};