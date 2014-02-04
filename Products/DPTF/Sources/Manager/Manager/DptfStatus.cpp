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

#include "DptfStatus.h"
#include "Indent.h"
#include "esif_ccb_string.h"
#include "esif_uf_app_iface.h"
#include "ParticipantSpecificInfoKey.h"
#include "Participant.h"
#include "Policy.h"
#include <fstream>
#include "DptfManager.h"
#include "PolicyManager.h"
#include "ParticipantManager.h"
#include "WorkItemQueueManager.h"
#include "StatusFormat.h"
#include <iostream>
#include "BinaryParse.h"
#include "XmlGeneratorFactory.h"
#include "XmlNode.h"
#include "ParticipantStatusMap.h"
#include "EsifDataString.h"

static const Guid FormatId(0x3E, 0x58, 0x63, 0x46, 0xF8, 0xF7, 0x45, 0x4A, 0xA8, 0xF7, 0xDE, 0x7E, 0xC6, 0xF7, 0x61, 0xA8);

DptfStatus::DptfStatus(DptfManager* dptfManager) :
    m_dptfManager(dptfManager)
{
    m_policyManager = m_dptfManager->getPolicyManager();
    m_participantManager = m_dptfManager->getParticipantManager();
    m_participantStatusMap = new ParticipantStatusMap(m_participantManager);
}

DptfStatus::~DptfStatus()
{
    DELETE_MEMORY_TC(m_participantStatusMap);
}

namespace GroupType
{
    enum Type
    {
        Policies = 0,
        Participants = 1,
        Framework = 2
    };
}

void DptfStatus::getStatus(const eAppStatusCommand command, const UInt32 appStatusIn,
    esif::EsifDataPtr appStatusOut, eEsifError* returnCode)
{
    std::string response;

    switch (command)
    {
        case eAppStatusCommandGetXSLT:
            response = getXsltContent(returnCode);
            break;
        case eAppStatusCommandGetGroups:
            response = getGroupsXml(returnCode);
            break;
        case eAppStatusCommandGetModulesInGroup:
            response = getModulesInGroup(appStatusIn, returnCode);
            break;
        case eAppStatusCommandGetModuleData:
            response = getModuleData(appStatusIn, returnCode);
            break;
        default:
            *returnCode = ESIF_E_UNSPECIFIED;
            throw dptf_exception("Received invalid command status code.");
    }
    fillEsifString(appStatusOut, response, returnCode);
}

std::string DptfStatus::getFileContent(std::string fileName)
{
    // Try to find file in current directory
    std::ifstream file(fileName,
        std::ios::in | std::ios::binary | std::ios::ate);

    if (file.is_open())
    {
        std::string content;
        content.resize(static_cast<UIntN>(file.tellg()));
        file.seekg(0, std::ios::beg);
        file.read(&content[0], content.size());
        file.close();
        return content;
    }
    else
    {
        throw dptf_exception("File not found.");
    }
}

std::string DptfStatus::getXsltContent(eEsifError* returnCode)
{
    try
    {
        return getFileContent(m_dptfManager->getDptfHomeDirectoryPath() + "combined.xsl");
    }
    catch (dptf_exception)
    {
        // Could not find file, try from Resources/
        *returnCode = ESIF_E_UNSPECIFIED;
        throw;
    }
}

std::string DptfStatus::getGroupsXml(eEsifError* returnCode)
{
    XmlNode* groups = XmlNode::createWrapperElement("groups");

    XmlNode* group0 = XmlNode::createWrapperElement("group");
    groups->addChild(group0);
    group0->addChild(XmlNode::createDataElement("id", "0"));
    group0->addChild(XmlNode::createDataElement("name", "Policies"));

    XmlNode* group1 = XmlNode::createWrapperElement("group");
    groups->addChild(group1);
    group1->addChild(XmlNode::createDataElement("id", "1"));
    group1->addChild(XmlNode::createDataElement("name", "Participants"));

    XmlNode* group2 = XmlNode::createWrapperElement("group");
    groups->addChild(group2);
    group2->addChild(XmlNode::createDataElement("id", "2"));
    group2->addChild(XmlNode::createDataElement("name", "Manager"));

    std::string s = groups->toString();
    delete groups;

    return s;
}

