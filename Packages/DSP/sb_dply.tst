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
rem ETF Package: sb_dply
rem     Format: ESIF Shell Script
rem     Version: 1.0
rem
rem =====                                  ======
rem ===== AUTO GENERATED FEEL FREE TO EDIT ======
rem =====                                  ======
rem
log sb_dply.log
info
seterrorlevel 0
timerstart
setb 8192
getp 185 D0 255                                              ;GET_CLOCK_COUNT
getp 187 D0 000                                              ;GET_CLOCK_ORIGINAL_FREQUENCY
getp 187 D0 001                                              ;GET_CLOCK_ORIGINAL_FREQUENCY
getp 187 D0 002                                              ;GET_CLOCK_ORIGINAL_FREQUENCY
getp 187 D0 003                                              ;GET_CLOCK_ORIGINAL_FREQUENCY
getp 187 D0 004                                              ;GET_CLOCK_ORIGINAL_FREQUENCY
getp 191 D0 255                                              ;GET_CLOCK_SPREAD_DIRECTION
getp 190 D0 255 -u 1000 -l 0                                 ;GET_CLOCK_SPREAD_PERCENTAGE
getp 192 D0 000 -u 3 -l 1                                    ;GET_CLOCK_SPREAD_SUBHARMONICS
getp 189 D0 000                                              ;GET_CLOCK_SSC_DISABLED_FREQUENCY
getp 189 D0 001                                              ;GET_CLOCK_SSC_DISABLED_FREQUENCY
getp 189 D0 002                                              ;GET_CLOCK_SSC_DISABLED_FREQUENCY
getp 189 D0 003                                              ;GET_CLOCK_SSC_DISABLED_FREQUENCY
getp 189 D0 004                                              ;GET_CLOCK_SSC_DISABLED_FREQUENCY
getp 188 D0 000                                              ;GET_CLOCK_SSC_ENABLED_FREQUENCY
getp 188 D0 001                                              ;GET_CLOCK_SSC_ENABLED_FREQUENCY
getp 188 D0 002                                              ;GET_CLOCK_SSC_ENABLED_FREQUENCY
getp 188 D0 003                                              ;GET_CLOCK_SSC_ENABLED_FREQUENCY
getp 188 D0 004                                              ;GET_CLOCK_SSC_ENABLED_FREQUENCY
getp 053 D0 255 -u 0xffffffff -l 0                           ;GET_DEVICE_HARDWARE_ID
getp 088 D0 255 -u 0xf -l 0                                  ;GET_DEVICE_STATUS
getp 067 D0 255 -u 0xffffffff -l 0                           ;GET_DEVICE_UNIQUE_ID
getp 158 D0 255 -b sb_dply_158_D0_255.bin                    ;GET_DISPLAY_BRIGHTNESS_LEVELS
getp 157 D0 255 -u 100 -l 0                                  ;GET_DISPLAY_BRIGHTNESS_SOFT
getp 159 D0 255 -b sb_dply_159_D0_255.bin                    ;GET_DISPLAY_CAPABILITY
getp 181 D0 255 -u 1000 -l 0                                 ;GET_DISPLAY_CLOCK_DEVIATION
getp 160 D0 255                                              ;GET_DISPLAY_DEPTH_LIMIT
getp 179 D0 255 -u 2 -l 0                                    ;GET_DISPLAY_PANEL_TYPE
getp 182 D0 255 -u 1 -l 0                                    ;GET_GRAPHICS_CHIPSET_CHANNEL_TYPE
getp 184 D0 255 -u 1 -l 0                                    ;GET_GRAPHICS_CHIPSET_SSC_ENABLED
rem setp 194 D0 000                                          ;SET_CLOCK_SSC_DISABLED
rem setp 194 D0 001                                          ;SET_CLOCK_SSC_DISABLED
rem setp 194 D0 002                                          ;SET_CLOCK_SSC_DISABLED
rem setp 194 D0 003                                          ;SET_CLOCK_SSC_DISABLED
rem setp 194 D0 004                                          ;SET_CLOCK_SSC_DISABLED
rem setp 193 D0 000                                          ;SET_CLOCK_SSC_ENABLED
rem setp 193 D0 001                                          ;SET_CLOCK_SSC_ENABLED
rem setp 193 D0 002                                          ;SET_CLOCK_SSC_ENABLED
rem setp 193 D0 003                                          ;SET_CLOCK_SSC_ENABLED
rem setp 193 D0 004                                          ;SET_CLOCK_SSC_ENABLED
rem setp 226 D0 255                                          ;SET_DISPLAY_BRIGHTNESS_HARD
rem setp 163 D0 255                                          ;SET_DISPLAY_BRIGHTNESS_SOFT
echo Test Count: 42
timerstop
geterrorlevel
nolog
rem end
