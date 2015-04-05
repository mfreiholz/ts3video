<!--
title: Documentation
-->
# Documentation

* [Client](#client)
  * Installation

* [Server](#server)
  * Installation
  * Configuration
  * Run as Windows Service
  * Run as Linux Daemon


# Client

## Installation
After downloading the plugin from the website you can simply double click the *.ts3\_plugin file, which will start the Teamspeak plugin installer.
_It's highly recommended to close Teamspeak before trying to install or update the plugin._

![X](img/docs/client-plugin-install.png)

__Linux:__ The client plugin for Linux is in development.

__Mac:__ I still don't have a Mac.


# Server

## Installation
The downloaded server package is usually a simple ZIP archive, which can be extracted wherever you want it.

__Windows:__ `C:\ts3video-server`

__Linux:__ `/opt/ts3video-server`

## Configuration
The server provides multiple ways of configurations before startup. You can configure it by appending commandline arguments directly or by passing a configuration file during startup.

### Commandline parameters
(TODO) To see a list of all available arguments type `server -h`.

### Config file (Recommended)
Saving your configuration in a file might be the better way to backup your configuration.
See the config file comments for further details.

```ini
# Default values for each server configuration.
#
# Special IP address values:
#   Any IPv4   = 0.0.0.0
#   Any IPv6   = ::
#   Any IPv4&6 = any
#
[default]

# The IP address on which to listen for new connections.
# @version 1.0
address=any

# The port for new connections and media data (TCP & UDP).
# @version 1.0
port=13370

# The IP address on which to listen for new web-socket (status) connections.
# It's recommended to allow only local access. You may allow restricted access via ProxyPass (Apache).
# @version 1.0
wsstatus-address=127.0.0.1

# The port for new web-socket (status) connections.
# @version 1.0
wsstatus-port=13375

# Maximum number of parallel client connections (slots).
# @version 1.0
connectionlimit=2147483647

# Maximum READ transfer rate the server is allowed to use (bytes/second).
# @version 1.0
bandwidthreadlimit=18446744073709551615

# Maximum WRITE transfer rate the server is allowed to use (bytes/second).
# @version 1.0
bandwidthwritelimit=18446744073709551615

# Comma separated list of valid channel-IDs, which are allowed to be used.
# Leave empty to allow all.
# @version 1.0
validchannelids=

# Global server password required to establish a connection.
# Leave empty to allow access to everyone.
# @version 1.0
password=
```

Currently there is not a lot to configure. The documentation will be updated as soon as new features has been added.

## Run as Windows Service
Registering the server process as a service can be done with the built-in Windows tool `sc`.
All upcoming commands need to run as privileged Administrator.

__Install__
`sc create "ts3video-server" binPath= "C:\path\app.exe --service" start= auto obj= ".\manuel" password= "" `


## Run as Linux Daemon
(TODO)