#
# Script to execute the "windeployqt.exe" for a specific binary during build.
#
# Required parameters:
#	BINARY_PATH
#		Path to the binary (exe, dll) for which to call windeployqt.
#	QT_DIR_PATH
#		Path to the "bin" directory of Qt
#

set(ENV{QTDIR} "${QT_DIR_PATH}")
set(ENV{PATH} "${QT_DIR_PATH};$ENV{PATH}")

message("Executing WINDEPLOYQT
	BINARY_PATH=${BINARY_PATH}
	QT_DIR_PATH=${QT_DIR_PATH}
	PATH=$ENV{PATH}
")

execute_process(
	COMMAND windeployqt.exe --release ${BINARY_PATH}
	WORKING_DIRECTORY ${QT_DIR_PATH}/bin
)