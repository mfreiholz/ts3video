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
  // TODO Convert \ to /.
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

char* getParentPath(const char *path)
{
  char *parentPath = new char[PATH_MAX_LENGTH];
  memset(parentPath, 0, PATH_MAX_LENGTH);
  strcpy(parentPath, path);

  // Found last occurance of "\".
  // TODO Convert \ to /.
  char *offset = parentPath;
  while (true) {
    char *p = strstr(offset, "\\");
    if (!p) {
      break;
    }
    offset = ++p;
  }
  if (offset && --offset) {
    memset(offset, 0, sizeof(char));
  }
  return parentPath;
}

/*!
  Uses the "djb2"-algorithm to hash a string value.
  \see http://www.cse.yorku.ca/~oz/hash.html
 */
unsigned long hashString(const char *str)
{
  unsigned long hash = 5381;
  int c;
  while (c = *str++)
    hash = ((hash << 5) + hash) + c; // hash * 33 + c
  return hash;
}

unsigned long generateUniqueChannelId(const TS3Data *data)
{
  // Concenate all important data which identifies makes the TS3Data unique
  // in the same way for all clients (do not include the client's ID!).
  // e.g.: <server-address>#<server-port>#<channel-id>
  char s[MAX_PATH];
  s[0] = 0;

  strcat(s, data->serverAddress);
  strcat(s, "#");

  char serverPortString[64];
  itoa(data->serverPort, serverPortString, 10);
  strcat(s, serverPortString);
  strcat(s, "#");

  char channelIdString[64];
  ltoa(data->channelId, channelIdString, 10);
  strcat(s, channelIdString);
  strcat(s, "#");
  
  // Generate a hashed number value for the unique string.
  return hashString(s);
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

  // Define working directory.
  char *workingDirectory = getParentPath(filePath);
  if (!workingDirectory) {
    return 2;
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
  ltoa(generateUniqueChannelId(ts3data)/*ts3data->channelId*/, ts3ChannelId, 10);
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
  execInfo.lpDirectory = workingDirectory;
  execInfo.nShow = SW_SHOWNORMAL;
  if (!ShellExecuteEx(&execInfo)) {
    delete[] workingDirectory;
    delete[] filePath;
    return 3;
  }
#endif

  delete[] workingDirectory;
  delete[] filePath;
  return 0;
}
