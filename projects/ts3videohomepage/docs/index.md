# TS3VIDEO Documentation

* Client plugin
  * Installation
    * Windows
    * Linux (Debian, Ubuntu, ...)

* Server
  * Installation
  * Configuration

--------------------------------------------------------------

# Client

## Installation
After downloading the plugin from the website you can simply double click the *.ts3\_plugin file, which will start the Teamspeak plugin installer. _We highly recommend to close Teamspeak before trying to install the plugin._

(TODO: Insert Screenshot of plugin installer here...)

__Linux:__ The client plugin for Linux is in development.  
__Mac:__ I still don't have a Mac.

--------------------------------------------------------------

# Server

## Installation
The downloaded server package is usually a simple ZIP archive, which can be extracted wherever you want it.

__Windows:__ `C:\ts3video-server`  
__Linux:__ `/opt/ts3video-server`

## Configuration
The server provides multiple ways of configurations before startup. You can configure it by appending commandline arguments directly or by passing a configuration file during startup.

### Commandline parameters
To see a list of all available arguments type `server -h`.

### Config file (Recommended)
Saving your configuration in a file might be the better way to backup your configuration.

```ini
# Default values for each server configuration.
[default]

# The IP address on which to listen for new connections (0.0.0.0 = ALL).
# @version 1.0
address=0.0.0.0

# The port for new connections and media data (TCP & UDP).
# @version 1.0
port=6000

# Maximum number of parallel client connections.
# @version 1.0
connectionlimit=10

# Maximum READ transfer rate the server is allowed to use.
# @version 1.0
bandwidthreadlimit=80000

# Maximum WRITE transfer rate the server is allowed to use.
# @version 1.0
bandwidthwritelimit=160000

# Comma separated list of valid TS3 channel-IDs, which are allowed to be used.
# Leave empty to allow all.
# @version 1.0
validchannelids=1,2,3,4
```

Currently there is not a lot to configure. The documentation will be updated as soon as new features has been added.