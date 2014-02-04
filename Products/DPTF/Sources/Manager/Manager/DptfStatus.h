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
#include "DptfManager.h"
#include "esif_rc.h"
#include "ParticipantSpecificInfoKey.h"

class XmlNode;
class Indent;
class Participant;
class ParticipantStatusMap;

class DptfStatus
{
public:

    DptfStatus(DptfManager* dptfManager);
    ~DptfStatus();

    void getStatus(const eAppStatusCommand command, const UInt32 appStatusIn,
        esif::EsifDataPtr appStatusOut, eEsifError* returnCode);

private:

    DptfManager* m_dptfManager;
    PolicyManager* m_policyManager;
    ParticipantManager* m_participantManager;
    ParticipantStatusMap* m_participantStatusMap;

    std::string getFileContent(std::string fileName);
    std::string getXsltContent(eEsifError* returnCode);
    std::string getGroupsXml(eEsifError* returnCode);
    std::string getModulesInGroup(const UInt32 appStatusIn, eEsifError* returnCode);
    std::string getPoliciesGroup();
    std::string getParticipantsGroup();
    std::string getFrameworkGroup();
    std::string getModuleData(const UInt32 appStatusIn, eEsifError* returnCode);
    std::string getXmlForPolicy(UInt32 policyIndex, eEsifError* returnCode);
    std::string getXmlForParticipant(UInt32 mappedIndex, eEsifError* returnCode);
    std::string getXmlForFramework(UInt32 moduleIndex, eEsifError* returnCode);
    XmlNode* getXmlForFrameworkLoadedPolicies();
    XmlNode* getXmlForFrameworkLoadedParticipants();

    void fillEsifString(esif::EsifDataPtr outputLocation, std::string inputString, eEsifError* returnCode);

    // KW error resolution
    DptfStatus(const DptfStatus&);
    DptfStatus& operator=(const DptfStatus&)
    {
        return *this;
    }
};