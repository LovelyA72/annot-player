// main.cc
// 9/3/2011

#include "config.h"
#include <string>
#include <memory>

#ifdef _MSC_VER
  #include <windows.h>
#endif // _MSC_VER

//#define DEBUG "main"
//#include "module/debug/debug.h"
//#ifdef DEBUG
//  #include <QtCore>
//  #define DOUT(_msg)    qDebug() << "main:" << _msg
//#else
  #define DOUT(_dummy)
//#endif // DEBUG

// - Launcher -

namespace { // anonymous

  inline std::string dirname(const std::string &path)
  { return path.substr(0, path.find_last_of('\\')); }

  inline std::wstring dirname(const std::wstring &path)
  { return path.substr(0, path.find_last_of(L'\\')); }

} // anonymous namespace


// - Main -

#ifdef _MSC_VER

int CALLBACK
WinMain(__in HINSTANCE hInstance, __in HINSTANCE hPrevInstance, __in LPSTR lpCmdLine, __in int nCmdShow)
{
  enum { BUFFER_SIZE = MAX_PATH * 3 };
  WCHAR wszBuffer[BUFFER_SIZE]; {
    int nSize = ::GetModuleFileNameW(0, wszBuffer, BUFFER_SIZE);
    if (nSize == BUFFER_SIZE)
      return -1;
  }
  std::wstring wsApp(wszBuffer);
  std::wstring wsNextApp = dirname(wsApp) + (L"\\" APP_PREFIX APP_EXE);
  std::wstring wsNextAppPath = dirname(wsNextApp);

  DOUT(QString::fromStdWString(app));

  STARTUPINFOW siStartupInfo;
  ::memset(&siStartupInfo, 0, sizeof(siStartupInfo));
  siStartupInfo.cb = sizeof(siStartupInfo);

  PROCESS_INFORMATION piProcessInfo;
  ::memset(&piProcessInfo, 0, sizeof(piProcessInfo));

  LPVOID lpEnvironment = 0;     // TODO

  LPWSTR lpwCmdLine = 0;;
  if (lpCmdLine) {
    BOOL bResult = ::MultiByteToWideChar(CP_ACP, 0, lpCmdLine, -1, wszBuffer, BUFFER_SIZE);
    if (bResult)
      lpwCmdLine = wszBuffer;
  }
  // See: http://msdn.microsoft.com/en-us/library/windows/desktop/ms682425(v=vs.85).aspx
  //
  // BOOL WINAPI CreateProcess(
  //   __in_opt     LPCTSTR lpApplicationName,
  //   __inout_opt  LPTSTR lpCommandLine,
  //   __in_opt     LPSECURITY_ATTRIBUTES lpProcessAttributes,
  //   __in_opt     LPSECURITY_ATTRIBUTES lpThreadAttributes,
  //   __in         BOOL bInheritHandles,
  //   __in         DWORD dwCreationFlags,
  //   __in_opt     LPVOID lpEnvironment,
  //   __in_opt     LPCTSTR lpCurrentDirectory,
  //   __in         LPSTARTUPINFO lpStartupInfo,
  //   __out        LPPROCESS_INFORMATION lpProcessInformation
  // );
  //
  BOOL bResult = ::CreateProcessW(
    wsNextApp.c_str(),  // app path
    lpwCmdLine,         // app param
    0, 0,               // security attributes
    false,              // inherited
    CREATE_DEFAULT_ERROR_MODE, // creation flags
    lpEnvironment,
    wsNextAppPath.c_str(),
    &siStartupInfo,
    &piProcessInfo
  );

  return bResult ? 0 : -1;
}

#else

// TODO
int main(int argc, char *argv[]) { return 0; }

#endif // _MSC_VER

// EOF