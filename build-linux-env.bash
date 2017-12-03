#!/bin/bash
#
# # Read line example
# read -rsp $'Press any key to run "make install"'
#

# Ugly way to get absolute path of this directory
pushd `dirname $0` > /dev/null
SCRIPTPATH=`pwd`
popd > /dev/null

# List of paths
BASEDIR=$SCRIPTPATH
ARCH=$(uname -m)

export OCS_SOURCE_DIR_PATH=$BASEDIR
export OCS_BUILD_DIR_PATH=$BASEDIR/build
export OCS_INSTALL_DIR_PATH=$OCS_BUILD_DIR_PATH/install

# Print used environment
echo
echo OCS Build Environment
echo ---------------------
echo OCS_SOURCE_DIR_PATH  : $OCS_SOURCE_DIR_PATH
echo OCS_BUILD_DIR_PATH   : $OCS_BUILD_DIR_PATH
echo OCS_INSTALL_DIR_PATH : $OCS_INSTALL_DIR_PATH
echo ARCH                 : $ARCH
echo ---------------------
echo

cd $OCS_SOURCE_DIR_PATH

# Prepare build directory
if [ ! -d "$OCS_BUILD_DIR_PATH" ]; then
	mkdir $OCS_BUILD_DIR_PATH
fi

# Prepare install directory
if [ ! -d "$OCS_INSTALL_DIR_PATH" ]; then
	mkdir $OCS_INSTALL_DIR_PATH
fi
