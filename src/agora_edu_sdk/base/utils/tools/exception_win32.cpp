//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <stdio.h>
#include <windows.h>

namespace agora {
namespace win32 {
extern HMODULE g_hInstDll;
}
namespace commons {

static DWORD nDumpType = 0;
struct ExceptionPatch {
  PBYTE addr;
  LPTOP_LEVEL_EXCEPTION_FILTER oldfilter;
  BYTE oldcode[5];
} dbgPatch;

char *GetMyFileNameStrrchr(char *dest, char ch) {
  GetModuleFileNameA(agora::win32::g_hInstDll, dest, MAX_PATH);
  return strrchr(dest, ch) + 1;
}

BOOL GetModuleFileNameByAddress(DWORD addr, LPSTR filename) {
  HMODULE hNtDll = GetModuleHandleA("ntdll.dll");
  if (!hNtDll) return FALSE;

  typedef DWORD *(WINAPI * PRtlCreateQueryDebugBuffer)(DWORD, DWORD);
  typedef DWORD(WINAPI * PRtlDestroyQueryDebugBuffer)(PDWORD);
  typedef DWORD(WINAPI * PRtlQueryProcessDebugInformation)(HANDLE, DWORD, PVOID);
  typedef struct _DEBUGMODULEINFO {
    DWORD ImageBase;
    DWORD ImageSize;
    DWORD Unknown01;        // possibly some kind of version info. nthandle doesnt use it
    USHORT DllSequenceNum;  // if 0 then EXE
    USHORT NumDlls;         // only know if seqnum is 0
    DWORD GrantedAccess;
    CHAR Name[MAX_PATH];
    DWORD Unknown02;
  } DEBUGMODULEINFO, *PDEBUGMODULEINFO;
  typedef struct _QUERYDEBUGBUFFER_HEADER {
    DWORD Unkown12[12];
    DWORD *ModArrayHeader;
    DWORD Unkown11[11];
    DWORD NumNames;  // total entries including the EXE
    DWORD Reserved[2];
    DEBUGMODULEINFO ModInfo[1];
  } QUERYDEBUGBUFFER, *PQUERYDEBUGBUFFER;

  PRtlCreateQueryDebugBuffer RtlCreateQueryDebugBuffer =
      (PRtlCreateQueryDebugBuffer)GetProcAddress(hNtDll, "RtlCreateQueryDebugBuffer");

  PRtlDestroyQueryDebugBuffer RtlDestroyQueryDebugBuffer =
      (PRtlDestroyQueryDebugBuffer)GetProcAddress(hNtDll, "RtlDestroyQueryDebugBuffer");

  PRtlQueryProcessDebugInformation RtlQueryProcessDebugInformation =
      (PRtlQueryProcessDebugInformation)GetProcAddress(hNtDll, "RtlQueryProcessDebugInformation");

  DWORD i;
  DWORD *pRtlBuffer, error;
  QUERYDEBUGBUFFER *pDebugInfo;
  BOOL retval = FALSE;

  pRtlBuffer = RtlCreateQueryDebugBuffer(NULL, NULL);

  if (pRtlBuffer == NULL) return FALSE;

  error = RtlQueryProcessDebugInformation((HANDLE)GetCurrentProcessId(), 1, pRtlBuffer);
  if (error != 0) {
    goto cleanup;
  }

  pDebugInfo = (QUERYDEBUGBUFFER *)pRtlBuffer;

  for (i = 0; i < pDebugInfo->NumNames; i++) {
    if (addr > pDebugInfo->ModInfo[i].ImageBase &&
        addr < pDebugInfo->ModInfo[i].ImageBase + pDebugInfo->ModInfo[i].ImageSize) {
      strcpy(filename, pDebugInfo->ModInfo[i].Name);
      retval = TRUE;
      break;
    }
  }

cleanup:
  if (pRtlBuffer != NULL) RtlDestroyQueryDebugBuffer(pRtlBuffer);
  return retval;
}

static DWORD WINAPI DumpThreadProc(PVOID) {
  typedef BOOL(WINAPI * PDbgMiniDumpWriteDump)(HANDLE, DWORD, HANDLE, DWORD, PVOID, PVOID, PVOID);
  PDbgMiniDumpWriteDump fnMiniDumpWriteDump;

  HMODULE hModule = NULL;
  HANDLE hFile = INVALID_HANDLE_VALUE;
  char filename[MAX_PATH], filename2[MAX_PATH];
  char temp[1024];
  char *szTittle = "Agora RtcEngine Fatal Error";
  BOOL fGotFileName = GetModuleFileNameByAddress((DWORD)dbgPatch.addr, filename2);

  strcpy(GetMyFileNameStrrchr(filename, '\\'), "dbghelp.dll");
  sprintf(temp,
          "Agora RtcEngine has catched an unhandled exception raised at %p, in %s, which is may or "
          "may not be caused by RtcEngine itself",
          dbgPatch.addr, fGotFileName ? filename2 : "unknown module");
  if ((int)nDumpType < 0) {
    sprintf(temp + strlen(temp),
            ".\nAgora RtcEngine can help you dump the crash information. If you want to do this, "
            "set \"Crash Dump File\" in your d2hackmap.cfg to:\n\t0 for normal dump. ~ 10KB\n\t1 "
            "for dump with data segments. ~ 1MB\n\t2 for dump with full memory. ~ 10MB");
    goto cleanup;
  }

  hModule = ::LoadLibraryA(filename);
  if (hModule == NULL) {
    hModule = ::LoadLibraryA("dgbhelp.dll");
    if (hModule == NULL) {
      *(GetMyFileNameStrrchr(filename, '\\') - 1) = '\0';
      sprintf(temp + strlen(temp),
              " and want to generate a crash dump file, but dbghelp.dll is missing.\nFor better "
              "diagnoses, I highly suggest you copy dbghelp.dll to %s or system32 directory, which "
              "was originally packed along with RtcEngine in case you haven't a working version.",
              filename);
      goto cleanup;
    }
  }

  fnMiniDumpWriteDump = (PDbgMiniDumpWriteDump)::GetProcAddress(hModule, "MiniDumpWriteDump");
  if (fnMiniDumpWriteDump == NULL) {
    sprintf(temp + strlen(temp),
            " and is loading dbghelp.dll to generate a crash dump file for further diagnoses, but "
            "it seems that your dbghelp.dll is outdated.\nPlease get the latest version.");
    goto cleanup;
  }

  strcpy(GetMyFileNameStrrchr(filename, '.'), "dmp");

  hFile =
      ::CreateFileA(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFile == INVALID_HANDLE_VALUE) {
    sprintf(temp + strlen(temp),
            " and want to generate a crash dump file, but %s can not be created.", filename);
    goto cleanup;
  }

  fnMiniDumpWriteDump(::GetCurrentProcess(), ::GetCurrentProcessId(), hFile, nDumpType, NULL, NULL,
                      NULL);
  sprintf(temp + strlen(temp),
          ". The crash dump information has been written to %s.\nPlease mail it to "
          "fengyue@agora.io as an attachment for further diagnoses.\nYour support is very "
          "important to me. Thanks in advance.",
          filename);
  sprintf(temp + strlen(temp),
          "\n\nTip: you can also specify the dump file type by setting \"Crash Dump File\" in your "
          "CFG to:\n\t-1 -- don't generate crash dump file\n\t 0 -- normal dump file. This is "
          "default setting. ~ 10KB\n\t 1 -- dump with data segments. This option will make the "
          "dump file significantly larger. ~ 1MB\n\t 2 -- dump with full memory. This option will "
          "result in a very large file. ~ 10MB");

cleanup:
  if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
  if (hModule != NULL) FreeLibrary(hModule);
  MessageBoxA(NULL, temp, szTittle, MB_OK | MB_ICONERROR);
  return 0;
}

static void CreateDumpFile() {
  HANDLE h = CreateThread(NULL, 0, &DumpThreadProc, NULL, 0, NULL);
  if (h) {
    WaitForSingleObject(h, INFINITE);
    CloseHandle(h);
  }
}

static void memcpyex(void *dst, void *src, size_t len) {
  DWORD oldprot;
  VirtualProtect(dst, len, PAGE_EXECUTE_READWRITE, &oldprot);
  memcpy(dst, src, len);
  VirtualProtect(dst, len, oldprot, NULL);
}

static void InterceptCall(PBYTE caller, PBYTE callee, size_t len) {
  BYTE buf[5] = {0xe8};
  *(DWORD *)(buf + 1) = callee - caller - 5;
  memcpyex(caller, buf, len);
}

static void ExceptionProc_C() {
  // restore old unhandle exception filter
  SetUnhandledExceptionFilter(dbgPatch.oldfilter);
  // restore old code where exception occoured;
  memcpyex(dbgPatch.addr, dbgPatch.oldcode, sizeof(dbgPatch.oldcode));
  // create dump file
  CreateDumpFile();
}

static void __declspec(naked) ExceptionProc_ASM() {
  __asm {
		sub dword ptr [esp], 5;
    // establish ebp frame pointer for stack walk
		push ebp;
		mov ebp, esp;
		call ExceptionProc_C;
		mov esp,ebp;
		pop ebp;
		ret;
  }
}

static LONG WINAPI DbgUnhandledExceptionFilter(_EXCEPTION_POINTERS *pei) {
  //	MessageBox(NULL, "dsfs", "sdsdf", MB_OK); // attach to debugger here
  // we need to intercept exception address to get stack information
  dbgPatch.addr = (PBYTE)pei->ExceptionRecord->ExceptionAddress;
  memcpy(dbgPatch.oldcode, dbgPatch.addr, sizeof(dbgPatch.oldcode));
  InterceptCall(dbgPatch.addr, (PBYTE)ExceptionProc_ASM, sizeof(dbgPatch.oldcode));

  // execute again
  return EXCEPTION_CONTINUE_EXECUTION;
}

void SetDbgUnexpectedHandler(bool first) {
  if (first) {
    if (!dbgPatch.oldfilter)
      dbgPatch.oldfilter = SetUnhandledExceptionFilter(&DbgUnhandledExceptionFilter);
  } else {
    if (dbgPatch.oldfilter) {
      SetUnhandledExceptionFilter(dbgPatch.oldfilter);
    }
  }
}
}  // namespace commons
}  // namespace agora
