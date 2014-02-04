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

#include "ManagerMessage.h"
#include "DptfManager.h"
#include "ParticipantManager.h"
#include "Participant.h"
#include "PolicyManager.h"
#include "Policy.h"
#include "FrameworkEvent.h"

ManagerMessage::ManagerMessage(const DptfManager* dptfManager, const std::string& fileName,
    UIntN lineNumber, const std::string& executingFunctionName) :
    DptfMessage(fileName, lineNumber, executingFunctionName),
    m_dptfManager(dptfManager),
    m_outputMessageStringCreated(false)
{
}

ManagerMessage::ManagerMessage(const DptfManager* dptfManager, const std::string& fileName,
    UIntN lineNumber, const std::string& executingFunctionName, const std::string& message) :
    DptfMessage(fileName, lineNumber, executingFunctionName),
    m_dptfManager(dptfManager),
    m_outputMessageStringCreated(false)
{
    addMessage(message);
}

ManagerMessage::ManagerMessage(const DptfManager* dptfManager, const DptfMessage& dptfMessage) :
    DptfMessage(dptfMessage),
    m_dptfManager(dptfManager),
    m_outputMessageStringCreated(false)
{
}

ManagerMessage::~ManagerMessage(void)
{
}

void ManagerMessage::setFrameworkEvent(FrameworkEvent::Type frameworkEvent)
{
    m_frameworkEventValid = true;
    m_frameworkEvent = frameworkEvent;
}

void ManagerMessage::setParticipantIndex(UIntN participantIndex)
{
    m_participantIndex = participantIndex;
}

void ManagerMessage::setParticipantAndDomainIndex(UIntN participantIndex, UIntN domainIndex)
{
    m_participantIndex = participantIndex;
    m_domainIndex = domainIndex;
}

void ManagerMessage::setPolicyIndex(UIntN policyIndex)
{
    m_policyIndex = policyIndex;
}

void ManagerMessage::setEsifPrimitive(esif_primitive_type primitive, UInt32 instance)
{
    m_esifPrimitiveValid = true;
    m_esifPrimitive = primitive;
    m_esifPrimitiveInstance = instance;
}

void ManagerMessage::setEsifEventGuid(const Guid& esifEventGuid)
{
    m_esifEventGuidValid = true;
    m_esifEventGuid = esifEventGuid;
}

void ManagerMessage::setEsifErrorCode(eEsifError esifErrorCode)
{
    m_esifErrorCodeValid = true;
    m_esifErrorCode = esifErrorCode;
}

ManagerMessage::operator std::string(void) const
{
    // Build the string message and return it.

    if (m_outputMessageStringCreated == false)
    {
        std::string key;
        std::stringstream message;

        message << std::endl;

        key = createStandardizedKey("DPTF Build Version");
        message << key << m_dptfVersion << std::endl;

        key = createStandardizedKey("DPTF Build Date");
        message << key << m_dptfBuildDate << " " << m_dptfBuildTime << std::endl;

        key = createStandardizedKey("Source File");
        message << key << m_fileName << " @ line " << m_lineNumber << std::endl;

        key = createStandardizedKey("Executing Function");
        message << key << m_executingFunctionName << std::endl;

        for (auto it = m_messageKeyValuePair.cbegin(); it != m_messageKeyValuePair.cend(); it++)
        {
            key = createStandardizedKey(it->m_messageKey);
            message << key << it->m_messageValue << std::endl;
        }

        if (m_frameworkEventValid == true)
        {
            key = createStandardizedKey("Framework Event");
            message << key << getFrameworkEventString(m_frameworkEvent) << createStandardizedIndex((UIntN)m_frameworkEvent) << std::endl;
        }

        if (m_participantIndex != Constants::Invalid)
        {
            key = createStandardizedKey("Participant");
            message << key << getParticipantName(m_participantIndex);

            if (m_participantIndex != Constants::Esif::NoParticipant)
            {
                message << createStandardizedIndex(m_participantIndex);
            }

            message << std::endl;
        }

        if (m_domainIndex != Constants::Invalid)
        {
            key = createStandardizedKey("Domain");
            message << key << getDomainName(m_participantIndex, m_domainIndex);

            if (m_domainIndex != Constants::Esif::NoDomain)
            {
                message << createStandardizedIndex(m_domainIndex);
            }

            message << std::endl;
        }

        if (m_policyIndex != Constants::Invalid)
        {
            key = createStandardizedKey("Policy");
            message << key << getPolicyName(m_policyIndex) << createStandardizedIndex(m_policyIndex) << std::endl;
        }

        if (m_esifPrimitiveValid == true)
        {
            key = createStandardizedKey("ESIF Primitive");
            message << key << getEsifPrimitiveName(m_esifPrimitive) << createStandardizedIndex((UIntN)m_esifPrimitive) << std::endl;

            key = createStandardizedKey("ESIF Instance");
            message << key << numToString(m_esifPrimitiveInstance) << std::endl;
        }

        if (m_esifEventGuidValid == true)
        {
            key = createStandardizedKey("ESIF Event Guid");
            message << key << m_esifEventGuid.toString() << std::endl;
        }

        if (m_esifErrorCodeValid == true)
        {
            key = createStandardizedKey("ESIF Return Code");
            message << key << getEsifErrorCodeString(m_esifErrorCode) << createStandardizedIndex((UIntN)m_esifErrorCode) << std::endl;
        }

        if (m_exceptionFunctionName.length() > 0 || m_exceptionText.length() > 0)
        {
            key = createStandardizedKey("Exception Function");
            message << key << m_exceptionFunctionName << std::endl;

            key = createStandardizedKey("Exception Text");
            message << key << std::endl << m_exceptionText << std::endl;
        }

        message << std::endl;

        m_outputMessageString = message.str();
        m_outputMessageStringCreated = true;
    }

    return m_outputMessageString;
}

