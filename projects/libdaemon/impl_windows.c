#include <Windows.h>
#include "api.h"

#define SVCNAME TEXT("MyService")

SERVICE_STATUS          gSvcStatus;
SERVICE_STATUS_HANDLE   gSvcStatusHandle;
HANDLE                  ghSvcStopEvent = NULL;

hbl_service_config_t  *gHblConf = 0;

void ReportSvcStatus(DWORD currentState, DWORD exitCode, DWORD waitHint);

/*! Called by SCM whenever a control code is sent to the service.
    Details about the different service-control-requests can be found in MSDN
    => https://msdn.microsoft.com/en-us/library/windows/desktop/ms685153(v=vs.85).aspx
 */
VOID WINAPI SvcCtrlHandler(DWORD ctrl)
{
  switch (ctrl)
  {
    case SERVICE_CONTROL_INTERROGATE:
      break;

    case SERVICE_CONTROL_PAUSE:
      break;

    case SERVICE_CONTROL_CONTINUE:
      break;

    case SERVICE_CONTROL_STOP:
      // Notify the SCM that we are trying to stop the service now.
      ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);

      // Signal the service to stop.
      SetEvent(ghSvcStopEvent);
      ReportSvcStatus(gSvcStatus.dwCurrentState, NO_ERROR, 0);
      break;
  }
}

/*! Helper function to set current service status and report it to the SCM.
 */
void ReportSvcStatus(DWORD currentState, DWORD exitCode, DWORD waitHint)
{
  static DWORD __checkPoint = 1;

  // Fill the SERVICE_STATUS structure.
  gSvcStatus.dwCurrentState = currentState;
  gSvcStatus.dwWin32ExitCode = exitCode;
  gSvcStatus.dwWaitHint = waitHint;

  if (currentState == SERVICE_START_PENDING)
    gSvcStatus.dwControlsAccepted = 0;
  else
    gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

  if ((currentState == SERVICE_RUNNING) || (currentState == SERVICE_STOPPED))
    gSvcStatus.dwCheckPoint = 0;
  else
    gSvcStatus.dwCheckPoint = __checkPoint++;

  // Report the status of the service to the SCM.
  SetServiceStatus(gSvcStatusHandle, &gSvcStatus);
}

/*! The service code
    The end of the function is also the end of the process.
 */
void SvcInit(DWORD dwArgc, LPTSTR *lpszArgv)
{
  // Create an event, which is used to stop the service.
  ghSvcStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
  if (!ghSvcStopEvent)
  {
    ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
    return;
  }

  // TODO: Start thread for the actual PROGRAM.
  if (gHblConf)
    gHblConf->start();

  // Report "running" status when initialization is complete.
  ReportSvcStatus(SERVICE_RUNNING, NO_ERROR, 0);

  // Do some work until service stops.
  while (1)
  {
    // Waits for the STOP event.
    WaitForSingleObject(ghSvcStopEvent, INFINITE);

    // TODO: Stop the PROGRAM.
    if (gHblConf)
      gHblConf->stop();

    // Notify the SCM that the service has been stopped.
    ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
    break;
  }
}

/*! Service entry point
 */
void WINAPI MySvcMain(DWORD dwArgc, LPTSTR *lpszArgv)
{
  // Register service handler.
  gSvcStatusHandle = RegisterServiceCtrlHandler(SVCNAME, SvcCtrlHandler);
  if (!gSvcStatusHandle)
  {
    //SvcReportEvent(TEXT("RegisterServiceCtrlHandler"));
    return;
  }

  gSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
  gSvcStatus.dwServiceSpecificExitCode = 0;

  // Notify the SCM that we are starting the service now.
  ReportSvcStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

  // Perform service specific initialization.
  SvcInit(dwArgc, lpszArgv);
}

/*! Process entry point
 */
void __cdecl _tmain(int argc, TCHAR *argv[])
{
  // Create service entry point table.
  SERVICE_TABLE_ENTRY serviceEntryDispatchTable[] =
  {
    { SVCNAME, (LPSERVICE_MAIN_FUNCTION) MySvcMain },
    { NULL, NULL }
  };

  // Start blocking service control dispatcher.
  // Blocks until the service is being stopped.
  if (!StartServiceCtrlDispatcher(serviceEntryDispatchTable))
  {
    //SvcReportEvent(TEXT("StartServiceCtrlDispatcher"));
  }
}

//int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
//{
//  _tmain(0, 0);
//  return 0;
//}
//
//int main(int argc, char *argv[])
//{
//  _tmain(0, 0);
//  return 0;
//}

///////////////////////////////////////////////////////////////////////
// Humble Win32 Service API
///////////////////////////////////////////////////////////////////////

hbl_service_config_t* hbl_service_create_config()
{
  hbl_service_config_t *c = malloc(sizeof(hbl_service_config_t));
  c->start = 0;
  c->stop = 0;
  return c;
}

void hbl_service_free_config(hbl_service_config_t *c)
{
  if (c)
    free(c);
}

int hbl_service_run(hbl_service_config_t *conf)
{
  if (!conf)
    return 1;
  gHblConf = conf;
  _tmain(0, 0);
  return 0;
}
