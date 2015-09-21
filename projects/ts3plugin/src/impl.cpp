#include <string>
#include <stdio.h>
#include <Windows.h>
#include "ts3video.h"
#include "ts3_functions.h"
#include "impl.h"

#define PATH_MAX_LENGTH 4096

// Logging ////////////////////////////////////////////////////////////

// TODO Implement some very simple logging or static link against HumbleLogging

// Common /////////////////////////////////////////////////////////////

/*
    Gets the path to the TS3VideoClient.exe.
    @return char* Ownership goes to caller.
*/
char* findClientExeFilePath()
{
#ifdef _WIN32
	char moduleFilePath[PATH_MAX_LENGTH]; ///< Path to teamspeak3client.exe
	if (GetModuleFileName(NULL, moduleFilePath, PATH_MAX_LENGTH) <= 0)
		return NULL;

	// Find last occurance of "\".
	// TODO Convert \ to /.
	char* offset = moduleFilePath;
	while (true)
	{
		char* p = strstr(offset, "\\");
		if (!p)
			break;
		offset = ++p;
	}
	if (offset)
		memset(offset, 0, sizeof(char));

	char* path = new char[PATH_MAX_LENGTH];
	path[0] = 0;
	strcat(path, moduleFilePath);
	strcat(path, "plugins");
	strcat(path, "\\");
	strcat(path, "ts3video");
	strcat(path, "\\");
	strcat(path, IFVS_SOFTWARE_VERSION);
	strcat(path, "\\");
	strcat(path, "videoclient.exe");
	return path;
#endif
}

char* getParentPath(const char* path)
{
	char* parentPath = new char[PATH_MAX_LENGTH];
	memset(parentPath, 0, PATH_MAX_LENGTH);
	strcpy(parentPath, path);

	// Find last occurance of "\".
	// TODO Convert \ to /.
	char* offset = parentPath;
	while (true)
	{
		char* p = strstr(offset, "\\");
		if (!p)
			break;
		offset = ++p;
	}
	if (offset && --offset)
		memset(offset, 0, sizeof(char));
	return parentPath;
}

char* generateUniqueChannelIdentifier(const TS3Data* data)
{
	// Concenate all important data which makes the TS3Data unique
	// in the same way for all clients (do not include the client's ID!).
	// e.g.: <server-address>#<server-port>#<channel-id>
	char* s = new char[MAX_PATH];
	memset(s, 0, MAX_PATH);

	strcat(s, data->serverAddress);
	strcat(s, "#");

	char serverPortString[64];
	ltoa(data->serverPort, serverPortString, 10);
	strcat(s, serverPortString);
	strcat(s, "#");

	char channelIdString[64];
	ltoa(data->targetChannelId, channelIdString, 10);
	strcat(s, channelIdString);
	strcat(s, "#");

	return s;
}

char* generateChannelPassword(const TS3Data* data)
{
	const char* ident = generateUniqueChannelIdentifier(data);
	const int identLen = strlen(ident);

	char* pass = new char[MAX_PATH];
	memset(pass, 0, MAX_PATH);

	for (int i = identLen / 2 - 1; i >= 0; --i)
	{
		const int ival = ident[i];
		char buff[128];
		itoa(ival, buff, 16);
		strcat(pass, buff);
	}

	delete[] ident;
	return pass;
}

// API ////////////////////////////////////////////////////////////////

int runClient(TS3Data* ts3data, int skipStartupDialog)
{
	// It's not allowed to join a conference of a different channel.
	if (ts3data->channelId != ts3data->targetChannelId)
	{
		ts3data->funcs->printMessageToCurrentTab("You can not join a video-conference of a different channel.");
		return 1;
	}

	// Find client executable.
	char* filePath = findClientExeFilePath();
	if (!filePath)
		return 1;

	// Define working directory.
	char* workingDirectory = getParentPath(filePath);
	if (!workingDirectory)
	{
		delete[] filePath;
		return 2;
	}

	// Command line parameters.
	// --mode ts3video --address teamspeak.insanefactory.com --port 9987 --channelid 1 --clientdbid 6 --username "Manuel"
	char params[PATH_MAX_LENGTH];
	params[0] = 0;

	strcat(params, " --mode ");
	strcat(params, " ts3video ");

	strcat(params, " --address ");
	strcat(params, ts3data->serverAddress);

	char serverPortString[64];
	itoa(ts3data->serverPort, serverPortString, 10);
	strcat(params, " --port ");
	strcat(params, " ");
	strcat(params, serverPortString);
	strcat(params, " ");

	char channelIdString[64];
	itoa(ts3data->channelId, channelIdString, 10);
	strcat(params, " --channelid ");
	strcat(params, " ");
	strcat(params, channelIdString);
	strcat(params, " ");

	char clientDatabaseIdString[64];
	ltoa(ts3data->clientDatabaseId, clientDatabaseIdString, 10);
	strcat(params, " --clientdbid ");
	strcat(params, " ");
	strcat(params, clientDatabaseIdString);
	strcat(params, " ");

	strcat(params, " --username ");
	strcat(params, " \"");
	strcat(params, ts3data->clientName);
	strcat(params, "\" ");

	if (skipStartupDialog)
		strcat(params, " --skip-startup-dialog ");

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
	if (!ShellExecuteEx(&execInfo))
	{
		delete[] workingDirectory;
		delete[] filePath;
		return 3;
	}
#endif

	delete[] workingDirectory;
	delete[] filePath;
	return 0;
}
