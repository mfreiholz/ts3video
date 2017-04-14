#!/bin/bash
source build-linux-env.bash

# Deploy
# Copy dependencies
read -rsp $'Press any key to run "deployment"'
if [ $ARCH = "i686" ]; then
	echo "no logic yet..."
#	cp /usr/lib/i386-linux-gnu/libstdc++.so.6 $OCS_DEPLOY_DIR_PATH/server/
#	cp /lib/i386-linux-gnu/libgcc_s.so.1 $OCS_DEPLOY_DIR_PATH/server/
#	cp /lib/i386-linux-gnu/libc.so.6 $OCS_DEPLOY_DIR_PATH/server/
#	cp /lib/i386-linux-gnu/libpthread.so.0 $OCS_DEPLOY_DIR_PATH/server/
#	cp /lib/i386-linux-gnu/libdl.so.2 $OCS_DEPLOY_DIR_PATH/server/
#	cp /usr/lib/i386-linux-gnu/libgthread-2.0.so.0 $OCS_DEPLOY_DIR_PATH/server/
#	cp /lib/i386-linux-gnu/librt.so.1 $OCS_DEPLOY_DIR_PATH/server/
#	cp /lib/i386-linux-gnu/libglib-2.0.so.0 $OCS_DEPLOY_DIR_PATH/server/
#	cp /lib/i386-linux-gnu/libm.so.6 $OCS_DEPLOY_DIR_PATH/server/
#	cp /lib/ld-linux.so.2 $OCS_DEPLOY_DIR_PATH/server/ld.so
#	cp /lib/i386-linux-gnu/libpcre.so.3 $OCS_DEPLOY_DIR_PATH/server/
elif [ $ARCH = "x86_64" ]; then
	cp /usr/lib/x86_64-linux-gnu/libstdc++.so.6 $OCS_DEPLOY_DIR_PATH/server/
	cp /lib/x86_64-linux-gnu/libgcc_s.so.1 $OCS_DEPLOY_DIR_PATH/server/
	cp /lib/x86_64-linux-gnu/libc.so.6 $OCS_DEPLOY_DIR_PATH/server/
	cp /lib/x86_64-linux-gnu/libpthread.so.0 $OCS_DEPLOY_DIR_PATH/server/
	cp /lib/x86_64-linux-gnu/libm.so.6 $OCS_DEPLOY_DIR_PATH/server/
	cp /lib/x86_64-linux-gnu/libz.so.1 $OCS_DEPLOY_DIR_PATH/server/
	cp /lib/x86_64-linux-gnu/libdl.so.2 $OCS_DEPLOY_DIR_PATH/server/
	cp /lib/x86_64-linux-gnu/librt.so.1 $OCS_DEPLOY_DIR_PATH/server/
	cp /usr/lib/x86_64-linux-gnu/libgthread-2.0.so.0 $OCS_DEPLOY_DIR_PATH/server/
	cp /lib/x86_64-linux-gnu/libglib-2.0.so.0 $OCS_DEPLOY_DIR_PATH/server/
	cp /lib/x86_64-linux-gnu/libpcre.so.3 $OCS_DEPLOY_DIR_PATH/server/
	cp /lib64/ld-linux-x86-64.so.2 $OCS_DEPLOY_DIR_PATH/server/

	cp $OCS_QTDIR_X86_64/lib/libQt5Core.so.5 $OCS_DEPLOY_DIR_PATH/server/
	cp $OCS_QTDIR_X86_64/lib/libQt5Network.so.5 $OCS_DEPLOY_DIR_PATH/server/
	cp $OCS_QTDIR_X86_64/lib/libQt5WebSockets.so.5 $OCS_DEPLOY_DIR_PATH/server/
	cp $OCS_QTDIR_X86_64/lib/libicui18n.so.56 $OCS_DEPLOY_DIR_PATH/server/
	cp $OCS_QTDIR_X86_64/lib/libicuuc.so.56 $OCS_DEPLOY_DIR_PATH/server/
	cp $OCS_QTDIR_X86_64/lib/libicudata.so.56 $OCS_DEPLOY_DIR_PATH/server/
fi

cp $BASEDIR/projects/videoserver/res/scripts/videoserver.sh $OCS_DEPLOY_DIR_PATH/server/
cp $BASEDIR/projects/videoserver/res/scripts/videoserver-initd.sh $OCS_DEPLOY_DIR_PATH/server/
cp $BASEDIR/projects/videoserver/res/default.ini $OCS_DEPLOY_DIR_PATH/server/default.ini.orig
cp $BASEDIR/projects/videoserver/res/logging.conf $OCS_DEPLOY_DIR_PATH/server/logging.conf.orig

cp -rf $BASEDIR/projects/serverstatus $OCS_DEPLOY_DIR_PATH/server/

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