std::string ManagerMessage::getFrameworkEventString(FrameworkEvent::Type frameworkEvent) const
{
    std::string frameworkEventString;

    try
    {
        const FrameworkEventData event = (*FrameworkEventInfo::instance())[frameworkEvent];
        frameworkEventString = event.name;
    }
    catch (...)
    {
        frameworkEventString = "Invalid";
    }

    return frameworkEventString;
}

std::string ManagerMessage::getParticipantName(UIntN participantIndex) const
{
    std::string participantName;

    if (participantIndex == Constants::Esif::NoParticipant ||
        participantIndex == Constants::Invalid)
    {
        participantName = "NoParticipant";
    }
    else
    {
        try
        {
            Participant* participant = m_dptfManager->getParticipantManager()->getParticipantPtr(participantIndex);
            participantName = participant->getParticipantName();
        }
        catch (...)
        {
            participantName = "Invalid";
        }
    }

    return participantName;
}

std::string ManagerMessage::getDomainName(UIntN participantIndex, UIntN domainIndex) const
{
    std::string domainName;

    if (domainIndex == Constants::Esif::NoDomain ||
        domainIndex == Constants::Invalid)
    {
        domainName = "NoDomain";
    }
    else
    {
        try
        {
            Participant* participant = m_dptfManager->getParticipantManager()->getParticipantPtr(participantIndex);
            domainName = participant->getDomainName(domainIndex);
        }
        catch (...)
        {
            domainName = "Invalid";
        }
    }

    return domainName;
}

std::string ManagerMessage::getPolicyName(UIntN policyIndex) const
{
    std::string policyName;

    try
    {
        Policy* policy = m_dptfManager->getPolicyManager()->getPolicyPtr(policyIndex);
        policyName = policy->getName();
    }
    catch (...)
    {
        policyName = "Invalid";
    }

    return policyName;
}

std::string ManagerMessage::getEsifPrimitiveName(esif_primitive_type primitive) const
{
    std::string esifPrimitiveName;

    try
    {
        esifPrimitiveName = esif_primitive_str(primitive);
    }
    catch (...)
    {
        esifPrimitiveName = "Invalid";
    }

    return esifPrimitiveName;
}

std::string ManagerMessage::getEsifErrorCodeString(eEsifError esifErrorCode) const
{
    std::string errorCodeString;

    try
    {
        errorCodeString = esif_rc_str(esifErrorCode);
    }
    catch (...)
    {
        errorCodeString = "Invalid";
    }

    return errorCodeString;
}

std::string ManagerMessage::createStandardizedKey(const std::string& key) const
{
    std::string newKey = key + ":  ";
    return newKey;
}

std::string ManagerMessage::createStandardizedIndex(UIntN index) const
{
    std::string newIndex;

    try
    {
        newIndex = " [" + std::to_string(index) + "]";
    }
    catch (...)
    {
    }

    return newIndex;
}

std::string ManagerMessage::numToString(UInt64 number) const
{
    std::string outputNumber;

    try
    {
        outputNumber = std::to_string(number);
    }
    catch (...)
    {
        outputNumber = "Invalid";
    }

    return outputNumber;
}