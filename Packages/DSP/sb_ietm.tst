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
rem ETF Package: sb_ietm
rem     Format: ESIF Shell Script
rem     Version: 1.0
rem
rem =====                                  ======
rem ===== AUTO GENERATED FEEL FREE TO EDIT ======
rem =====                                  ======
rem
log sb_ietm.log
info
seterrorlevel 0
timerstart
setb 8192
getp 089 D0 255 -b sb_ietm_89_D0_255.bin                     ;GET_ACTIVE_RELATIONSHIP_TABLE
getp 094 D0 255                                              ;GET_CURRENT_LOW_POWER_MODE
getp 053 D0 255 -u 0xffffffff -l 0                           ;GET_DEVICE_HARDWARE_ID
getp 088 D0 255 -u 0xf -l 0                                  ;GET_DEVICE_STATUS
getp 237 D0 255 -b sb_ietm_237_D0_255.bin                    ;GET_LPM_TABLE
getp 092 D0 255 -b sb_ietm_92_D0_255.bin                     ;GET_SUPPORTED_POLICIES
getp 091 D0 255 -b sb_ietm_91_D0_255.bin                     ;GET_THERMAL_RELATIONSHIP_TABLE
rem setp 230 D0 255                                          ;SET_ACTIVE_RELATIONSHIP_TABLE
rem setp 236 D0 255                                          ;SET_CURRENT_LOW_POWER_MODE
rem setp 238 D0 255                                          ;SET_LPM_TABLE
rem setp 093 D0 255                                          ;SET_OPERATING_SYSTEM_CAPABILITIES
rem setp 175 D0 255                                          ;SET_SYSTEM_HIBERNATE
rem setp 173 D0 255                                          ;SET_SYSTEM_SHUTDOWN
rem setp 174 D0 255                                          ;SET_SYSTEM_SLEEP
rem setp 231 D0 255                                          ;SET_THERMAL_RELATIONSHIP_TABLE
echo Test Count: 15
timerstop
geterrorlevel
nolog
rem end
