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

# Copy Qt to deploy directory.
cp $QTDIR/lib/libQt5Core.so.5.6.0 $OCS_DEPLOY_DIR_PATH/server/libQt5Core.so.5
cp $QTDIR/lib/libQt5Network.so.5.6.0 $OCS_DEPLOY_DIR_PATH/server/libQt5Network.so.5
cp $QTDIR/lib/libQt5WebSockets.so.5.6.0 $OCS_DEPLOY_DIR_PATH/server/libQt5WebSockets.so.5
cp $BASEDIR/projects/videoserver/res/scripts/start.sh $OCS_DEPLOY_DIR_PATH/server/videoserver.sh
cp $BASEDIR/projects/videoserver/res/scripts/start-initd.sh $OCS_DEPLOY_DIR_PATH/server/videoserver-initd.sh

# Copy LDD dependencies (DO NOT USE YET!)
if false; then

	AppBasePath=$OCS_DEPLOY_DIR_PATH/server
	AppFilePath=$OCS_DEPLOY_DIR_PATH/server/videoserver
	echo "Collecting the shared library dependencies for $AppFilePath..."
	deps=$(ldd $AppFilePath | awk 'BEGIN{ORS=" "}$1\
	~/^\//{print $1}$3~/^\//{print $3}'\
	 | sed 's/,$/\n/')
	 
	for dep in $deps
	do
		echo "Copy $dep to $AppBasePath"
	done
fi
