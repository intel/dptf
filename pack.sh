#!/bin/bash

CHROMIUM_SDK_ROOT=`pwd`
BUILD_DIRECTORY=./chrome_build

echo -e "Chromium SDK located in $CHROMIUM_SDK_ROOT\n"
echo -e "Build output directory located in $BUILD_DIRECTORY\n"

if [ -d "$BUILD_DIRECTORY" ]; then
	echo "Removing existing build directory..."
	rm -fr $BUILD_DIRECTORY
	echo -e "Done.\n"
fi

echo "Creating directory structure..."
mkdir $BUILD_DIRECTORY
cd $BUILD_DIRECTORY
mkdir drivers
mkdir log
mkdir ufx64
mkdir cmd
mkdir dsp
echo -e "Done.\n"

echo "Copying ESIF binaries..."
cp $CHROMIUM_SDK_ROOT/Products/ESIF_LF/Linuxx64Atom/Debug/esif_lf.ko ./drivers/
cp $CHROMIUM_SDK_ROOT/Products/ESIF_LF/Linuxx64Atom/Debug/dptf_acpi.ko ./drivers/
cp $CHROMIUM_SDK_ROOT/Products/ESIF_LF/Linuxx64Atom/Debug/dptf_cpu.ko ./drivers/
cp $CHROMIUM_SDK_ROOT/Products/ESIF_LF/Linuxx64Atom/Debug/dptf_pch.ko ./drivers/
cp $CHROMIUM_SDK_ROOT/Products/ESIF_UF/Chrome/Debug/esif_ufd ./ufx64/
cp $CHROMIUM_SDK_ROOT/Packages/DSP/*.edp ./dsp/
echo -e "Done.\n"

echo "Copying DPTF binaries..."
cp $CHROMIUM_SDK_ROOT/Products/DPTF/Linux/build/x64/Dptf.so ./ufx64
cp $CHROMIUM_SDK_ROOT/Products/DPTF/Linux/build/x64/DptfPolicyPassive.so ./ufx64
cp $CHROMIUM_SDK_ROOT/Products/DPTF/Linux/build/x64/DptfPolicyCritical.so ./ufx64
cp $CHROMIUM_SDK_ROOT/Products/DPTF/Sources/Resources/combined.xsl ./ufx64
echo -e "Done.\n"

echo "Copying start script..."
cp $CHROMIUM_SDK_ROOT/Packages/Installers/chrome/start ./cmd
echo -e "Done.\n"

echo "Target kernel is `modinfo -F vermagic ./drivers/esif_lf.ko`."

cd ..
tar zcvf dptf_chrome_build.`date +%H%M.%F`.tar.gz $BUILD_DIRECTORY
