#include <string>
#include <stdio.h>
#include <Windows.h>
#include "impl.h"

#define PATH_MAX_LENGTH 4096

// Logging ////////////////////////////////////////////////////////////



// Common /////////////////////////////////////////////////////////////

/**
 * Gets the path to the TS3VideoClient.exe.
 * @return char* Ownership goes to caller.
 */
char* findClientExeFilePath()
{
#ifdef _WIN32
  char moduleFilePath[PATH_MAX_LENGTH]; ///< Path to teamspeak3client.exe
  if (GetModuleFileName(NULL, moduleFilePath, PATH_MAX_LENGTH) <= 0) {
    delete[] moduleFilePath;
    return NULL;
  }
  
  // Found last occurance of "\".
  char *offset = moduleFilePath;
  while (true) {
    char *p = strstr(offset, "\\");
    if (!p) {
      break;
    }
    offset = ++p;
  }
  if (offset) {
    memset(offset, 0, sizeof(char));
  }

  char *path = new char[PATH_MAX_LENGTH];
  path[0] = 0;
  strcat(path, moduleFilePath);
  strcat(path, "plugins\\qts3videoclientplugin\\qts3videoclient.exe");
  return path;
#endif
}

// API 1.0 ////////////////////////////////////////////////////////////

/**
 * Starts the ts3video plugin as an separate process.
 * @return 0 = OK; Everything else indicates an error.
 */
int runClient(const char *serverAddress, unsigned short serverPort, const char *username, TS3Data *ts3data)
{
  // Find client executable.
  char *filePath = findClientExeFilePath();
  if (!filePath) {
    return 1;
  }

  // Command line parameters.
  // e.g.: --server-address 0.0.0.0 --server-port 6000 --username "Foo Bar"
  char params[PATH_MAX_LENGTH];
  params[0] = 0;

  strcat(params, " --server-address ");
  strcat(params, serverAddress);

  char serverPortString[64];
  itoa(serverPort, serverPortString, 10);
  strcat(params, " --server-port ");
  strcat(params, " ");
  strcat(params, serverPortString);
  strcat(params, " ");

  strcat(params, " --username ");
  strcat(params, " \"");
  strcat(params, username);
  strcat(params, "\" ");

  char ts3ClientId[64];
  ltoa(ts3data->clientId, ts3ClientId, 10);
  strcat(params, " --ts3-clientid ");
  strcat(params, " ");
  strcat(params, ts3ClientId);
  strcat(params, " ");

  char ts3ChannelId[64];
  ltoa(ts3data->channelId, ts3ChannelId, 10);
  strcat(params, " --ts3-channelid ");
  strcat(params, " ");
  strcat(params, ts3ChannelId);
  strcat(params, " ");

#ifdef _WIN32
  SHELLEXECUTEINFO execInfo;
  memset(&execInfo, 0, sizeof(execInfo));
  execInfo.cbSize = sizeof(execInfo);
  execInfo.hwnd = NULL;
  execInfo.lpVerb = "open";
  execInfo.lpFile = filePath;
  execInfo.lpParameters = params;
  execInfo.lpDirectory = NULL;
  execInfo.nShow = SW_SHOWNORMAL;
  if (!ShellExecuteEx(&execInfo)) {
    delete[] filePath;
    return 2;
  }
#endif

  delete[] filePath;
  return 0;
}
