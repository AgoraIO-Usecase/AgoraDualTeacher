//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "utils/tools/cpu_usage.h"
#include "utils/log/log.h"
#include <shlobj.h>
#include "utils/tools/util.h"
#include <wchar.h>
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <iterator>
#include <string>
#include <thread>
#include "utils/tools/sys_compat.h"

#if 0
#if defined(_MSC_VER) || defined(__BORLANDC__)
#define EPOCHFILETIME (116444736000000000i64)
#else
#define EPOCHFILETIME (116444736000000000LL)
#endif


struct timezone {
	int tz_minuteswest; /* minutes W of Greenwich */
	int tz_dsttime;     /* type of dst correction */
};
int pri_gettimeofday(struct timeval *tv, struct timezone *tz)
{
	FILETIME        ft;
	LARGE_INTEGER   li;
	__int64         t;
	static int      tzflag;

	if (tv)
	{
		GetSystemTimeAsFileTime(&ft);
		li.LowPart  = ft.dwLowDateTime;
		li.HighPart = ft.dwHighDateTime;
		t  = li.QuadPart;       /* In 100-nanosecond intervals */
		t -= EPOCHFILETIME;     /* Offset to the Epoch time */
		t /= 10;                /* In microseconds */
		tv->tv_sec  = (long)(t / 1000000);
		tv->tv_usec = (long)(t % 1000000);
	}

	if (tz)
	{
		if (!tzflag)
		{
			_tzset();
			tzflag++;
		}
		tz->tz_minuteswest = _timezone / 60;
		tz->tz_dsttime = _daylight;
	}

	return 0;
}

int pri_inet_pton(int af, const char *src, void *dst)
{
	struct sockaddr_storage ss;
	int size = sizeof(ss);
	char src_copy[INET6_ADDRSTRLEN + 1];

	ZeroMemory(&ss, sizeof(ss));
	/* stupid non-const API */
	strncpy(src_copy, src, INET6_ADDRSTRLEN + 1);
	src_copy[INET6_ADDRSTRLEN] = 0;

	if (WSAStringToAddressA(src_copy, af, NULL, (struct sockaddr *)&ss, &size) == 0)
	{
		switch (af)
		{
		case AF_INET:
			*(struct in_addr *)dst = ((struct sockaddr_in *)&ss)->sin_addr;
			return 1;
		case AF_INET6:
			*(struct in6_addr *)dst = ((struct sockaddr_in6 *)&ss)->sin6_addr;
			return 1;
		}
	}
	return 0;
}

const char *pri_inet_ntop(int af, const void *src, char *dst, socklen_t size)
{
	struct sockaddr_storage ss;
	unsigned long s = size;

	ZeroMemory(&ss, sizeof(ss));
	ss.ss_family = af;

	switch (af)
	{
	case AF_INET:
		((struct sockaddr_in *)&ss)->sin_addr = *(struct in_addr *)src;
		break;
	case AF_INET6:
		((struct sockaddr_in6 *)&ss)->sin6_addr = *(struct in6_addr *)src;
		break;
	default:
		return NULL;
	}
	/* cannot direclty use &size because of strict aliasing rules */
	return (WSAAddressToStringA((struct sockaddr *)&ss, sizeof(ss), NULL, dst, &s) == 0) ? dst : NULL;
}
#endif

