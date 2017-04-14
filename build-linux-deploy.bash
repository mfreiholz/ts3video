#!/bin/bash
source build-linux-env.bash

cd $OCS_BUILD_DIR_PATH

# Prepare binaries and DLLs.
make -j4 install

# Copy dependencies
if [ $ARCH = "i686" ]; then

	echo "Not yet supported."

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

	# System (Ubuntu 16.04 LTS)
	cp --dereference --update /usr/lib/x86_64-linux-gnu/libstdc++.so.6 $OCS_DEPLOY_DIR_PATH/server/
	cp --dereference --update /lib/x86_64-linux-gnu/libgcc_s.so.1 $OCS_DEPLOY_DIR_PATH/server/
	cp --dereference --update /lib/x86_64-linux-gnu/libc.so.6 $OCS_DEPLOY_DIR_PATH/server/
	cp --dereference --update /lib/x86_64-linux-gnu/libpthread.so.0 $OCS_DEPLOY_DIR_PATH/server/
	cp --dereference --update /lib/x86_64-linux-gnu/libm.so.6 $OCS_DEPLOY_DIR_PATH/server/
	cp --dereference --update /lib/x86_64-linux-gnu/libz.so.1 $OCS_DEPLOY_DIR_PATH/server/
	cp --dereference --update /lib/x86_64-linux-gnu/libdl.so.2 $OCS_DEPLOY_DIR_PATH/server/
	cp --dereference --update /lib/x86_64-linux-gnu/librt.so.1 $OCS_DEPLOY_DIR_PATH/server/
	cp --dereference --update /usr/lib/x86_64-linux-gnu/libgthread-2.0.so.0 $OCS_DEPLOY_DIR_PATH/server/
	cp --dereference --update /lib/x86_64-linux-gnu/libglib-2.0.so.0 $OCS_DEPLOY_DIR_PATH/server/
	cp --dereference --update /lib/x86_64-linux-gnu/libpcre.so.3 $OCS_DEPLOY_DIR_PATH/server/
	cp --dereference --update /lib64/ld-linux-x86-64.so.2 $OCS_DEPLOY_DIR_PATH/server/

	# Qt 5.8.0
	cp --dereference --update $OCS_QTDIR_X86_64/lib/libQt5Core.so.5 $OCS_DEPLOY_DIR_PATH/server/
	cp --dereference --update $OCS_QTDIR_X86_64/lib/libQt5Network.so.5 $OCS_DEPLOY_DIR_PATH/server/
	cp --dereference --update $OCS_QTDIR_X86_64/lib/libQt5WebSockets.so.5 $OCS_DEPLOY_DIR_PATH/server/
	cp --dereference --update $OCS_QTDIR_X86_64/lib/libicui18n.so.56 $OCS_DEPLOY_DIR_PATH/server/
	cp --dereference --update $OCS_QTDIR_X86_64/lib/libicuuc.so.56 $OCS_DEPLOY_DIR_PATH/server/
	cp --dereference --update $OCS_QTDIR_X86_64/lib/libicudata.so.56 $OCS_DEPLOY_DIR_PATH/server/

fi

# VideoServer configs and scripts
rm $OCS_DEPLOY_DIR_PATH/server/*.ini $OCS_DEPLOY_DIR_PATH/server/*.conf
cp --dereference --update $OCS_BUILD_DIR_PATH/projects/videoserver/videoserver $OCS_DEPLOY_DIR_PATH/server/
cp --dereference --update $BASEDIR/projects/videoserver/res/scripts/videoserver.sh $OCS_DEPLOY_DIR_PATH/server/
cp --dereference --update $BASEDIR/projects/videoserver/res/scripts/videoserver-initd.sh $OCS_DEPLOY_DIR_PATH/server/
cp --dereference --update $BASEDIR/projects/videoserver/res/default.ini $OCS_DEPLOY_DIR_PATH/server/default.ini.orig
cp --dereference --update $BASEDIR/projects/videoserver/res/logging.conf $OCS_DEPLOY_DIR_PATH/server/logging.conf.orig
cp --dereference --update -r -f $BASEDIR/projects/serverstatus $OCS_DEPLOY_DIR_PATH/server/