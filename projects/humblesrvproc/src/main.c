#include <Windows.h>
#include "humblesrvproc/api.h"

#if 0

///////////////////////////////////////////////////////////////////////
// The actual "program" of YOU that will run.
///////////////////////////////////////////////////////////////////////

HANDLE gThreadHandle = NULL;
LONG gThreadStopFlag = 0;

DWORD WINAPI threadRun(LPVOID param)
{
  while (1)
  {
    // Keep running...
    Sleep(2000);

    // Check for stop flag.
    if (InterlockedCompareExchange(&gThreadStopFlag, 0, 1) == 1)
      break;
  }
  return 0;
}

///////////////////////////////////////////////////////////////////////
// Callbacks for "humble service helper".
///////////////////////////////////////////////////////////////////////

int serviceStart(void)
{
  gThreadHandle = CreateThread(NULL, 0, threadRun, NULL, 0, NULL);
  return 0;
}

int serviceStop(void)
{
  InterlockedExchange(&gThreadStopFlag, 1);
  WaitForSingleObject(gThreadHandle, INFINITE);
  return 0;
}

/*! Simple example for service execution.
 */
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  // Initialize service configuration.
  hbl_service_config_t *conf = hbl_service_create_config();
  conf->start = &serviceStart;
  conf->stop = &serviceStop;

  // Run service.
  int exitCode = hbl_service_run(conf);

  // Clean up.
  hbl_service_free_config(conf);

  return exitCode;
}

#endif