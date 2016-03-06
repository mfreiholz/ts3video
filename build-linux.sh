#!/bin/bash

# Ugly way to get absolute path of this directory
pushd `dirname $0` > /dev/null
SCRIPTPATH=`pwd`
popd > /dev/null

# List of paths
BASEDIR=$SCRIPTPATH
export OCS_BUILD_DIR_PATH=$BASEDIR/build
export OCS_DEPLOY_DIR_PATH=$OCS_BUILD_DIR_PATH/deploy

# Print used environment
echo
echo OCS Build Environment
echo ---------------------
echo OCS_BUILD_DIR_PATH  : $OCS_BUILD_DIR_PATH
echo OCS_DEPLOY_DIR_PATH : $OCS_DEPLOY_DIR_PATH
echo ---------------------
echo 

# Run CMake
cd $BASEDIR
mkdir $OCS_BUILD_DIR_PATH
cd $OCS_BUILD_DIR_PATH
cmake -DCMAKE_INSTALL_PREFIX="$OCS_DEPLOY_DIR_PATH" -DCMAKE_BUILD_TYPE=Release ..

# Run Make + Install
make && make install