std::string DptfStatus::getModulesInGroup(const UInt32 appStatusIn, eEsifError* returnCode)
{
    std::string modulesInGroup;

    switch (appStatusIn)
    {
        case GroupType::Policies:
            modulesInGroup = getPoliciesGroup();
            break;
        case GroupType::Participants:
            modulesInGroup = getParticipantsGroup();
            break;
        case GroupType::Framework:
            modulesInGroup = getFrameworkGroup();
            break;
        default:
            *returnCode = ESIF_E_UNSPECIFIED;
            throw dptf_exception("Invalid group ID specified.");
    }
    return modulesInGroup;
}

std::string DptfStatus::getPoliciesGroup()
{
    XmlNode* modules = XmlNode::createWrapperElement("modules");

    UIntN policyCount = m_policyManager->getPolicyListCount();
    for (UIntN policyIndex = 0; policyIndex < policyCount; policyIndex++)
    {
        try
        {
            // Get the policy variables before adding nodes.  This forces
            // exceptions to be thrown first.
            Policy* policy = m_policyManager->getPolicyPtr(policyIndex);
            std::string name = policy->getName();

            XmlNode* module = XmlNode::createWrapperElement("module");
            modules->addChild(module);

            XmlNode* policyId = XmlNode::createDataElement("id", std::to_string(policyIndex));
            module->addChild(policyId);

            XmlNode* policyName = XmlNode::createDataElement("name", name);
            module->addChild(policyName);
        }
        catch (...)
        {
            // policy not available, don't add it to the list
        }
    }

    std::string s = modules->toString();
    delete modules;

    return s;
}

std::string DptfStatus::getParticipantsGroup()
{
    return m_participantStatusMap->getGroupsString();
}

std::string DptfStatus::getFrameworkGroup()
{
    XmlNode* modules = XmlNode::createWrapperElement("modules");

    // DPPM Status

    XmlNode* module = XmlNode::createWrapperElement("module");
    modules->addChild(module);

    XmlNode* moduleId = XmlNode::createDataElement("id", std::to_string(0));
    module->addChild(moduleId);

    XmlNode* moduleName = XmlNode::createDataElement("name", "DPPM Status");
    module->addChild(moduleName);

#ifdef INCLUDE_WORK_ITEM_STATISTICS

    // Work Item Statistics

    module = XmlNode::createWrapperElement("module");
    modules->addChild(module);

    moduleId = XmlNode::createDataElement("id", std::to_string(1));
    module->addChild(moduleId);

    moduleName = XmlNode::createDataElement("name", "Work Item Statistics");
    module->addChild(moduleName);

#endif

    std::string s = modules->toString();
    delete modules;

    return s;
}

std::string DptfStatus::getModuleData(const UInt32 appStatusIn, eEsifError* returnCode)
{
    std::string moduleData;

    UInt32 groupId = static_cast<UInt32>(BinaryParse::extractBits(32, 16, appStatusIn));
    UInt32 moduleId = static_cast<UInt32>(BinaryParse::extractBits(15, 0, appStatusIn));

    switch (groupId)
    {
        case GroupType::Policies:
            moduleData = getXmlForPolicy(moduleId, returnCode);
            break;
        case GroupType::Participants:
            moduleData = getXmlForParticipant(moduleId, returnCode);
            break;
        case GroupType::Framework:
            moduleData = getXmlForFramework(moduleId, returnCode);
            break;
        default:
            *returnCode = ESIF_E_UNSPECIFIED;
            throw dptf_exception("Invalid group ID specified.");
    }
    return moduleData;
}

std::string DptfStatus::getXmlForPolicy(UInt32 policyIndex, eEsifError* returnCode)
{
    UIntN policyCount = m_policyManager->getPolicyListCount();
    if (policyIndex >= policyCount)
    {
        *returnCode = ESIF_E_UNSPECIFIED;
        throw dptf_exception("Invalid policy status requested.");
    }

    try
    {
        Policy* policy = m_policyManager->getPolicyPtr(policyIndex);
        return policy->getStatusAsXml();
    }
    catch (...)
    {
        *returnCode = ESIF_E_UNSPECIFIED;
        throw;
    }
}

