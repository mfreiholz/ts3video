#!/bin/bash
source build-linux-env.bash

cd $OCS_BUILD_DIR_PATH
cmake -DCMAKE_INSTALL_PREFIX="$OCS_INSTALL_DIR_PATH" -DCMAKE_BUILD_TYPE=Debug -DIncludeOpenGLSupport=OFF -DIncludeAudioSupport=OFF -DIncludeClientPrograms=OFF ..
