################################################################################
## This file is provided under a dual BSD/GPLv2 license.  When using or 
## redistributing this file, you may do so under either license.
##
## GPL LICENSE SUMMARY
##
## Copyright (c) 2013 Intel Corporation All Rights Reserved
##
## This program is free software; you can redistribute it and/or modify it under 
## the terms of version 2 of the GNU General Public License as published by the
## Free Software Foundation.
##
## This program is distributed in the hope that it will be useful, but WITHOUT
## ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
## FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more 
## details.
##
## You should have received a copy of the GNU General Public License along with
## this program; if not, write to the Free Software  Foundation, Inc., 
## 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
## The full GNU General Public License is included in this distribution in the
## file called LICENSE.GPL.
##
## BSD LICENSE 
##
## Copyright (c) 2013 Intel Corporation All Rights Reserved
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions are met:
##
## * Redistributions of source code must retain the above copyright notice, this
##   list of conditions and the following disclaimer.
## * Redistributions in binary form must reproduce the above copyright notice,
##   this list of conditions and the following disclaimer in the documentation 
##   and/or other materials provided with the distribution.
## * Neither the name of Intel Corporation nor the names of its contributors may
##   be used to endorse or promote products derived from this software without
##   specific prior written permission.
##
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
## AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
## IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
## ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
## LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,  SPECIAL, EXEMPLARY, OR
## CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
## SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
## INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
## CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
## ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
## POSSIBILITY OF SUCH DAMAGE.
##
################################################################################

#!/bin/bash
ESIF_HOME="/opt/dptf"
echo ""
echo "#########################################################################"
echo "# ESIF Installing and Setting UP DPTF/DSP Environment in $ESIF_HOME" 
echo "#########################################################################"
echo ""
ESIF_HOME_BIN="$ESIF_HOME/esif_bin"
ESIF_HOME_CMD="$ESIF_HOME/esif_cmd"
ESIF_HOME_CPC="$ESIF_HOME/esif_cpc"
ESIF_HOME_FPC="$ESIF_HOME/esif_fpc"
ESIF_HOME_DSP="$ESIF_HOME/esif_dsp"
ESIF_HOME_LOG="$ESIF_HOME/esif_log"
ESIF_HOME_TST="$ESIF_HOME/esif_tst"

if [ -d "$ESIF_HOME" ]
then
	echo "$ESIF_HOME exists refresh"
else
	echo "$ESIF_HOME creating"
	mkdir $ESIF_HOME
fi

if [ -d "$ESIF_HOME_BIN" ]
then
	echo "$ESIF_HOME_BIN exists refresh"
else
	echo "$ESIF_HOME_BIN creating"
	mkdir $ESIF_HOME_BIN 
fi

if [ -d "$ESIF_HOME_CMD" ]
then
	echo "$ESIF_HOME_CMD exists refresh"
else
	echo "$ESIF_HOME_CMD creating"
	mkdir $ESIF_HOME_CMD 
fi

if [ -d "$ESIF_HOME_CPC" ]
then
	echo "$ESIF_HOME_CPC exists refresh"
else
	echo "$ESIF_HOME_CPC creating"
	mkdir $ESIF_HOME_CPC 
fi

if [ -d "$ESIF_HOME_FPC" ]
then
	echo "$ESIF_HOME_FPC exists refresh"
else
	echo "$ESIF_HOME_FPC creating"
	mkdir $ESIF_HOME_FPC 
fi

if [ -d "$ESIF_HOME_DSP" ]
then
	echo "$ESIF_HOME_DSP exists refresh"
else
	echo "$ESIF_HOME_DSP creating"
	mkdir $ESIF_HOME_DSP
fi

if [ -d "$ESIF_HOME_LOG" ]
then
	echo "$ESIF_HOME_LOG exists refresh"
else
	echo "$ESIF_HOME_LOG creating"
	mkdir $ESIF_HOME_LOG
fi

if [ -d "$ESIF_HOME_TST" ]
then
	echo "$ESIF_HOME_TST exists refresh"
else
	echo "$ESIF_HOME_TST creating"
	mkdir $ESIF_HOME_TST
fi

cp esif_lf.ko $ESIF_HOME
cp dptf_cpu.ko $ESIF_HOME
cp dptf_pch.ko $ESIF_HOME
cp dptf_plat.ko $ESIF_HOME
cp dptf_acpi.ko $ESIF_HOME

cp ../../../../Packages/DSP/REF/*.ref $ESIF_HOME_CMD
cp ../../../../Packages/DSP/REF/rapl $ESIF_HOME_CMD
cp ../../../../Packages/DSP/REF/dppm $ESIF_HOME_CMD
cp ../../../../Packages/DSP/CPC/*.cpc $ESIF_HOME_CPC
cp ../../../../Packages/DSP/FPC/*.fpc $ESIF_HOME_FPC
cp ../../../../Packages/DSP/TST/*.tst $ESIF_HOME_TST
cp ../../../../Packages/DSP/*.dsp $ESIF_HOME_DSP

echo
