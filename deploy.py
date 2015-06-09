#!/bin/python
# -*- coding: utf-8 -*-
#
# DO NOT USE THIS SCRIPT YET - NOT FINISHED

import sys
import subprocess
import shutil
from pathlib import Path

g_install_dir_path = "./build/install"
g_deploy_dir_path = "./build/deploy"

def deploy_qt5(target_file_path):
    exit_code = subprocess.call(["windeployqt.exe", "--no-compiler-runtime", target_file_path])
    return exit_code


def deploy_server():
    deploy_dir = g_deploy_dir_path + "/server";

    # Prepare directories.
    if not Path(deploy_dir).exists():
        Path(deploy_dir).mkdir(parents = True)
    
    # Copy basic files.
    files = []
    files.append(g_install_dir_path + "/server/bin/win-x86-64/videoserver.exe")
    files.append(g_install_dir_path + "/server/bin/win-x86-64/default.ini")
    files.append(g_install_dir_path + "/bin/humblelogging.dll")
    for file in files:
        print("Copy file: {} => {}".format(file, deploy_dir))
        shutil.copy(file, deploy_dir)

    # Copy Qt.
    deploy_qt5(deploy_dir)
    return True


def deploy_client():
    files = []
    files.append(g_install_dir_path + "/client/bin/win-x86-64/videoclient.exe")


def deploy_ts3plugin():
    pass;
    

if __name__ == "__main__":
    deploy_server()
    pass
