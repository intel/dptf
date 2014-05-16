# Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EAPI="4"
EGIT_REPO_URI="git://github.com/01org/dptf.git"
EGIT_COMMIT="3d767c13b9567a2106691f86519c205874422fcd"

inherit git-2 toolchain-funcs

DESCRIPTION="Shared library build for Dynamic Platform & Thermal Framework"
HOMEPAGE=""

LICENSE="Apache 2.0"
SLOT="0"
KEYWORDS="amd64 ~x86"

DEPEND="dev-util/cmake"
RDEPEND="sys-power/dptfd
	sys-power/dptf-kmod"

CMAKE_DIR=Products/DPTF/Linux/build
#TARGET_SRC_DIR=$CMAKE_DIR/x64/release
DPTF_DST_DIR=usr/lib64
POLICY_DST_DIR=usr/share/dptf

src_configure() {
	cd ${CMAKE_DIR}
	builddir="${CMAKE_DIR}"
	buildconf=" -DCHROMIUM_BUILD=YES"

	if use x86 ; then
		buildconf="${buildconf} -DBUILD_ARCH=32bit"
		builddir="${builddir}/x32"
	fi

	if use amd64 ; then
		buildconf="${buildconf} -DBUILD_ARCH=64bit"
		builddir="${builddir}/x64"
	fi

	if use debug ; then
		buildconf="${buildconf} -DCMAKE_BUILD_TYPE=Debug .."
		builddir="${builddir}/debug"
	else
	 	buildconf="${buildconf} -DCMAKE_BUILD_TYPE=Release .."
		builddir="${builddir}/release"
	fi

	cmake ${buildconf}
}

src_compile() {
	cd ${CMAKE_DIR}
	emake || die "Make failed!"
}

src_install() {
	insinto ${DPTF_DST_DIR}
	doins ${builddir}/Dptf.so
	insinto ${POLICY_DST_DIR}
	doins ${builddir}/DptfPolicy*.so
}

