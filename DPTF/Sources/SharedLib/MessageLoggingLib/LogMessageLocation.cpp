/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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

#include "LogMessageLocation.h"
#include "FileIo.h"
using namespace std;

LogMessageLocation::LogMessageLocation(string file, string line, string function)
	: m_file(move(file))
	, m_line(move(line))
	, m_function(move(function))
{
}
LogMessageLocation::LogMessageLocation(string file, int line, string function)
	: m_file(move(file))
	, m_line(to_string(line))
	, m_function(move(function))
{
}
bool LogMessageLocation::operator==(const LogMessageLocation& other) const
{
	return (m_file == other.m_file) && (m_line == other.m_line) && (m_function == other.m_function);
}
string LogMessageLocation::toStringFull() const
{
	stringstream stream;
	stream << m_file << "|" << m_function << "|" << m_line;
	return stream.str();
}

string LogMessageLocation::toStringCompact() const
{
	stringstream stream;
	stream << IFileIo::getFileNameFromPath(m_file) << "|" << m_line;
	return stream.str();
}
