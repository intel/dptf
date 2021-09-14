/******************************************************************************
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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

namespace DataVaultPathBasePaths
{
	static const std::string FeaturesRoot = "/features";
	static const std::string SharedRoot = "/shared";
	static const std::string TablesRoot = SharedRoot + "/tables";
	static const std::string ValuesRoot = SharedRoot + "/values";
	static const std::string ExportRoot = SharedRoot + "/export";
	static const std::string AUPPControlsRoot = SharedRoot + "/aupp_controls";
}

namespace DataVaultPath
{
	namespace Shared
	{
		namespace Export
		{
			static const std::string WorkloadHints = DataVaultPathBasePaths::ExportRoot + "/workload_hints/";
			static const std::string DoNotRebalanceUnusedPidBudget =
				DataVaultPathBasePaths::ExportRoot + "/do_not_rebalance_unused_pid_budget/";
			static const std::string Pl1TimeWindow = DataVaultPathBasePaths::ExportRoot + "/pl1_time_window/";
			static const std::string ProcPl1TimeWindow =
				DataVaultPathBasePaths::ExportRoot + "/proc_ps_pl1_time_window/";
			static const std::string GfxPl1TimeWindow = DataVaultPathBasePaths::ExportRoot + "/gfx_ps_pl1_time_window/";
			static const std::string PowerShareParamsLoggingState =
				DataVaultPathBasePaths::ExportRoot + "/powershare_params_logging/";

			static const std::string PredictiveBattDelayStart =
				DataVaultPathBasePaths::ExportRoot + "/pred_batt_delay_start";
			static const std::string PredictiveBattFastCheckInterval =
				DataVaultPathBasePaths::ExportRoot + "/pred_batt_fast_chk_intrvl";
			static const std::string PredictiveBattEporStepSize =
				DataVaultPathBasePaths::ExportRoot + "/pred_batt_epor_step_size";
			static const std::string Above2cTimeConstraintStepsize =
				DataVaultPathBasePaths::ExportRoot + "/above_2c_time_constraint_stepsize";
		};

		namespace Tables
		{
			static const std::string Psvt = DataVaultPathBasePaths::TablesRoot + "/psvt/";
			static const std::string Art = DataVaultPathBasePaths::TablesRoot + "/_art/";
			static const std::string Trt = DataVaultPathBasePaths::TablesRoot + "/_trt/";
			static const std::string Pida = DataVaultPathBasePaths::TablesRoot + "/pida/";
			static const std::string Psha = DataVaultPathBasePaths::TablesRoot + "/psha/";
			static const std::string Acpr = DataVaultPathBasePaths::TablesRoot + "/acpr/";
			static const std::string Psh2 = DataVaultPathBasePaths::TablesRoot + "/psh2/";
			static const std::string Itmt = DataVaultPathBasePaths::TablesRoot + "/itmt/";
		};
	};

	namespace Features
	{
		static const std::string Hwpf = DataVaultPathBasePaths::FeaturesRoot + "/hwpf";
		static const std::string PowerShare2Pl2Pl4Sharing = DataVaultPathBasePaths::FeaturesRoot + "/pl2_pl4_sharing";
	};
};