namespace agora {
namespace commons {

std::string uuid() {
  std::string str;
  UUID uuid;
  if (UuidCreate(&uuid) == RPC_S_OK) {
    RPC_CSTR szGUID;
    if (RPC_S_OK == UuidToStringA(&uuid, (RPC_CSTR*)&szGUID)) {
      std::string guidStr((char*)szGUID);
      str = uuid_normalize(guidStr);
      RpcStringFreeA(&szGUID);
    }
  }
  return str;
}

#pragma warning(push)
#pragma warning(disable : 4996)
extern std::string device_id_win32(bool);
std::string device_id() {
  std::string deviceId = device_id_win32(false);
  OSVERSIONINFOW osvi;
  ZeroMemory(&osvi, sizeof(osvi));
  osvi.dwOSVersionInfoSize = sizeof(osvi);
  GetVersionExW(&osvi);
  char verStr[100];
  sprintf(verStr, "Windows %d.%d.%d", osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber);
  if (!deviceId.empty())
    return deviceId + "/" + verStr;
  else
    return verStr;
}

std::string device_info() { return device_id_win32(true); }

std::string system_info() {
  OSVERSIONINFOW osvi;
  ZeroMemory(&osvi, sizeof(osvi));
  osvi.dwOSVersionInfoSize = sizeof(osvi);
  GetVersionExW(&osvi);
  char verStr[100];
  sprintf(verStr, "Windows/%d.%d.%d", osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber);
  return verStr;
}
#pragma warning(pop)

static bool is_dir_writable(const std::string& dir) {
  char buffer[MAX_PATH];
  if (::GetTempFileNameA(dir.c_str(), "agora", 0, buffer)) {
    ::DeleteFileA(buffer);
    return true;
  }
  return false;
}

std::wstring wjoin_path(const std::wstring& path1, const std::wstring& path2) {
  if (path1.empty())
    return path2;
  else if (path2.empty())
    return path1;
  wchar_t bch = *path1.rbegin();
  if (bch == L'/' || bch == L'\\')
    return path1 + path2;
  else
    return path1 + L'/' + path2;
}

std::string get_config_dir() {
  char workingDir[MAX_PATH];
  workingDir[0] = '\0';
  ::GetCurrentDirectoryA(MAX_PATH, workingDir);
  workingDir[MAX_PATH - 1] = '\0';
  return std::string(workingDir);
}

std::string get_data_dir() {
  std::wstring path;  // = get_config_dir();
  //	if (is_dir_writable(path))
  //		return path;
  wchar_t buffer[MAX_PATH];
  // dont call SHGetFolderPathA, which returns invalid directory if containing chinese characters
  HRESULT hr = ::SHGetFolderPathW(0, CSIDL_LOCAL_APPDATA, 0, 0, buffer);
  if (SUCCEEDED(hr)) {
    path.assign(buffer);
    path += L"\\Agora";
    if (::CreateDirectoryW(path.c_str(), NULL) || GetLastError() == ERROR_ALREADY_EXISTS) {
      ::GetModuleFileNameW(NULL, buffer, MAX_PATH);
      wchar_t* p0 = wcsrchr(buffer, L'\\');
      wchar_t* p1 = wcsrchr(buffer, L'.');
      if (p0 && p1) {
        p0++;
        *p1 = L'\0';
        path = wjoin_path(path, p0);
        if (::CreateDirectoryW(path.c_str(), NULL) || GetLastError() == ERROR_ALREADY_EXISTS) {
          char buffer2[MAX_PATH];
          WideCharToMultiByte(CP_UTF8, 0, path.c_str(), -1, buffer2, sizeof(buffer2), NULL, NULL);
          return buffer2;
        }
      }
    }
  }
  return "";
}

static uint64_t convertFileTime(const FILETIME& time) {
  ULARGE_INTEGER ui;
  ui.HighPart = time.dwHighDateTime;
  ui.LowPart = time.dwLowDateTime;
  return ui.QuadPart;
}

static void getTotalCpuUsage(uint64_t& idle, uint64_t& used) {
  FILETIME idleTime, kernelTime, userTime;
  GetSystemTimes(&idleTime, &kernelTime, &userTime);
  idle = convertFileTime(idleTime);
  used = convertFileTime(kernelTime) + convertFileTime(userTime);
}

static void getAppCpuUsage(uint64_t& used) {
  FILETIME creationTime, exitTime, kernelTime, userTime;
  GetProcessTimes(::GetCurrentProcess(), &creationTime, &exitTime, &kernelTime, &userTime);
  used = convertFileTime(kernelTime) + convertFileTime(userTime);
}

std::string ansi2utf8(const std::string& ansi) {
  if (ansi.empty()) return std::move(std::string());
  int len = MultiByteToWideChar(CP_ACP, 0, ansi.c_str(), ansi.size(), nullptr, 0);
  wchar_t* widestr = new wchar_t[len + 1];
  if (!widestr) return std::move(std::string());
  ZeroMemory(widestr, sizeof(wchar_t) * (len + 1));
  MultiByteToWideChar(CP_ACP, 0, ansi.c_str(), ansi.size(), widestr, len + 1);

  int utf8len = WideCharToMultiByte(CP_UTF8, 0, widestr, len + 1, nullptr, 0, nullptr, nullptr);

  char* utf8str = new char[utf8len + 1];
  if (!utf8str) {
    delete[] widestr;
    return std::move(std::string());
  }
  ZeroMemory(utf8str, utf8len + 1);
  WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)widestr, len + 1, utf8str, utf8len + 1, nullptr, nullptr);
  std::string result = utf8str;
  delete[] widestr;
  delete[] utf8str;
  return result;
}

