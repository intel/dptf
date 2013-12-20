rem ##############################################################################
rem ## Copyright (c) 2013 Intel Corporation All Rights Reserved
rem ##
rem ## Licensed under the Apache License, Version 2.0 (the "License"); you may not
rem ## use this file except in compliance with the License.
rem ##
rem ## You may obtain a copy of the License at
rem ##     http://www.apache.org/licenses/LICENSE-2.0
rem ##
rem ## Unless required by applicable law or agreed to in writing, software
rem ## distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
rem ## WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
rem ##
rem ## See the License for the specific language governing permissions and
rem ## limitations under the License.
rem ##
rem ##############################################################################
rem
rem Eco-System Independent Framework v1 (ESIF)
rem ESIF Test Framework (ETF)
rem
rem ETF Package: sb_b0_d4_f0
rem     Format: ESIF Shell Script
rem     Version: 1.0
rem
rem =====                                  ======
rem ===== AUTO GENERATED FEEL FREE TO EDIT ======
rem =====                                  ======
rem
log sb_b0_d4_f0.log
info
seterrorlevel 0
timerstart
setb 8192
getp 049 D0 255 -u 0xffffffff -l 0                           ;GET_DEVICE_ADDRESS_ON_PARENT_BUS
getp 088 D0 255 -u 0xf -l 0                                  ;GET_DEVICE_STATUS
getp 178 D1 255 -u 3 -l 0                                    ;GET_DOMAIN_PRIORITY
getp 178 D2 255 -u 3 -l 0                                    ;GET_DOMAIN_PRIORITY
getp 054 D0 255 -u 105 -l 0                                  ;GET_NOTIFICATION_TEMP_THRESHOLD
getp 137 D2 255 -b sb_b0_d4_f0_137_D2_255.bin                ;GET_PERF_SUPPORT_STATES
getp 068 D0 255                                              ;GET_PROC_APPLICATION_EXCLUDE_LIST
getp 079 D0 255                                              ;GET_PROC_CTDP_CAPABILITY
getp 026 D0 255                                              ;GET_PROC_CTDP_CURRENT_SETTING
getp 027 D0 255 -u 1 -l 0                                    ;GET_PROC_CTDP_LOCK_STATUS
getp 080 D0 255 -b sb_b0_d4_f0_80_D0_255.bin                 ;GET_PROC_CTDP_POINT_LIST
getp 025 D0 255                                              ;GET_PROC_CTDP_SUPPORT_CHECK
getp 028 D0 255                                              ;GET_PROC_CTDP_TAR_LOCK_STATUS
getp 069 D1 255 -b sb_b0_d4_f0_69_D1_255.bin                 ;GET_PROC_CURRENT_LOGICAL_PROCESSOR_OFFLINING
getp 228 D1 255                                              ;GET_PROC_LOGICAL_PROCESSOR_AFFINITY
getp 207 D0 255 -u 1 -l 0                                    ;GET_PROC_MFM_LPM_SUPPORTED
getp 209 D0 255                                              ;GET_PROC_MFM_MAX_EFFICIENCY_RATIO
getp 208 D0 255                                              ;GET_PROC_MFM_MIN_SUPPORTED_RATIO
getp 056 D1 255                                              ;GET_PROC_PERF_PRESENT_CAPABILITY
getp 055 D1 255 -u 100 -l 0                                  ;GET_PROC_PERF_PSTATE_DEPTH_LIMIT
getp 095 D1 255 -b sb_b0_d4_f0_95_D1_255.bin                 ;GET_PROC_PERF_SUPPORT_STATES
getp 058 D0 255 -b sb_b0_d4_f0_58_D0_255.bin                 ;GET_PROC_PERF_THROTTLE_CONTROL
getp 062 D1 255                                              ;GET_PROC_PERF_THROTTLE_PRESENT_CAPABILITY
getp 061 D1 255                                              ;GET_PROC_PERF_TSTATE_DEPTH_LIMIT
getp 021 D0 255 -u 200000 -l 0                               ;GET_PROC_THERMAL_DESIGN_POWER
getp 020 D0 255 -u 105 -l 0                                  ;GET_PROC_TJMAX
getp 219 D0 255                                              ;GET_PROC_TURBO_ACTIVATION_RATIO
getp 218 D0 255 -u 1 -l 0                                    ;GET_PROC_TURBO_STATE
getp 128 D0 255                                              ;GET_RAPL_ENERGY
getp 128 D1 255                                              ;GET_RAPL_ENERGY
getp 128 D2 255                                              ;GET_RAPL_ENERGY
getp 123 D0 255                                              ;GET_RAPL_ENERGY_UNIT
getp 035 D0 255                                              ;GET_RAPL_POWER
getp 035 D1 255                                              ;GET_RAPL_POWER
getp 035 D2 255                                              ;GET_RAPL_POWER
getp 075 D0 255 -b sb_b0_d4_f0_75_D0_255.bin                 ;GET_RAPL_POWER_CONTROL_CAPABILITIES
getp 038 D0 000                                              ;GET_RAPL_POWER_LIMIT
getp 038 D0 001                                              ;GET_RAPL_POWER_LIMIT
getp 038 D1 000                                              ;GET_RAPL_POWER_LIMIT
getp 038 D2 000                                              ;GET_RAPL_POWER_LIMIT
getp 126 D0 000 -u 1 -l 0                                    ;GET_RAPL_POWER_LIMIT_ENABLE
getp 126 D0 001 -u 1 -l 0                                    ;GET_RAPL_POWER_LIMIT_ENABLE
getp 126 D1 000 -u 1 -l 0                                    ;GET_RAPL_POWER_LIMIT_ENABLE
getp 126 D2 000 -u 1 -l 0                                    ;GET_RAPL_POWER_LIMIT_ENABLE
getp 172 D0 255 -u 1 -l 0                                    ;GET_RAPL_POWER_LIMIT_LOCK
getp 172 D1 255 -u 1 -l 0                                    ;GET_RAPL_POWER_LIMIT_LOCK
getp 172 D2 255 -u 1 -l 0                                    ;GET_RAPL_POWER_LIMIT_LOCK
getp 023 D0 255                                              ;GET_RAPL_POWER_MAX
getp 024 D0 255                                              ;GET_RAPL_POWER_MAX_TIME_WINDOW
getp 022 D0 255                                              ;GET_RAPL_POWER_MIN
getp 124 D0 255 -u 3 -l 3                                    ;GET_RAPL_POWER_UNIT
getp 122 D0 255 -u 10 -l 10                                  ;GET_RAPL_TIME_UNIT
getp 039 D0 000                                              ;GET_RAPL_TIME_WINDOW
getp 039 D0 001                                              ;GET_RAPL_TIME_WINDOW
getp 039 D1 000                                              ;GET_RAPL_TIME_WINDOW
getp 039 D2 000                                              ;GET_RAPL_TIME_WINDOW
getp 014 D0 255 -u 105 -l 0                                  ;GET_TEMPERATURE
getp 014 D1 255 -u 105 -l 0                                  ;GET_TEMPERATURE
getp 014 D2 255 -u 105 -l 0                                  ;GET_TEMPERATURE
getp 072 D0 255 -u 2 -l 2                                    ;GET_TEMPERATURE_THRESHOLD_COUNT
getp 015 D0 255 -u 105 -l 0                                  ;GET_TEMPERATURE_THRESHOLD_HYSTERESIS
getp 143 D0 000                                              ;GET_TEMPERATURE_THRESHOLDS
getp 143 D0 001                                              ;GET_TEMPERATURE_THRESHOLDS
getp 001 D0 000 -u 105 -l 0                                  ;GET_TRIP_POINT_ACTIVE
getp 001 D0 001 -u 105 -l 0                                  ;GET_TRIP_POINT_ACTIVE
getp 001 D0 002 -u 105 -l 0                                  ;GET_TRIP_POINT_ACTIVE
getp 001 D0 003 -u 105 -l 0                                  ;GET_TRIP_POINT_ACTIVE
getp 001 D0 004 -u 105 -l 0                                  ;GET_TRIP_POINT_ACTIVE
getp 001 D0 005 -u 105 -l 0                                  ;GET_TRIP_POINT_ACTIVE
getp 001 D0 006 -u 105 -l 0                                  ;GET_TRIP_POINT_ACTIVE
getp 001 D0 007 -u 105 -l 0                                  ;GET_TRIP_POINT_ACTIVE
getp 001 D0 008 -u 105 -l 0                                  ;GET_TRIP_POINT_ACTIVE
getp 001 D0 009 -u 105 -l 0                                  ;GET_TRIP_POINT_ACTIVE
getp 013 D0 255 -u 105 -l 0                                  ;GET_TRIP_POINT_CRITICAL
getp 012 D0 255 -u 105 -l 0                                  ;GET_TRIP_POINT_HOT
getp 011 D0 255 -u 105 -l 0                                  ;GET_TRIP_POINT_PASSIVE
getp 177 D0 255 -u 105 -l 0                                  ;GET_TRIP_POINT_WARM
getp 177 D1 255 -u 105 -l 0                                  ;GET_TRIP_POINT_WARM
getp 016 D1 255                                              ;GET_TSTATE_CURRENT
getp 065 D1 255 -b sb_b0_d4_f0_65_D1_255.bin                 ;GET_TSTATES
rem setp 153 D1 255                                          ;SET_ACTIVE_CORE_LIMIT
rem setp 081 D0 255                                          ;SET_COOLING_POLICY
rem setp 083 D0 255                                          ;SET_CTDP_POINT
rem setp 051 D0 255                                          ;SET_DEVICE_TEMPERATURE_INDICATION
rem setp 082 D1 255                                          ;SET_PERF_PRESENT_CAPABILITY
rem setp 082 D2 255                                          ;SET_PERF_PRESENT_CAPABILITY
rem setp 221 D0 255                                          ;SET_PROC_CTDP_CONTROL
rem setp 229 D1 255                                          ;SET_PROC_LOGICAL_PROCESSOR_AFFINITY
rem setp 220 D0 255                                          ;SET_PROC_TURBO_ACTIVATION_RATIO
rem setp 100 D0 255                                          ;SET_PROC_TURBO_STATE
rem setp 130 D0 000                                          ;SET_RAPL_POWER_LIMIT
rem setp 130 D0 001                                          ;SET_RAPL_POWER_LIMIT
rem setp 130 D1 000                                          ;SET_RAPL_POWER_LIMIT
rem setp 130 D2 000                                          ;SET_RAPL_POWER_LIMIT
rem setp 222 D0 000                                          ;SET_RAPL_POWER_LIMIT_ENABLE
rem setp 222 D0 001                                          ;SET_RAPL_POWER_LIMIT_ENABLE
rem setp 222 D1 000                                          ;SET_RAPL_POWER_LIMIT_ENABLE
rem setp 222 D2 000                                          ;SET_RAPL_POWER_LIMIT_ENABLE
rem setp 127 D0 000                                          ;SET_RAPL_TIME_WINDOW
rem setp 127 D0 001                                          ;SET_RAPL_TIME_WINDOW
rem setp 127 D1 000                                          ;SET_RAPL_TIME_WINDOW
rem setp 127 D2 000                                          ;SET_RAPL_TIME_WINDOW
rem setp 241 D0 255                                          ;SET_TEMPERATURE
rem setp 241 D1 255                                          ;SET_TEMPERATURE
rem setp 241 D2 255                                          ;SET_TEMPERATURE
rem setp 047 D0 000                                          ;SET_TEMPERATURE_THRESHOLDS
rem setp 047 D0 001                                          ;SET_TEMPERATURE_THRESHOLDS
rem setp 202 D0 000                                          ;SET_TRIP_POINT_ACTIVE
rem setp 202 D0 001                                          ;SET_TRIP_POINT_ACTIVE
rem setp 202 D0 002                                          ;SET_TRIP_POINT_ACTIVE
rem setp 202 D0 003                                          ;SET_TRIP_POINT_ACTIVE
rem setp 202 D0 004                                          ;SET_TRIP_POINT_ACTIVE
rem setp 202 D0 005                                          ;SET_TRIP_POINT_ACTIVE
rem setp 202 D0 006                                          ;SET_TRIP_POINT_ACTIVE
rem setp 202 D0 007                                          ;SET_TRIP_POINT_ACTIVE
rem setp 202 D0 008                                          ;SET_TRIP_POINT_ACTIVE
rem setp 202 D0 009                                          ;SET_TRIP_POINT_ACTIVE
rem setp 203 D0 255                                          ;SET_TRIP_POINT_CRITICAL
rem setp 204 D0 255                                          ;SET_TRIP_POINT_HOT
rem setp 206 D0 255                                          ;SET_TRIP_POINT_PASSIVE
rem setp 205 D0 255                                          ;SET_TRIP_POINT_WARM
rem setp 147 D1 255                                          ;SET_TSTATE_CURRENT
echo Test Count: 122
timerstop
geterrorlevel
nolog
rem end
