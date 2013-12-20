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
rem ETF Package: sb_wgig
rem     Format: ESIF Shell Script
rem     Version: 1.0
rem
rem =====                                  ======
rem ===== AUTO GENERATED FEEL FREE TO EDIT ======
rem =====                                  ======
rem
log sb_wgig.log
info
seterrorlevel 0
timerstart
setb 8192
getp 060 D0 255 -s sb_wgig_60_D0_255.str                     ;GET_DEVICE_DESCRIPTION
getp 053 D0 255 -u 0xffffffff -l 0                           ;GET_DEVICE_HARDWARE_ID
getp 067 D0 255 -u 0xffffffff -l 0                           ;GET_DEVICE_UNIQUE_ID
getp 139 D0 255 -u 18 -l 0                                   ;GET_PARTICIPANT_TYPE
getp 014 D0 255 -u 105 -l 0                                  ;GET_TEMPERATURE
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
rem setp 241 D0 255                                          ;SET_TEMPERATURE
rem setp 232 D0 255                                          ;SET_TEMPERATURE_THRESHOLD_HYSTERESIS
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
echo Test Count: 41
timerstop
geterrorlevel
nolog
rem end
