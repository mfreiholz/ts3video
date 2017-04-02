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
export OCS_BUILD_DIR_PATH=$BASEDIR/build
export OCS_DEPLOY_DIR_PATH=$OCS_BUILD_DIR_PATH/deploy

# Print used environment
echo
echo OCS Build Environment
echo ---------------------
echo BASEDIR             : $BASEDIR
echo OCS_BUILD_DIR_PATH  : $OCS_BUILD_DIR_PATH
echo OCS_DEPLOY_DIR_PATH : $OCS_DEPLOY_DIR_PATH
echo ARCH                : $ARCH
echo ---------------------
echo

cd $BASEDIR

# Prepare build directory
if [ ! -d "$OCS_BUILD_DIR_PATH" ]; then
	mkdir $OCS_BUILD_DIR_PATH
fi

# Prepare deploy directory
if [ ! -d "$OCS_DEPLOY_DIR_PATH" ]; then
	mkdir $OCS_DEPLOY_DIR_PATH
fi