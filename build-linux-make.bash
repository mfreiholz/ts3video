#!/bin/bash
source build-linux-env.bash

cd $OCS_BUILD_DIR_PATH

make -j

# Call make with parameter, e.g. "install"
if [ $# -gt 0 ]; then
	echo $1
	make -j $1
fi
