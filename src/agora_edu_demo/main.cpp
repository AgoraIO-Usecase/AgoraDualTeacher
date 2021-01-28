#include "src/login_widget_manager.h"
#include "util/util.h"
#include <shellscalingapi.h>
#include <QIcon>
#include <QTextCodec>
#include <QtWidgets/QApplication>
#include <Dbghelp.h>
#include <tchar.h>
#pragma comment(lib, "Shcore.lib")
#pragma comment(lib,"Dbghelp.lib")

LONG ApplicationCrashHandler(EXCEPTION_POINTERS *pException) {
  TCHAR buffer[MAX_PATH];
  GetCurrentDirectory(MAX_PATH,buffer);
  _tcscpy_s(buffer,L"crash.dmp");
  HANDLE hDumpFile = CreateFile(buffer, GENERIC_WRITE, 0, NULL,
                                CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
  dumpInfo.ExceptionPointers = pException;
  dumpInfo.ThreadId = GetCurrentThreadId();
  dumpInfo.ClientPointers = TRUE;
  MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile,
                    MiniDumpNormal, &dumpInfo, NULL, NULL);
  CloseHandle(hDumpFile);
  return EXCEPTION_EXECUTE_HANDLER;
}

int main(int argc, char *argv[]) {
  QGuiApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
  QGuiApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
  QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
  QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
  QApplication app(argc, argv);
  SetUnhandledExceptionFilter(
      (LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashHandler);
  app.setWindowIcon(QIcon(":/image/resource/logo-dual-teacher@2x.png"));
  SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);
  std::shared_ptr<LoginWidgetManager> login_widget_manager =
      std::make_shared<LoginWidgetManager>();
  login_widget_manager->ShowLoginDialog();
  return app.exec();
}
