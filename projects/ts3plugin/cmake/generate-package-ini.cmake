#
# Script to generate ts3_plugin's package.ini
# The script requires some parameters:
# - VERSION_MAJOR
# - VERSION_MINOR
# - CMAKE_INSTALL_PREFIX
#

message("Generate package.ini (major=${VERSION_MAJOR}; minor=${VERSION_MINOR})")
file(WRITE
  ${CMAKE_INSTALL_PREFIX}/ts3_plugin/package.ini
  "Name = TS3VIDEO\nType = Plugin\nAuthor = Manuel Freiholz\nVersion = ${VERSION_MAJOR}.${VERSION_MINOR}\nPlatforms = win32,win64\nDescription = ATTENTION for Updaters:\\nDeactivate the previous installed version of TS3VIDEO in the Settings->Plugins dialog before you continue with the installation.\n"
)