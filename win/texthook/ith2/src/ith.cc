// ith.cc
// 10/15/2011

#include "ith.h"
#include "ith/hookman.h"
#include "ith/inject.h"
#include "ith/main.h"
#include "ith/pipe.h"
#include <QtCore>
#include <cstdlib> // ::atexit
#include <string>  // std::wstring
#include <qt_windows.h>

#define DEBUG "ith"
#include "module/debug/debug.h"

#define ITH_RUNNING_MUTEX       L"ITH_RUNNING"
#define ITH_RUNNING_EVENT       L"ITH_PIPE_EXIST"

// - Ith background service -

/*
// jichi:10/20/2011:Not needed after eliminate Rtl global heap.
// in static initializers from other modules invoked before this module.
namespace { // anonymous

  struct IthBackgroundService
  {
    IthBackgroundService()
    {
      DOUT("initializing system service (fn:IthInitSystemService) ...");
      ::IthInitSystemService(); // So that Ith- work normally.
    }

    ~IthBackgroundService()
    {
      DOUT("closing system service (fn:IthCloseSystemService) ..." );
      ::IthCloseSystemService();
    }
  };
  IthBackgroundService IthDaemon_;

} // anonymous
*/

// - Global variables -

// + Global settings +
DWORD cyclic_remove = 0, global_filter = 0; // boolean
DWORD split_time = 200, process_time = 50; // in msecs
DWORD repeat_count = 0;

// + Global states +

bool running = true;
BYTE* static_large_buffer; // used in utility.h

HANDLE hPipeExist;

// + Global windows +

HWND hMainWnd;

// + Global singletons +

TextBuffer *texts;
HookManager *man;
//ProfileManager  *pfman;
//CommandQueue *cmdq;
BitMap *pid_map;
CustomFilterMultiByte *mb_filter;
CustomFilterUnicode *uni_filter;

// - Static members -

namespace { // anonymous, handlers

  LONG WINAPI
  exceptionFilter_(EXCEPTION_POINTERS *e)
  {
    DOUT("enter");
    //WCHAR str[0x40],name[0x100];

    //::swprintf(str, L"Exception code: 0x%.8X\r\nAddress: 0x%.8X",
    //  ExceptionInfo->ExceptionRecord->ExceptionCode,
    //  ExceptionInfo->ContextRecord->Eip);
    //::MessageBox(0,str, 0, 0);
    QString msg = QString("Exception code: %1\nAddress: %2")
        .arg(QString::number(e->ExceptionRecord->ExceptionCode, 16))
        .arg(QString::number(e->ContextRecord->Eip, 16));

    /*
    MEMORY_BASIC_INFORMATION info;
    NT_STATUS status = ::NtQueryVirtualMemory(
      NtCurrentProcess(),
      (PVOID)e->ContextRecord->Eip,
      MemoryBasicInformation, &info,sizeof(info),0
    );
    if (NT_SUCCESS(status) )
      msg += QString("\nException offset: %2").arg(
        QString::number(ExceptionInfo->ContextRecord->Eip - (DWORD)info.AllocationBase, 16)
      );
    */

    //QMessageBox::warning(0, "Exception", msg);
    //NtTerminateProcess(NtCurrentProcess(),0);
    DOUT(msg);
    DOUT("WARNING: kill all running processes of annot-player");
    ::WinExec("tskill annot-player", SW_HIDE);
    //exit(-1);
    DOUT("exit");
    return 0;
  }

} // anonymous namespace

// - Initializations -

/*

namespace { // anonymous, status variables
  bool serviceStarted_ = false;
} // anonymous namespace

void
Ith::startService()
{
  Q_ASSERT(!serviceStarted_);
  if (!serviceStarted_) {
    ::IthInitSystemService();
    serviceStarted_ = true;

    ::atexit(Ith::stopService);
  }
}

void
Ith::stopService()
{
  if (serviceStarted_) {
    ::IthCloseSystemService();
    serviceStarted_ = false;
  }
}
*/

HWND
Ith::parentWindow()
{ return ::hMainWnd; }

void
Ith::setParentWindow(HWND hwnd)
{ ::hMainWnd = hwnd; }

int
Ith::messageInterval()
{ return ::split_time; }

void
Ith::setMessageInterval(int msecs)
{ ::split_time = msecs; }

void
Ith::init()
{
  //Q_UNUSED(runAsAdmin) DOUT("init:enter")

  ::hMainWnd = 0;

  DOUT("initializing system service (fn:IthInitSystemService) ...");
  ::IthInitSystemService(); // So that Ith- work normally.

  DWORD dwDummy;
  ::IthCreateMutex(ITH_RUNNING_MUTEX, TRUE ,&dwDummy);
  ::hPipeExist=::IthCreateEvent(ITH_RUNNING_EVENT);
  ::NtSetEvent(::hPipeExist, NULL);

  DOUT("installing exception filer ...");
  ::SetUnhandledExceptionFilter(::exceptionFilter_);

  //DOUT("creating hidden windows ...");
  //createHiddenWindows(hIns);

  DOUT("allocating mb_filter ...");
  ::mb_filter = new CustomFilterMultiByte;

  DOUT("allocating uni_filter ...");
  ::uni_filter = new CustomFilterUnicode;

  DOUT("allocating pid_map ...");
  ::pid_map = new BitMap(0x100);

  DOUT("allocating texts ...");
  ::texts = new TextBuffer;

  DOUT("allocating man ...");
  ::man = new HookManager;

  //DOUT("allocating pfman ...");
  //::pfman = new ProfileManager;

  //DOUT("allocating cmdq ...");
  //::cmdq = new CommandQueue;

  DOUT("creating pipes ...");
  ::OpenPipe();

  DOUT("exit");
}

void
Ith::destroy()
{
  DOUT("enter");

  DOUT("clearing running pipe event ..." );
  ::NtClearEvent(hPipeExist);

  //DOUT("deleting cmdq ..." );
  //delete cmdq;

  //DOUT("deleting pfman ..." );
  //delete pfman;

  DOUT("deleting man ..." );
  delete man;

  DOUT("deleting texts ..." );
  delete texts;

  DOUT("deleting mb_filter ..." );
  delete mb_filter;

  DOUT("deleting uni_filter ..." );
  delete uni_filter;

  DOUT("deleting pid_map ..." );
  //delete pid_map;

  DOUT("deleting static_large_buffer if valid ..." );
  if (static_large_buffer)
    delete static_large_buffer;

  DOUT("closing system service (fn:IthCloseSystemService) ..." );
  ::IthCloseSystemService();

  DOUT("exit");
}

// - Injections -

bool
Ith::attachProcess(ulong pid)
{ return ::InjectProcessByPid(pid) != INVALID_HANDLE_VALUE; }

bool
Ith::detachProcess(ulong pid)
{ return !::DetachProcessByPid(pid); }

// - Queries -

QString
Ith::getHookNameById(ulong hookId)
{
  QString ret;
  if (hookId) {
    auto p = reinterpret_cast<TextThread *>(hookId);
    if (p->good())
      ret = p->name();
  }
  return ret;
}

// EOF
