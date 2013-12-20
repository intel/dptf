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
rem ETF Package: sb_tcpu
rem     Format: ESIF Shell Script
rem     Version: 1.0
rem
rem =====                                  ======
rem ===== AUTO GENERATED FEEL FREE TO EDIT ======
rem =====                                  ======
rem
log sb_tcpu.log
info
seterrorlevel 0
timerstart
setb 8192
getp 053 D0 255 -u 0xffffffff -l 0                           ;GET_DEVICE_HARDWARE_ID
getp 088 D0 255 -u 0xf -l 0                                  ;GET_DEVICE_STATUS
getp 067 D0 255 -u 0xffffffff -l 0                           ;GET_DEVICE_UNIQUE_ID
getp 178 D1 255 -u 3 -l 0                                    ;GET_DOMAIN_PRIORITY
getp 178 D2 255 -u 3 -l 0                                    ;GET_DOMAIN_PRIORITY
getp 054 D0 255 -u 105 -l 0                                  ;GET_NOTIFICATION_TEMP_THRESHOLD
getp 137 D2 255 -b sb_tcpu_137_D2_255.bin                    ;GET_PERF_SUPPORT_STATES
getp 068 D1 255                                              ;GET_PROC_APPLICATION_EXCLUDE_LIST
getp 069 D1 255 -b sb_tcpu_69_D1_255.bin                     ;GET_PROC_CURRENT_LOGICAL_PROCESSOR_OFFLINING
getp 056 D1 255                                              ;GET_PROC_PERF_PRESENT_CAPABILITY
getp 055 D1 255 -u 100 -l 0                                  ;GET_PROC_PERF_PSTATE_DEPTH_LIMIT
getp 095 D1 255 -b sb_tcpu_95_D1_255.bin                     ;GET_PROC_PERF_SUPPORT_STATES
getp 058 D0 255 -b sb_tcpu_58_D0_255.bin                     ;GET_PROC_PERF_THROTTLE_CONTROL
getp 062 D1 255                                              ;GET_PROC_PERF_THROTTLE_PRESENT_CAPABILITY
getp 061 D1 255                                              ;GET_PROC_PERF_TSTATE_DEPTH_LIMIT
getp 020 D0 255 -u 105 -l 0                                  ;GET_PROC_TJMAX
getp 218 D0 255 -u 1 -l 0                                    ;GET_PROC_TURBO_STATE
getp 128 D0 255                                              ;GET_RAPL_ENERGY
getp 128 D1 255                                              ;GET_RAPL_ENERGY
getp 123 D0 255                                              ;GET_RAPL_ENERGY_UNIT
getp 035 D0 255                                              ;GET_RAPL_POWER
getp 035 D1 255                                              ;GET_RAPL_POWER
getp 035 D2 255                                              ;GET_RAPL_POWER
getp 075 D0 255 -b sb_tcpu_75_D0_255.bin                     ;GET_RAPL_POWER_CONTROL_CAPABILITIES
getp 038 D0 000                                              ;GET_RAPL_POWER_LIMIT
getp 126 D0 000 -u 1 -l 0                                    ;GET_RAPL_POWER_LIMIT_ENABLE
getp 124 D0 255 -u 3 -l 3                                    ;GET_RAPL_POWER_UNIT
getp 122 D0 255 -u 10 -l 10                                  ;GET_RAPL_TIME_UNIT
getp 039 D0 000                                              ;GET_RAPL_TIME_WINDOW
getp 014 D0 255 -u 105 -l 0                                  ;GET_TEMPERATURE
getp 014 D1 255 -u 105 -l 0                                  ;GET_TEMPERATURE
getp 014 D2 255 -u 105 -l 0                                  ;GET_TEMPERATURE
getp 014 D3 255 -u 105 -l 0                                  ;GET_TEMPERATURE
getp 014 D4 255 -u 105 -l 0                                  ;GET_TEMPERATURE
getp 072 D0 255 -u 2 -l 2                                    ;GET_TEMPERATURE_THRESHOLD_COUNT
getp 015 D0 255 -u 105 -l 0                                  ;GET_TEMPERATURE_THRESHOLD_HYSTERESIS
getp 143 D0 000                                              ;GET_TEMPERATURE_THRESHOLDS
getp 143 D0 001                                              ;GET_TEMPERATURE_THRESHOLDS
getp 013 D0 255 -u 105 -l 0                                  ;GET_TRIP_POINT_CRITICAL
getp 012 D0 255 -u 105 -l 0                                  ;GET_TRIP_POINT_HOT
getp 011 D0 255 -u 105 -l 0                                  ;GET_TRIP_POINT_PASSIVE
getp 177 D0 255 -u 105 -l 0                                  ;GET_TRIP_POINT_WARM
getp 016 D1 255                                              ;GET_TSTATE_CURRENT
getp 065 D1 255 -b sb_tcpu_65_D1_255.bin                     ;GET_TSTATES
rem setp 153 D1 255                                          ;SET_ACTIVE_CORE_LIMIT
rem setp 081 D0 255                                          ;SET_COOLING_POLICY
rem setp 051 D0 255                                          ;SET_DEVICE_TEMPERATURE_INDICATION
rem setp 082 D1 255                                          ;SET_PERF_PRESENT_CAPABILITY
rem setp 082 D2 255                                          ;SET_PERF_PRESENT_CAPABILITY
rem setp 100 D0 255                                          ;SET_PROC_TURBO_STATE
rem setp 130 D0 000                                          ;SET_RAPL_POWER_LIMIT
rem setp 222 D0 000                                          ;SET_RAPL_POWER_LIMIT_ENABLE
rem setp 127 D0 000                                          ;SET_RAPL_TIME_WINDOW
rem setp 241 D0 255                                          ;SET_TEMPERATURE
rem setp 241 D1 255                                          ;SET_TEMPERATURE
rem setp 241 D2 255                                          ;SET_TEMPERATURE
rem setp 241 D3 255                                          ;SET_TEMPERATURE
rem setp 241 D4 255                                          ;SET_TEMPERATURE
rem setp 047 D0 000                                          ;SET_TEMPERATURE_THRESHOLDS
rem setp 047 D0 001                                          ;SET_TEMPERATURE_THRESHOLDS
rem setp 203 D0 255                                          ;SET_TRIP_POINT_CRITICAL
rem setp 204 D0 255                                          ;SET_TRIP_POINT_HOT
rem setp 206 D0 255                                          ;SET_TRIP_POINT_PASSIVE
rem setp 205 D0 255                                          ;SET_TRIP_POINT_WARM
rem setp 147 D1 255                                          ;SET_TSTATE_CURRENT
echo Test Count: 65
timerstop
geterrorlevel
nolog
rem end
