#!/bin/bash
ESIF_HOME="/opt/dptf"
echo ""
echo "#########################################################################"
echo "# ESIF Installing and Setting UP DPTF/DSP Environment in $ESIF_HOME" 
echo "#########################################################################"
echo ""

if [ -d "$ESIF_HOME" ]
then
	echo "$ESIF_HOME exists refresh"
else
	echo "$ESIF_HOME creating"
	mkdir $ESIF_HOME
fi

cp esif_uf $ESIF_HOME

echo