std::string DptfStatus::getXmlForParticipant(UInt32 mappedIndex, eEsifError* returnCode)
{
    UIntN participantCount = m_participantManager->getParticipantListCount();
    UIntN totalDomainCount = 0;

    // Total # of participants + domains for error checking
    for (UIntN i = 0; i < participantCount; i++)
    {
        Participant* participant = m_participantManager->getParticipantPtr(i);
        totalDomainCount += participant->getDomainCount();
    }

    if (mappedIndex >= totalDomainCount)
    {
        *returnCode = ESIF_E_UNSPECIFIED;
        throw dptf_exception("Invalid participant status requested.");
    }

    try
    {
        XmlNode* participantData = m_participantStatusMap->getStatusAsXml(mappedIndex);
        std::string s = participantData->toString();
        delete participantData;

        return s;
    }
    catch (...)
    {
        *returnCode = ESIF_E_UNSPECIFIED;
        throw;
    }
}

std::string DptfStatus::getXmlForFramework(UInt32 moduleIndex, eEsifError* returnCode)
{
    XmlNode* frameworkRoot = nullptr;

    switch (moduleIndex)
    {
        case 0:
        {
            frameworkRoot = XmlNode::createRoot();
            XmlNode* formatId = XmlNode::createComment("format_id=" + FormatId.toString());
            frameworkRoot->addChild(formatId);

            XmlNode* dppmRoot = XmlNode::createWrapperElement("dppm_status");
            frameworkRoot->addChild(dppmRoot);

            XmlNode* policiesRoot = getXmlForFrameworkLoadedPolicies();
            dppmRoot->addChild(policiesRoot);

            XmlNode* participantsRoot = getXmlForFrameworkLoadedParticipants();
            dppmRoot->addChild(participantsRoot);

            *returnCode = ESIF_OK;
            break;
        }
        case 1:
        {
            frameworkRoot = m_dptfManager->getWorkItemQueueManager()->getStatusAsXml();
            *returnCode = ESIF_OK;
            break;
        }
        default:
            *returnCode = ESIF_E_UNSPECIFIED;
            break;
    }

    std::string s;
    if (frameworkRoot != nullptr)
    {
        s = frameworkRoot->toString();
        delete frameworkRoot;
    }

    return s;
}

XmlNode* DptfStatus::getXmlForFrameworkLoadedPolicies()
{
    XmlNode* policiesRoot = XmlNode::createWrapperElement("policies");

    UIntN policyCount = m_policyManager->getPolicyListCount();
    for (UIntN i = 0; i < policyCount; i++)
    {
        try
        {
            Policy* policy = m_policyManager->getPolicyPtr(i);
            std::string name = policy->getName();

            XmlNode* policyRoot = XmlNode::createWrapperElement("policy");

            XmlNode* policyIndex = XmlNode::createDataElement("policy_index", std::to_string(i));
            policyRoot->addChild(policyIndex);

            XmlNode* policyName = XmlNode::createDataElement("policy_name", name);
            policyRoot->addChild(policyName);

            policiesRoot->addChild(policyRoot);
        }
        catch (...)
        {
            // Policy not available, do not add.
        }
    }

    return policiesRoot;
}

XmlNode* DptfStatus::getXmlForFrameworkLoadedParticipants()
{
    XmlNode* participantsRoot = XmlNode::createWrapperElement("participants");

    UIntN participantCount = m_participantManager->getParticipantListCount();
    for (UIntN i = 0; i < participantCount; i++)
    {
        Participant* p = m_participantManager->getParticipantPtr(i);
        participantsRoot->addChild(p->getXml(Constants::Invalid));
    }

    return participantsRoot;
}

void DptfStatus::fillEsifString(esif::EsifDataPtr outputLocation, std::string inputString, eEsifError* returnCode)
{
    eEsifError result = FillDataPtrWithString(outputLocation, inputString);
    if (result != ESIF_OK)
    {
        *returnCode = result;
        throw dptf_exception("Failed to fill ESIF data pointer with string.");
    }
}