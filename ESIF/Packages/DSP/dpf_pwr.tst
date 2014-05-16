rem ##############################################################################
rem ## Copyright (c) 2014 Intel Corporation All Rights Reserved
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
rem ETF Package: dpf_pwr
rem     Format: ESIF Shell Script
rem     Version: 1.0
rem
rem =====                                  ======
rem ===== AUTO GENERATED FEEL FREE TO EDIT ======
rem =====                                  ======
rem
log dpf_pwr.log
info
seterrorlevel 0
timerstart
setb 8192
getp 223 D0 255                                              ;GET_BATTERY_STATUS
getp 060 D0 255 -s dpf_pwr_60_D0_255.str                     ;GET_DEVICE_DESCRIPTION
getp 053 D0 255 -u 0xffffffff -l 0                           ;GET_DEVICE_HARDWARE_ID
getp 067 D0 255 -u 0xffffffff -l 0                           ;GET_DEVICE_UNIQUE_ID
getp 279 D0 255                                              ;GET_DYNAMIC_BATTERY_POWER_ACTION
getp 139 D0 255 -u 27 -l 0                                   ;GET_PARTICIPANT_TYPE
getp 278 D0 255 -u 100 -l 0                                  ;GET_PLATFORM_STATE_OF_CHARGE
echo Test Count: 7
timerstop
geterrorlevel
nolog
rem end
