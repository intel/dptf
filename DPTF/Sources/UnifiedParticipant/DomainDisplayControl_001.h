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

#pragma once

#include "Dptf.h"
#include "DomainDisplayControlBase.h"
#include "CachedValue.h"

//
// Implements the regular display controls, v001.
//

class DomainDisplayControl_001 : public DomainDisplayControlBase
{
public:

    DomainDisplayControl_001(UIntN participantIndex, UIntN domainIndex, 
        ParticipantServicesInterface* participantServicesInterface);
    virtual ~DomainDisplayControl_001(void);

    virtual DisplayControlDynamicCaps getDisplayControlDynamicCaps(UIntN participantIndex, UIntN domainIndex) override;
    virtual DisplayControlStatus getDisplayControlStatus(UIntN participantIndex, UIntN domainIndex) override;
    virtual UIntN getUserPreferredDisplayIndex(UIntN participantIndex, UIntN domainIndex) override;
    virtual Bool isUserPreferredIndexModified(UIntN participantIndex, UIntN domainIndex) override;
    virtual DisplayControlSet getDisplayControlSet(UIntN participantIndex, UIntN domainIndex) override;
    virtual void setDisplayControl(UIntN participantIndex, UIntN domainIndex, UIntN displayControlIndex) override;
    virtual void setDisplayControlDynamicCaps(UIntN participantIndex, UIntN domainIndex, 
        DisplayControlDynamicCaps newCapabilities) override;
    virtual void setDisplayCapsLock(UIntN participantIndex, UIntN domainIndex, Bool lock) override;

    // ParticipantActivityLoggingInterface
    virtual void sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex) override;

    // ComponentExtendedInterface
    virtual void clearCachedData(void) override;
    virtual std::string getName(void) override;
    virtual std::shared_ptr<XmlNode> getXml(UIntN domainIndex) override;

protected:
    virtual void restore(void) override;

private:

    // hide the copy constructor and = operator
    DomainDisplayControl_001(const DomainDisplayControl_001& rhs);
    DomainDisplayControl_001& operator=(const DomainDisplayControl_001& rhs);

    // Functions
    DisplayControlDynamicCaps createDisplayControlDynamicCaps(UIntN domainIndex);
    DisplayControlSet createDisplayControlSet(UIntN domainIndex);
    void throwIfControlIndexIsOutOfRange(UIntN displayControlIndex, UIntN domainIndex);
    void throwIfDisplaySetIsEmpty(UIntN sizeOfSet);
    UIntN getLowerLimitIndex(UIntN domainIndex, DisplayControlSet displaySet);
    UIntN getUpperLimitIndex(UIntN domainIndex, DisplayControlSet displaySet);
    UIntN getAllowableDisplayBrightnessIndex(UIntN participantIndex, UIntN domainIndex, UIntN requestedIndex);

    // Vars (external)
    CachedValue<DisplayControlDynamicCaps> m_displayControlDynamicCaps;
    CachedValue<DisplayControlSet> m_displayControlSet;

    // Vars (internal)
    CachedValue<UIntN> m_currentDisplayControlIndex;
    UIntN m_userPreferredIndex;
    UIntN m_lastSetDisplayBrightness;
    Bool m_isUserPreferredIndexModified;
    Bool m_capabilitiesLocked;
};