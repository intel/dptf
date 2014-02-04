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

#include "ParticipantMessage.h"

ParticipantMessage::ParticipantMessage(const std::string& fileName, UIntN lineNumber,
    const std::string& executingFunctionName) :
    DptfMessage(fileName, lineNumber, executingFunctionName)
{
}

ParticipantMessage::ParticipantMessage(const std::string& fileName, UIntN lineNumber,
    const std::string& executingFunctionName, const std::string& message, UIntN domainIndex) :
    DptfMessage(fileName, lineNumber, executingFunctionName)
{
    addMessage("Message", message);
    m_domainIndex = domainIndex;
}

ParticipantMessage::~ParticipantMessage(void)
{
}

void ParticipantMessage::setDomainIndex(UIntN domainIndex)
{
    m_domainIndex = domainIndex;
}

void ParticipantMessage::setEsifPrimitive(esif_primitive_type primitive, UInt32 instance)
{
    m_esifPrimitiveValid = true;
    m_esifPrimitive = primitive;
    m_esifPrimitiveInstance = instance;
}