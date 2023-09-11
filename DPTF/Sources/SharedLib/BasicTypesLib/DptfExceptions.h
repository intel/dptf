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

#pragma once

// Do not include Dptf.h

#include "DptfExport.h"
#include <stdexcept>
#include <string>
#include "esif_ccb_rc.h"

class dptf_exception : public std::logic_error
{
public:
	dptf_exception(const std::string& description) noexcept;
	const char* what() const noexcept override;
	std::string getDescription() const;

protected:
	std::string m_description;
};

class invalid_data : public dptf_exception
{
public:
	invalid_data(const std::string& description);
};

class invalid_data_type : public dptf_exception
{
public:
	invalid_data_type(const std::string& description);
};

class dptf_out_of_range : public dptf_exception
{
public:
	dptf_out_of_range(const std::string& description);
};

class temperature_out_of_range : public dptf_exception
{
public:
	temperature_out_of_range(const std::string& description);
};

class memory_allocation_failure : public dptf_exception
{
public:
	memory_allocation_failure(const std::string& description);
};

class file_open_create_failure : public dptf_exception
{
public:
	file_open_create_failure(const std::string& description);
};

class buffer_too_small : public dptf_exception
{
public:
	buffer_too_small(const std::string& description);
};

class primitive_execution_failed : public dptf_exception
{
public:
	primitive_execution_failed(const std::string& description);
};

class primitive_try_again : public dptf_exception
{
public:
	primitive_try_again(const std::string& description);
};

class primitive_destination_unavailable : public dptf_exception
{
public:
	primitive_destination_unavailable(const std::string& description);
};

class not_implemented : public dptf_exception
{
public:
	not_implemented();
};

class participant_not_enabled : public dptf_exception
{
public:
	participant_not_enabled();
};

class domain_not_enabled : public dptf_exception
{
public:
	domain_not_enabled();
};

class domain_control_nullptr : public dptf_exception
{
public:
	domain_control_nullptr();
};

class policy_index_invalid : public dptf_exception
{
public:
	policy_index_invalid();
};

class policy_already_exists : public dptf_exception
{
public:
	policy_already_exists();
};

class policy_not_in_idsp_list : public dptf_exception
{
public:
	policy_not_in_idsp_list();
};

class dynamic_policy_template_guid_invalid : public dptf_exception
{
public:
	dynamic_policy_template_guid_invalid();
};

class participant_index_invalid : public dptf_exception
{
public:
	participant_index_invalid();
};

class unsupported_result_temp_type : public dptf_exception
{
public:
	unsupported_result_temp_type();
};

class duplicate_work_item : public dptf_exception
{
public:
	duplicate_work_item(const std::string& description);
};

class primitive_not_found_in_dsp : public dptf_exception
{
public:
	primitive_not_found_in_dsp(const std::string& description);
};

class acpi_object_not_found : public dptf_exception
{
public:
	acpi_object_not_found(const std::string& description);
};

class command_failure : public dptf_exception
{
public:
	command_failure(esif_error_t errorCode, const std::string& description);
	esif_error_t getErrorCode() const;

private:
	esif_error_t m_errorCode;
};

// FIXME: implement_me is in place until we implement the function.  we should
// not release code that throws this exception.
class implement_me : public dptf_exception
{
public:
	implement_me();
};
