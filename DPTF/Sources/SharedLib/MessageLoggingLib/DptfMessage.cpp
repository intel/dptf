/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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

#include "DptfMessage.h"
#include "DptfVer.h"
using namespace std;

DptfMessage::DptfMessage(const string& fileName, UIntN lineNumber, const string& executingFunctionName)
	: m_dptfVersion(VERSION_STR)
	, m_dptfBuildDate(__DATE__)
	, m_dptfBuildTime(__TIME__)
	, m_fileName(fileName)
	, m_executingFunctionName(executingFunctionName)
	, m_frameworkEventValid(false)
	, m_frameworkEvent()
	, m_participantIndex(Constants::Invalid)
	, m_domainIndex(Constants::Invalid)
	, m_policyIndex(Constants::Invalid)
	, m_esifPrimitiveValid(false)
	, m_esifPrimitive()
	, m_esifPrimitiveInstance(Constants::Esif::NoInstance)
	, m_esifEventGuidValid(false)
	, m_esifErrorCodeValid(false)
	, m_esifErrorCode(ESIF_E_UNSPECIFIED)
	, m_exceptionFunctionName(Constants::EmptyString)
	, m_exceptionText(Constants::EmptyString)
{
	try
	{
		m_lineNumber = to_string(lineNumber);
	}
	catch (...)
	{
		m_lineNumber = "Unknown";
	}
}

DptfMessage::~DptfMessage(void)
{
}

void DptfMessage::addMessage(const string& message)
{
	try
	{
		m_messageKeyValuePair.push_back(MessageKeyValuePair("Message", message));
	}
	catch (...)
	{
	}
}

void DptfMessage::addMessage(const string& messageKey, const string& messageValue)
{
	try
	{
		m_messageKeyValuePair.push_back(MessageKeyValuePair(messageKey, messageValue));
	}
	catch (...)
	{
	}
}

void DptfMessage::addMessage(const string& messageKey, UInt64 messageValue)
{
	try
	{
		m_messageKeyValuePair.push_back(MessageKeyValuePair(messageKey, to_string(messageValue)));
	}
	catch (...)
	{
	}
}

void DptfMessage::addMessage(const string& messageKey, Temperature messageValue)
{
	try
	{
		m_messageKeyValuePair.push_back(MessageKeyValuePair(messageKey, messageValue.toString()));
	}
	catch (...)
	{
	}
}

void DptfMessage::setExceptionCaught(const string& exceptionFunctionName, const string& exceptionText)
{
	m_exceptionFunctionName = exceptionFunctionName;
	m_exceptionText = exceptionText;
}
