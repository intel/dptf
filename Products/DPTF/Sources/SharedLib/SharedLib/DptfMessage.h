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
#include "FrameworkEvent.h"
#include "esif_uf_app_event_type.h"
#include "esif_primitive_type.h"
#include "esif_rc.h"
#include "MessageCategory.h"

// We would like to write out the compiler provided function name in the message.  But, different compilers
// support different version of this functionality.  We use the preprocessor to help.
#ifdef ESIF_ATTR_OS_WINDOWS
#define FUNCTION __FUNCTION__
#else
#define FUNCTION __func__
#endif

#define FLF __FILE__, __LINE__, FUNCTION

struct MessageKeyValuePair
{
    MessageKeyValuePair(const std::string& messageKey, const std::string& messageValue) :
        m_messageKey(messageKey), m_messageValue(messageValue)
    {
    }

    std::string m_messageKey;
    std::string m_messageValue;
};

class DptfMessage
{
public:

    DptfMessage(const std::string& fileName, UIntN lineNumber, const std::string& executingFunctionName);
    virtual ~DptfMessage(void);

    void addMessage(const std::string& messageValue);
    void addMessage(const std::string& messageKey, const std::string& messageValue);
    void addMessage(const std::string& messageKey, UInt64 messageValue);
    void addMessage(const std::string& messageKey, Temperature messageValue);

    // If an exception was caught and needs to be displayed, this stores the function that was called and the
    // exception text that was caught in the exception handler.
    void setExceptionCaught(const std::string& exceptionFunctionName, const std::string& exceptionText);

protected:

    std::string m_dptfVersion;
    std::string m_dptfBuildDate;
    std::string m_dptfBuildTime;

    std::string m_fileName;
    std::string m_lineNumber;
    std::string m_executingFunctionName;

    std::vector<MessageKeyValuePair> m_messageKeyValuePair;

    Bool m_frameworkEventValid;
    FrameworkEvent::Type m_frameworkEvent;

    UIntN m_participantIndex;
    UIntN m_domainIndex;
    UIntN m_policyIndex;

    Bool m_esifPrimitiveValid;
    esif_primitive_type m_esifPrimitive;
    UInt32 m_esifPrimitiveInstance;

    Bool m_esifEventGuidValid;
    Guid m_esifEventGuid;

    Bool m_esifErrorCodeValid;
    eEsifError m_esifErrorCode;

    std::string m_exceptionFunctionName;
    std::string m_exceptionText;
};