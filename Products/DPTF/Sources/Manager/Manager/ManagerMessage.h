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

#include "DptfMessage.h"

class DptfManager;

class ManagerMessage : public DptfMessage
{
public:

    ManagerMessage(const DptfManager* dptfManager, const std::string& fileName, UIntN lineNumber,
        const std::string& executingFunctionName);
    ManagerMessage(const DptfManager* dptfManager, const std::string& fileName, UIntN lineNumber,
        const std::string& executingFunctionName, const std::string& message);
    ManagerMessage(const DptfManager* dptfManager, const DptfMessage& dptfMessage);
    virtual ~ManagerMessage(void);

    void setFrameworkEvent(FrameworkEvent::Type frameworkEvent);
    void setParticipantIndex(UIntN participantIndex);
    void setParticipantAndDomainIndex(UIntN participantIndex, UIntN domainIndex);
    void setPolicyIndex(UIntN policyIndex);
    void setEsifPrimitive(esif_primitive_type primitive, UInt32 instance);
    void setEsifEventGuid(const Guid& esifEventGuid);
    void setEsifErrorCode(eEsifError esifErrorCode);

    // Allows ManagerMessage to be used anywhere a std::string is required
    operator std::string(void) const;

private:

    const DptfManager* m_dptfManager;

    mutable Bool m_outputMessageStringCreated;
    mutable std::string m_outputMessageString;

    static const UIntN KeyLength = 22;

    std::string getFrameworkEventString(FrameworkEvent::Type frameworkEvent) const;
    std::string getParticipantName(UIntN participantIndex) const;
    std::string getDomainName(UIntN participantIndex, UIntN domainIndex) const;
    std::string getPolicyName(UIntN policyIndex) const;
    std::string getEsifPrimitiveName(esif_primitive_type primitive) const;
    std::string getEsifErrorCodeString(eEsifError esifErrorCode) const;

    std::string createStandardizedKey(const std::string& key) const;
    std::string createStandardizedIndex(UIntN index) const;
    std::string numToString(UInt64 number) const;
};