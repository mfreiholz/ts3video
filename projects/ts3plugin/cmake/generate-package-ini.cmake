#
# Script to generate ts3_plugin's package.ini
# The script requires some parameters:
# - VERSION_MAJOR
# - VERSION_MINOR
# - PACKAGE_INI_DIR_PATH
#

message("Generate package.ini (major=${VERSION_MAJOR}; minor=${VERSION_MINOR})")
file(WRITE
  ${PACKAGE_INI_DIR_PATH}/package.ini
  "Name = TS3VIDEO\nType = Plugin\nAuthor = Manuel Freiholz\nVersion = ${VERSION_MAJOR}.${VERSION_MINOR}\nPlatforms = win32,win64\nDescription = ATTENTION for Updaters:\\nDeactivate the previous installed version of TS3VIDEO in the Settings->Plugins dialog before you continue with the installation.\n"
)