#!/bin/bash
source build-linux-env.bash

cd $OCS_BUILD_DIR_PATH
make -j

read -rsp $'Press any key to run "make install"'
make install