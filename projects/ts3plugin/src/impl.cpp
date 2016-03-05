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

/*
	Searches the absolute path to videoclient.exe in Win32 registry.
	@return char* NULL terminated string or NULL. Ownership goes to caller.
*/
char* findClientExeFilePathInRegistry()
{
#ifdef _WIN32
	HKEY hkey;
	long result;

	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\mfreiholz.de\\VideoClient", 0, KEY_READ, &hkey);
	if (result != ERROR_SUCCESS)
	{
		result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\WOW6432Node\\mfreiholz.de\\VideoClient", 0, KEY_READ, &hkey);
		if (result != ERROR_SUCCESS)
			return 0;
	}

	// Read path from found key.
	char* path = new char[PATH_MAX_LENGTH];
	memset(path, 0, PATH_MAX_LENGTH);

	DWORD type;
	DWORD valueSize = PATH_MAX_LENGTH;
	DWORD dwRet = RegQueryValueEx(hkey, "", NULL, &type, (BYTE*)path, &valueSize);
	if (result != ERROR_SUCCESS)
	{
		delete[] path;
		return 0;
	}
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

// API ////////////////////////////////////////////////////////////////

int runClient(TS3Data* ts3data, int runOpts)
{
	// It's not allowed to join a conference of a different channel.
	if (ts3data->channelId != ts3data->targetChannelId)
	{
		ts3data->funcs->printMessageToCurrentTab("You can not join a video-conference of a different channel.");
		return 1;
	}
	// Find client executable.
	char* filePath = findClientExeFilePathInRegistry();
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

	if (runOpts & RUNOPT_PUBLIC)
		strcat(params, " --public ");

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