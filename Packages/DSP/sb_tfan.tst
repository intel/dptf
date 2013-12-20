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
rem ETF Package: sb_tfan
rem     Format: ESIF Shell Script
rem     Version: 1.0
rem
rem =====                                  ======
rem ===== AUTO GENERATED FEEL FREE TO EDIT ======
rem =====                                  ======
rem
log sb_tfan.log
info
seterrorlevel 0
timerstart
setb 8192
getp 060 D0 255 -s sb_tfan_60_D0_255.str                     ;GET_DEVICE_DESCRIPTION
getp 053 D0 255 -u 0xffffffff -l 0                           ;GET_DEVICE_HARDWARE_ID
getp 088 D0 255 -u 0xf -l 0                                  ;GET_DEVICE_STATUS
getp 067 D0 255 -u 0xffffffff -l 0                           ;GET_DEVICE_UNIQUE_ID
getp 084 D0 255 -b sb_tfan_84_D0_255.bin                     ;GET_FAN_INFORMATION
getp 085 D0 255 -b sb_tfan_85_D0_255.bin                     ;GET_FAN_PERFORMANCE_STATES
getp 086 D0 255                                              ;GET_FAN_STATUS
rem setp 087 D0 255                                          ;SET_FAN_LEVEL
echo Test Count: 8
timerstop
geterrorlevel
nolog
rem end
