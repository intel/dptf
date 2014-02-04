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

#include "LpmConfigurationReader.h"
#include "LpmTable.h"
#include <regex>
#include <sstream>

LpmConfigurationReader::LpmConfigurationReader(
    string root, PolicyServicesInterfaceContainer policyServices)
    :m_root(root)
{
    setPolicyServices(policyServices);
}

LpmConfigurationReader::~LpmConfigurationReader()
{

}

void LpmConfigurationReader::setKeyPath(string keyPath)
{
    m_keyPath = keyPath;
}

vector<string> LpmConfigurationReader::tokenize(string inputString, Int8 c)
{
    istringstream inputStringStream(inputString);
    string splitField;
    vector<string> splitFields;

    if (inputString.empty() == true)
    {
        return splitFields;
    }

    while (inputStringStream.eof() == false)
    {
        getline(inputStringStream, splitField, c);
        splitFields.push_back(splitField);
    }

    return splitFields;
}

string LpmConfigurationReader::root(void) const
{
    return m_root;
}

string LpmConfigurationReader::keyPath(void) const
{
    return m_keyPath;
}

void LpmConfigurationReader::trim(string& stringToTrim, Int8 c)
{
    string::iterator it = std::remove(stringToTrim.begin(), stringToTrim.end(), c);
    stringToTrim.erase(it, stringToTrim.end());
}

std::string LpmConfigurationReader::getIndexAsString(UIntN index)
{
    stringstream indexStream;
    indexStream.fill('0');
    indexStream.width(4);
    indexStream << index;

    return indexStream.str();
}