std::wstring ansi2wide(const std::string& ansi) {
  if (ansi.empty()) return std::move(std::wstring());
  int len = MultiByteToWideChar(CP_ACP, 0, ansi.c_str(), ansi.size(), nullptr, 0);
  wchar_t* widestr = new wchar_t[len + 1];
  if (!widestr) return std::move(std::wstring());
  ZeroMemory(widestr, sizeof(wchar_t) * (len + 1));
  MultiByteToWideChar(CP_ACP, 0, ansi.c_str(), ansi.size(), widestr, len + 1);
  std::wstring result(widestr);
  delete[] widestr;
  return std::move(result);
}

std::wstring utf82wide(const std::string& utf8) {
  if (utf8.empty()) return std::move(std::wstring());
  int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), utf8.size(), nullptr, 0);
  wchar_t* buf = new wchar_t[len + 1];
  if (!buf) return std::move(std::wstring());
  ZeroMemory(buf, sizeof(wchar_t) * (len + 1));
  MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), utf8.size(), buf, sizeof(wchar_t) * (len + 1));
  std::wstring result(buf);
  delete[] buf;
  return std::move(result);
}

std::string wide2utf8(const std::wstring& wide) {
  if (wide.empty()) return std::move(std::string());
  int len =
      WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), wide.size(), nullptr, 0, nullptr, nullptr);
  char* buf = new char[len + 1];
  if (!buf) return std::move(std::string());
  ZeroMemory(buf, len + 1);
  WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), wide.size(), buf, len + 1, nullptr, nullptr);
  std::string result(buf);
  delete[] buf;
  return std::move(result);
}

std::string wide2ansi(const std::wstring& wide) {
  if (wide.empty()) return std::move(std::string());
  int len = WideCharToMultiByte(CP_ACP, 0, wide.c_str(), wide.size(), nullptr, 0, nullptr, nullptr);
  char* buf = new char[len + 1];
  if (!buf) return std::move(std::string());
  ZeroMemory(buf, len + 1);
  WideCharToMultiByte(CP_ACP, 0, wide.c_str(), wide.size(), buf, len + 1, nullptr, nullptr);
  std::string result(buf);
  delete[] buf;
  return std::move(result);
}

cpu_usage::cpu_usage() {
  getTotalCpuUsage(m_lastTotalIdle, m_lastTotalUsed);
  getAppCpuUsage(m_lastAppUsed);
}

bool cpu_usage::get_usage(unsigned int& total, unsigned int& me) {
  uint64_t idle, totalUsed, appUsed;
  getTotalCpuUsage(idle, totalUsed);
  getAppCpuUsage(appUsed);

  int64_t i = idle - m_lastTotalIdle;
  int64_t u = (totalUsed - m_lastTotalUsed);

  // if u == 0
  if (u == 0) {
    total = 0;
    me = 0;
  } else {
    total = (unsigned int)((u - i) * 10000 / u);
    me = (unsigned int)((appUsed - m_lastAppUsed) * 10000 / u);
  }
  //    unsigned int ii = i * 10000 / u;
  //    log(LOG_INFO, "cpu system/app/idle %u.%u/%u.%u/%u.%u", total / 100, total % 100, me / 100,
  //    me % 100, ii / 100, ii % 100);

  m_lastTotalIdle = idle;
  m_lastTotalUsed = totalUsed;
  m_lastAppUsed = appUsed;
  return true;
}

int cpu_usage::get_cores() { return std::thread::hardware_concurrency(); }

int cpu_usage::get_online_cores() { return 1; }

int cpu_usage::get_battery_life() {
  SYSTEM_POWER_STATUS sps;
  if (GetSystemPowerStatus(&sps)) {
    return sps.BatteryLifePercent;
  }
  return cpu_usage::UNKNOWN_BATTERY_LIFE;
}

}  // namespace commons
}  // namespace agora
