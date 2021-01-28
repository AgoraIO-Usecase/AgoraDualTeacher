//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "utils/net/ip_type.h"
#include <iphlpapi.h>
#include "utils/net/network_helper.h"
#include "utils/tools/sys_type.h"
#include "utils/tools/util.h"
#include <wlanapi.h>
#include <thread>
#include <vector>
#include "utils/log/log.h"

#define WORKING_BUFFER_SIZE 15000
#define MAX_TRIES 3

using namespace agora::commons;

namespace agora {
namespace commons {
namespace detail {
class IpHelperLib {
  typedef ULONG(WINAPI *PFN_GetAdaptersInfo)(PIP_ADAPTER_INFO AdapterInfo,
                                             PULONG SizePointer);
  typedef DWORD(WINAPI *PFN_GetNetworkParams)(PFIXED_INFO pFixedInfo,
                                              PULONG pOutBufLen);
  typedef ULONG(WINAPI *PFN_GetAdaptersAddresses)(
      ULONG Family, ULONG Flags, PVOID Reserved,
      PIP_ADAPTER_ADDRESSES AdapterAddresses, PULONG SizePointer);
  typedef DWORD(WINAPI *PFN_NotifyAddrChange)(PHANDLE Handle,
                                              LPOVERLAPPED overlapped);
  typedef BOOL(WINAPI *PFN_CancelIPChangeNotify)(LPOVERLAPPED notifyOverlapped);
  typedef HANDLE(WINAPI *PFN_IcmpCreateFile)();
  typedef BOOL(WINAPI *PFN_IcmpCloseHandle)(HANDLE IcmpHandle);
  typedef DWORD(WINAPI *PFN_IcmpSendEcho2)(
      HANDLE IcmpHandle, HANDLE Event, PVOID ApcRoutine, PVOID ApcContext,
      IPAddr DestinationAddress, LPVOID RequestData, WORD RequestSize,
      PIP_OPTION_INFORMATION RequestOptions, LPVOID ReplyBuffer,
      DWORD ReplySize, DWORD Timeout);
  typedef DWORD(WINAPI *PFN_IcmpParseReplies)(LPVOID ReplyBuffer,
                                              DWORD ReplySize);
  typedef DWORD(WINAPI *PFN_Icmp6SendEcho2)(
      HANDLE Icmp6Handle, HANDLE Event, PVOID ApcRoutine, PVOID ApcContext,
      sockaddr_in6 *SourceAddress, sockaddr_in6 *DestinationAddress,
      LPVOID RequestData, WORD RequestSize,
      PIP_OPTION_INFORMATION RequestOptions, LPVOID ReplyBuffer,
      DWORD ReplySize, DWORD Timeout);
  typedef DWORD(WINAPI *PFN_Icmp6ParseReplies)(LPVOID ReplyBuffer,
                                               DWORD ReplySize);

 public:
  IpHelperLib()
      : m_firstInit(true),
        m_pfnGetAdaptersInfo(NULL),
        m_pfnGetNetworkParams(NULL),
        m_pfnGetAdaptersAddresses(NULL),
        m_pfnNotifyAddrChange(NULL),
        m_pfnCancelIPChangeNotify(NULL),
        m_pfnIcmpCreateFile(NULL),
        m_pfnIcmpCloseHandle(NULL),
        m_pfnIcmpSendEcho2(NULL),
        m_pfnIcmpParseReplies(NULL),
        m_pfnIcmp6CreateFile(NULL),
        m_pfnIcmp6SendEcho2(NULL),
        m_pfnIcmp6ParseReplies(NULL),
        m_isVistaOrAbove(pri_isVistaOrAbove()) {}
  bool initialize() {
    if (!m_firstInit) return isValid();
    m_firstInit = false;

    m_hDll = LoadLibrary(TEXT("IPHLPAPI.dll"));
    if (m_hDll) {
      *(FARPROC *)&m_pfnGetAdaptersInfo =
          GetProcAddress(m_hDll, "GetAdaptersInfo");
      *(FARPROC *)&m_pfnGetNetworkParams =
          GetProcAddress(m_hDll, "GetNetworkParams");
      *(FARPROC *)&m_pfnGetAdaptersAddresses =
          GetProcAddress(m_hDll, "GetAdaptersAddresses");
      *(FARPROC *)&m_pfnNotifyAddrChange =
          GetProcAddress(m_hDll, "NotifyAddrChange");
      *(FARPROC *)&m_pfnCancelIPChangeNotify =
          GetProcAddress(m_hDll, "CancelIPChangeNotify");
      *(FARPROC *)&m_pfnIcmpCloseHandle =
          GetProcAddress(m_hDll, "IcmpCloseHandle");
      *(FARPROC *)&m_pfnIcmpCreateFile =
          GetProcAddress(m_hDll, "IcmpCreateFile");
      *(FARPROC *)&m_pfnIcmpSendEcho2 = GetProcAddress(m_hDll, "IcmpSendEcho2");
      *(FARPROC *)&m_pfnIcmpParseReplies =
          GetProcAddress(m_hDll, "IcmpParseReplies");
      *(FARPROC *)&m_pfnIcmp6CreateFile =
          GetProcAddress(m_hDll, "Icmp6CreateFile");
      *(FARPROC *)&m_pfnIcmp6SendEcho2 =
          GetProcAddress(m_hDll, "Icmp6SendEcho2");
      *(FARPROC *)&m_pfnIcmp6ParseReplies =
          GetProcAddress(m_hDll, "Icmp6ParseReplies");
    }
    return isValid();
  }
  ~IpHelperLib() {
    if (m_hDll) FreeLibrary(m_hDll);
  }
  ULONG GetAdaptersInfo(PIP_ADAPTER_INFO AdapterInfo, PULONG SizePointer) {
    if (m_pfnGetAdaptersInfo)
      return m_pfnGetAdaptersInfo(AdapterInfo, SizePointer);
    return ERROR_NOT_SUPPORTED;
  }
  DWORD GetNetworkParams(PFIXED_INFO pFixedInfo, PULONG pOutBufLen) {
    if (m_pfnGetNetworkParams)
      return m_pfnGetNetworkParams(pFixedInfo, pOutBufLen);
    return ERROR_NOT_SUPPORTED;
  }
  ULONG GetAdaptersAddresses(ULONG Family, ULONG Flags, PVOID Reserved,
                             PIP_ADAPTER_ADDRESSES AdapterAddresses,
                             PULONG SizePointer) {
    if (m_pfnGetAdaptersAddresses)
      return m_pfnGetAdaptersAddresses(Family, Flags, Reserved,
                                       AdapterAddresses, SizePointer);
    return ERROR_NOT_SUPPORTED;
  }

  DWORD NotifyAddrChange(PHANDLE Handle, LPOVERLAPPED overlapped) {
    if (m_pfnNotifyAddrChange) return m_pfnNotifyAddrChange(Handle, overlapped);
    return ERROR_NOT_SUPPORTED;
  }

  BOOL CancelIPChangeNotify(LPOVERLAPPED notifyOverlapped) {
    if (m_pfnCancelIPChangeNotify)
      return m_pfnCancelIPChangeNotify(notifyOverlapped);
    return FALSE;
  }
  HANDLE IcmpCreateFile() {
    if (m_pfnIcmpCreateFile) return m_pfnIcmpCreateFile();
    return INVALID_HANDLE_VALUE;
  }
  BOOL IcmpCloseHandle(HANDLE IcmpHandle) {
    if (m_pfnIcmpCloseHandle) return m_pfnIcmpCloseHandle(IcmpHandle);
    return FALSE;
  }
  DWORD IcmpSendEcho2(HANDLE IcmpHandle, HANDLE Event, PVOID ApcRoutine,
                      PVOID ApcContext, IPAddr DestinationAddress,
                      LPVOID RequestData, WORD RequestSize,
                      PIP_OPTION_INFORMATION RequestOptions, LPVOID ReplyBuffer,
                      DWORD ReplySize, DWORD Timeout) {
    if (m_pfnIcmpSendEcho2)
      return m_pfnIcmpSendEcho2(IcmpHandle, Event, ApcRoutine, ApcContext,
                                DestinationAddress, RequestData, RequestSize,
                                RequestOptions, ReplyBuffer, ReplySize,
                                Timeout);
    return 0;
  }
  DWORD IcmpParseReplies(LPVOID ReplyBuffer, DWORD ReplySize) {
    if (m_pfnIcmpParseReplies)
      return m_pfnIcmpParseReplies(ReplyBuffer, ReplySize);
    return 0;
  }
  HANDLE Icmp6CreateFile() {
    if (m_pfnIcmp6CreateFile) return m_pfnIcmp6CreateFile();
    return INVALID_HANDLE_VALUE;
  }
  DWORD Icmp6SendEcho2(HANDLE IcmpHandle, HANDLE Event, PVOID ApcRoutine,
                       PVOID ApcContext, sockaddr_in6 *SourceAddress,
                       sockaddr_in6 *DestinationAddress, LPVOID RequestData,
                       WORD RequestSize, PIP_OPTION_INFORMATION RequestOptions,
                       LPVOID ReplyBuffer, DWORD ReplySize, DWORD Timeout) {
    if (m_pfnIcmp6SendEcho2)
      return m_pfnIcmp6SendEcho2(IcmpHandle, Event, ApcRoutine, ApcContext,
                                 SourceAddress, DestinationAddress, RequestData,
                                 RequestSize, RequestOptions, ReplyBuffer,
                                 ReplySize, Timeout);
    return 0;
  }
  DWORD Icmp6ParseReplies(LPVOID ReplyBuffer, DWORD ReplySize) {
    if (m_pfnIcmp6ParseReplies)
      return m_pfnIcmp6ParseReplies(ReplyBuffer, ReplySize);
    return 0;
  }

  bool isValid() {
    return m_pfnGetAdaptersInfo != NULL && m_pfnGetNetworkParams != NULL &&
           m_pfnGetAdaptersAddresses != NULL && m_pfnNotifyAddrChange != NULL &&
           m_pfnCancelIPChangeNotify != NULL && m_pfnIcmpCloseHandle != NULL &&
           m_pfnIcmpCreateFile != NULL && m_pfnIcmpSendEcho2 != NULL &&
           m_pfnIcmpParseReplies != NULL && m_pfnIcmp6CreateFile != NULL &&
           m_pfnIcmp6SendEcho2 != NULL && m_pfnIcmp6ParseReplies != NULL;
  }
  bool isVistaOrAbove() { return m_isVistaOrAbove; }

 private:
#pragma warning(push)
#pragma warning(disable : 4996)
  bool pri_isVistaOrAbove() {
    OSVERSIONINFOW osvi;
    ZeroMemory(&osvi, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionExW(&osvi);
    return osvi.dwMajorVersion >= 6;
  }
#pragma warning(pop)
 private:
  bool m_firstInit;
  bool m_isVistaOrAbove;
  HINSTANCE m_hDll;
  PFN_GetAdaptersInfo m_pfnGetAdaptersInfo;
  PFN_GetNetworkParams m_pfnGetNetworkParams;
  PFN_GetAdaptersAddresses m_pfnGetAdaptersAddresses;
  PFN_NotifyAddrChange m_pfnNotifyAddrChange;
  PFN_CancelIPChangeNotify m_pfnCancelIPChangeNotify;
  PFN_IcmpCloseHandle m_pfnIcmpCloseHandle;
  PFN_IcmpCreateFile m_pfnIcmpCreateFile;
  PFN_IcmpSendEcho2 m_pfnIcmpSendEcho2;
  PFN_IcmpParseReplies m_pfnIcmpParseReplies;
  PFN_IcmpCreateFile m_pfnIcmp6CreateFile;
  PFN_Icmp6SendEcho2 m_pfnIcmp6SendEcho2;
  PFN_Icmp6ParseReplies m_pfnIcmp6ParseReplies;
};
class WlanHelperLib {
  typedef DWORD(WINAPI *PFN_WlanOpenHandle)(DWORD dwClientVersion,
                                            PVOID pReserved,
                                            PDWORD pdwNegotiatedVersion,
                                            PHANDLE phClientHandle);
  typedef DWORD(WINAPI *PFN_WlanCloseHandle)(HANDLE hClientHandle,
                                             PVOID pReserved);
  typedef VOID(WINAPI *PFN_WlanFreeMemory)(PVOID pMemory);
  typedef DWORD(WINAPI *PFN_WlanEnumInterfaces)(
      HANDLE hClientHandle, PVOID pReserved,
      PWLAN_INTERFACE_INFO_LIST *ppInterfaceList);
  typedef DWORD(WINAPI *PFN_WlanGetAvailableNetworkList)(
      HANDLE hClientHandle, const GUID *pInterfaceGuid, DWORD dwFlags,
      PVOID pReserved, PWLAN_AVAILABLE_NETWORK_LIST *ppAvailableNetworkList);
  typedef DWORD(WINAPI *PFN_WlanGetNetworkBssList)(
      HANDLE hClientHandle, const GUID *pInterfaceGuid,
      const PDOT11_SSID pDot11Ssid, DOT11_BSS_TYPE dot11BssType,
      BOOL bSecurityEnabled, PVOID pReserved, PWLAN_BSS_LIST *ppWlanBssList);
  typedef DWORD(WINAPI *PFN_WlanQueryInterface)(
      HANDLE hClientHandle, const GUID *pInterfaceGuid, WLAN_INTF_OPCODE OpCode,
      PVOID pReserved, PDWORD pdwDataSize, PVOID *ppData,
      PWLAN_OPCODE_VALUE_TYPE pWlanOpcodeValueType);

 public:
  WlanHelperLib()
      : m_firstInit(true),
        m_pfnWlanOpenHandle(NULL),
        m_pfnWlanCloseHandle(NULL),
        m_pfnWlanFreeMemory(NULL),
        m_pfnWlanEnumInterfaces(NULL),
        m_pfnWlanGetAvailableNetworkList(NULL),
        m_pfnWlanGetNetworkBssList(NULL),
        m_pfnWlanQueryInterface(NULL) {}
  bool initialize() {
    if (!m_firstInit) return isValid();
    m_firstInit = false;

    m_hDll = LoadLibrary(TEXT("wlanapi.dll"));
    if (m_hDll) {
      *(FARPROC *)&m_pfnWlanOpenHandle =
          GetProcAddress(m_hDll, "WlanOpenHandle");
      *(FARPROC *)&m_pfnWlanCloseHandle =
          GetProcAddress(m_hDll, "WlanCloseHandle");
      *(FARPROC *)&m_pfnWlanFreeMemory =
          GetProcAddress(m_hDll, "WlanFreeMemory");
      *(FARPROC *)&m_pfnWlanEnumInterfaces =
          GetProcAddress(m_hDll, "WlanEnumInterfaces");
      *(FARPROC *)&m_pfnWlanGetAvailableNetworkList =
          GetProcAddress(m_hDll, "WlanGetAvailableNetworkList");
      *(FARPROC *)&m_pfnWlanGetNetworkBssList =
          GetProcAddress(m_hDll, "WlanGetNetworkBssList");
      *(FARPROC *)&m_pfnWlanQueryInterface =
          GetProcAddress(m_hDll, "WlanQueryInterface");
    }
    return isValid();
  }
  ~WlanHelperLib() {
    if (m_hDll) FreeLibrary(m_hDll);
  }
  DWORD WlanOpenHandle(DWORD dwClientVersion, PVOID pReserved,
                       PDWORD pdwNegotiatedVersion, PHANDLE phClientHandle) {
    if (m_pfnWlanOpenHandle)
      return m_pfnWlanOpenHandle(dwClientVersion, pReserved,
                                 pdwNegotiatedVersion, phClientHandle);
    return ERROR_NOT_SUPPORTED;
  }
  DWORD WlanCloseHandle(HANDLE hClientHandle, PVOID pReserved) {
    if (m_pfnWlanCloseHandle)
      return m_pfnWlanCloseHandle(hClientHandle, pReserved);
    return ERROR_NOT_SUPPORTED;
  }

  VOID WlanFreeMemory(PVOID pMemory) {
    if (m_pfnWlanFreeMemory) m_pfnWlanFreeMemory(pMemory);
  }

  DWORD WlanEnumInterfaces(HANDLE hClientHandle, PVOID pReserved,
                           PWLAN_INTERFACE_INFO_LIST *ppInterfaceList) {
    if (m_pfnWlanEnumInterfaces)
      return m_pfnWlanEnumInterfaces(hClientHandle, pReserved, ppInterfaceList);
    return ERROR_NOT_SUPPORTED;
  }

  DWORD WlanGetAvailableNetworkList(
      HANDLE hClientHandle, const GUID *pInterfaceGuid, DWORD dwFlags,
      PVOID pReserved, PWLAN_AVAILABLE_NETWORK_LIST *ppAvailableNetworkList) {
    if (m_pfnWlanGetAvailableNetworkList)
      return m_pfnWlanGetAvailableNetworkList(hClientHandle, pInterfaceGuid,
                                              dwFlags, pReserved,
                                              ppAvailableNetworkList);
    return ERROR_NOT_SUPPORTED;
  }
  DWORD WlanGetNetworkBssList(HANDLE hClientHandle, const GUID *pInterfaceGuid,
                              const PDOT11_SSID pDot11Ssid,
                              DOT11_BSS_TYPE dot11BssType,
                              BOOL bSecurityEnabled, PVOID pReserved,
                              PWLAN_BSS_LIST *ppWlanBssList) {
    if (m_pfnWlanGetNetworkBssList)
      return m_pfnWlanGetNetworkBssList(
          hClientHandle, pInterfaceGuid, pDot11Ssid, dot11BssType,
          bSecurityEnabled, pReserved, ppWlanBssList);
    return ERROR_NOT_SUPPORTED;
  }
  DWORD WlanQueryInterface(HANDLE hClientHandle, const GUID *pInterfaceGuid,
                           WLAN_INTF_OPCODE OpCode, PVOID pReserved,
                           PDWORD pdwDataSize, PVOID *ppData,
                           PWLAN_OPCODE_VALUE_TYPE pWlanOpcodeValueType) {
    if (m_pfnWlanQueryInterface)
      return m_pfnWlanQueryInterface(hClientHandle, pInterfaceGuid, OpCode,
                                     pReserved, pdwDataSize, ppData,
                                     pWlanOpcodeValueType);
    return ERROR_NOT_SUPPORTED;
  }
  DWORD QueryRssi(HANDLE hClientHandle, const GUID *pInterfaceGuid, int *rssi) {
    DWORD dwDataSize;
    PVOID pData = NULL;
    DWORD dwResult =
        WlanQueryInterface(hClientHandle, pInterfaceGuid, wlan_intf_opcode_rssi,
                           NULL, &dwDataSize, &pData, NULL);
    if (dwResult == ERROR_SUCCESS && pData && dwDataSize == sizeof(int))
      *rssi = *(int *)pData;
    if (pData) this->WlanFreeMemory(pData);
    return dwResult;
  }
  DWORD QueryChannelNumber(HANDLE hClientHandle, const GUID *pInterfaceGuid,
                           int *channelNumber) {
    DWORD dwDataSize;
    PVOID pData = NULL;
    DWORD dwResult = WlanQueryInterface(hClientHandle, pInterfaceGuid,
                                        wlan_intf_opcode_channel_number, NULL,
                                        &dwDataSize, &pData, NULL);
    if (dwResult == ERROR_SUCCESS && pData && dwDataSize == sizeof(int))
      *channelNumber = *(int *)pData;
    if (pData) this->WlanFreeMemory(pData);
    return dwResult;
  }
  DWORD QueryConnectionAttributes(HANDLE hClientHandle,
                                  const GUID *pInterfaceGuid,
                                  PWLAN_CONNECTION_ATTRIBUTES *ppWlanConnAttr) {
    DWORD dwDataSize;
    PVOID pData = NULL;
    DWORD dwResult = WlanQueryInterface(hClientHandle, pInterfaceGuid,
                                        wlan_intf_opcode_current_connection,
                                        NULL, &dwDataSize, &pData, NULL);
    if (dwResult == ERROR_SUCCESS && pData &&
        dwDataSize == sizeof(WLAN_CONNECTION_ATTRIBUTES))
      *ppWlanConnAttr = (PWLAN_CONNECTION_ATTRIBUTES)pData;
    return dwResult;
  }
  bool isValid() {
    return m_pfnWlanOpenHandle != NULL && m_pfnWlanCloseHandle != NULL &&
           m_pfnWlanFreeMemory != NULL && m_pfnWlanEnumInterfaces != NULL &&
           m_pfnWlanGetAvailableNetworkList != NULL;
  }

 private:
  bool m_firstInit;
  HINSTANCE m_hDll;
  PFN_WlanOpenHandle m_pfnWlanOpenHandle;
  PFN_WlanCloseHandle m_pfnWlanCloseHandle;
  PFN_WlanFreeMemory m_pfnWlanFreeMemory;
  PFN_WlanEnumInterfaces m_pfnWlanEnumInterfaces;
  PFN_WlanGetAvailableNetworkList m_pfnWlanGetAvailableNetworkList;
  PFN_WlanGetNetworkBssList m_pfnWlanGetNetworkBssList;
  PFN_WlanQueryInterface m_pfnWlanQueryInterface;
};

IpHelperLib ipHelperLib;
static WlanHelperLib wlanHelperLib;

struct AdapterIpAddressInfo {
  network::NetworkType networkType;
  std::string localIp4;
  std::string gatewayIp4;
  std::vector<std::string> localIp6List;
  std::string gatewayIp6;
};

static bool GetAdapterAddressByIpAddress(
    ip_t &localIp, ip_t &gatewayIp,
    BYTE adapterAddress[MAX_ADAPTER_ADDRESS_LENGTH]) {
  if (!ipHelperLib.initialize()) return false;

  PIP_ADAPTER_INFO pAdapterInfo;
  PIP_ADAPTER_INFO pAdapter = NULL;
  DWORD dwRetVal = 0;
  ip_t tmpIp;
  bool found = false;

  ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
  pAdapterInfo = (IP_ADAPTER_INFO *)malloc(sizeof(IP_ADAPTER_INFO));
  if (pAdapterInfo == NULL) {
    return found;
  }
  // Make an initial call to GetAdaptersInfo to get
  // the necessary size into the ulOutBufLen variable
  if (ipHelperLib.GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) ==
      ERROR_BUFFER_OVERFLOW) {
    free(pAdapterInfo);
    pAdapterInfo = (IP_ADAPTER_INFO *)malloc(ulOutBufLen);
    if (pAdapterInfo == NULL) {
      return found;
    }
  }

  if ((dwRetVal = ipHelperLib.GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) ==
      NO_ERROR) {
    pAdapter = pAdapterInfo;
    while (pAdapter) {
      if (!ip::is_valid(localIp)) {
        PIP_ADDR_STRING pGatewayAddress = &pAdapter->GatewayList;
        tmpIp = ip::from_string(pGatewayAddress->IpAddress.String);
        if (!ip::is_valid(tmpIp)) {
          pAdapter = pAdapter->Next;
          continue;
        } else
          gatewayIp = tmpIp;
      }
      PIP_ADDR_STRING pIpAddress = &pAdapter->IpAddressList;
      do {
        tmpIp = ip::from_string(pIpAddress->IpAddress.String);
        if (!ip::is_valid(localIp) || tmpIp == localIp) {
          if (!ip::is_valid(localIp)) localIp = tmpIp;
          found = true;
          memcpy(adapterAddress, pAdapter->Address, pAdapter->AddressLength);
          goto Done;
        }
        pIpAddress = pIpAddress->Next;
      } while (pIpAddress != NULL);

      pAdapter = pAdapter->Next;
    }
  } else {
    log(LOG_WARN, "GetAdaptersInfo failed with error: %d\n", dwRetVal);
  }

Done:
  if (pAdapterInfo) free(pAdapterInfo);

  return found;
}

std::string SocketAddressToIpAddress(const SOCKET_ADDRESS &socketAddress) {
  char address[128];
  DWORD length = sizeof(address) / sizeof(address[0]);
  if (WSAAddressToStringA(socketAddress.lpSockaddr,
                          socketAddress.iSockaddrLength, NULL, address,
                          &length) == 0)
    return address;
  return std::string();
}

static std::vector<std::string> GetDnsList() {
  std::vector<std::string> dnsList;

  FIXED_INFO *pFixedInfo = (FIXED_INFO *)malloc(sizeof(FIXED_INFO));
  if (!pFixedInfo) return dnsList;
  ULONG ulOutBufLen = sizeof(FIXED_INFO);

  if (ipHelperLib.GetNetworkParams(pFixedInfo, &ulOutBufLen) ==
      ERROR_BUFFER_OVERFLOW) {
    free(pFixedInfo);
    pFixedInfo = (FIXED_INFO *)malloc(ulOutBufLen);
    if (!pFixedInfo) return dnsList;
  }

  if (ipHelperLib.GetNetworkParams(pFixedInfo, &ulOutBufLen) == NO_ERROR) {
    IP_ADDR_STRING *pIpAddress = &pFixedInfo->DnsServerList;
    do {
      if (ip::is_valid(ip::from_string(pIpAddress->IpAddress.String))) {
        dnsList.emplace_back(pIpAddress->IpAddress.String);
      }
      pIpAddress = pIpAddress->Next;
    } while (pIpAddress);
  }

  /* Free allocated memory no longer needed */
  if (pFixedInfo) free(pFixedInfo);
  return dnsList;
}

static bool GetAdapterAddressInfo(AdapterIpAddressInfo &adapterInfo) {
  adapterInfo.networkType = network::NetworkType::UNKNOWN;
  if (!ipHelperLib.initialize()) return false;

  DWORD dwRetVal = 0;

  // Set the flags to pass to GetAdaptersAddresses
  ULONG flags = GAA_FLAG_INCLUDE_PREFIX | GAA_FLAG_INCLUDE_GATEWAYS;

  // default to unspecified address family (both)
  ULONG family = AF_UNSPEC;

  LPVOID lpMsgBuf = NULL;

  PIP_ADAPTER_ADDRESSES pAddresses = NULL;
  ULONG outBufLen = 0;
  ULONG Iterations = 0;

  PIP_ADAPTER_ADDRESSES pCurrAddresses = NULL;

  //	family = AF_INET;

  // Allocate a 15 KB buffer to start with.
  outBufLen = WORKING_BUFFER_SIZE;

  do {
    pAddresses = (IP_ADAPTER_ADDRESSES *)malloc(outBufLen);
    if (!pAddresses) {
      return false;
    }

    dwRetVal = ipHelperLib.GetAdaptersAddresses(family, flags, NULL, pAddresses,
                                                &outBufLen);

    if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
      free(pAddresses);
      pAddresses = NULL;
    } else {
      break;
    }

    Iterations++;

  } while ((dwRetVal == ERROR_BUFFER_OVERFLOW) && (Iterations < MAX_TRIES));

  bool checkGatewayAddress = ipHelperLib.isVistaOrAbove();

  if (dwRetVal == NO_ERROR) {
    // If successful, output some information from the data we received
    pCurrAddresses = pAddresses;
    while (pCurrAddresses) {
      if (pCurrAddresses->OperStatus != IfOperStatusUp ||
          (checkGatewayAddress && !pCurrAddresses->FirstGatewayAddress) ||
          !pCurrAddresses->FirstUnicastAddress) {
        pCurrAddresses = pCurrAddresses->Next;
      } else {
        PIP_ADAPTER_UNICAST_ADDRESS pUnicast =
            pCurrAddresses->FirstUnicastAddress;
        while (pUnicast) {
          if (pUnicast->Address.lpSockaddr->sa_family == AF_INET)
            adapterInfo.localIp4 = SocketAddressToIpAddress(pUnicast->Address);
          else if (checkGatewayAddress &&
                   pUnicast->Address.lpSockaddr->sa_family == AF_INET6)
            adapterInfo.localIp6List.emplace_back(
                SocketAddressToIpAddress(pUnicast->Address));
          pUnicast = pUnicast->Next;
        }
        if (checkGatewayAddress) {
          PIP_ADAPTER_GATEWAY_ADDRESS_LH pGateway =
              pCurrAddresses->FirstGatewayAddress;
          while (pGateway) {
            if (pGateway->Address.lpSockaddr->sa_family == AF_INET)
              adapterInfo.gatewayIp4 =
                  SocketAddressToIpAddress(pGateway->Address);
            else if (pGateway->Address.lpSockaddr->sa_family == AF_INET6)
              adapterInfo.gatewayIp6 =
                  SocketAddressToIpAddress(pGateway->Address);
            pGateway = pGateway->Next;
          }
        }

        switch (pCurrAddresses->IfType) {
          case IF_TYPE_ETHERNET_CSMACD:
            adapterInfo.networkType = network::NetworkType::LAN;
            break;
          case IF_TYPE_IEEE80211:
            adapterInfo.networkType = network::NetworkType::WIFI;
            break;
          default:
            adapterInfo.networkType = network::NetworkType::UNKNOWN;
            break;
        }

        break;
      }
    }
  } else {
    log(LOG_WARN, "Failed to get network adapter addresses, err=%d", dwRetVal);
    if (dwRetVal == ERROR_NO_DATA)
      log(LOG_WARN, "No addresses were found when getting network adapters");
    else {
      if (FormatMessage(
              FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS,
              NULL, dwRetVal, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
              // Default language
              (LPTSTR)&lpMsgBuf, 0, NULL)) {
        log(LOG_WARN, "failed to get network adapter, err=%s", lpMsgBuf);
        LocalFree(lpMsgBuf);
      }
    }
  }

  if (pAddresses) {
    free(pAddresses);
  }

  return true;
}

static int GetNetworkAdapterOpStatus(
    BYTE adapterAddress[MAX_ADAPTER_ADDRESS_LENGTH]) {
  if (!ipHelperLib.initialize()) return (int)network::NetworkType::UNKNOWN;

  DWORD dwRetVal = 0;

  network::NetworkType networkType = network::NetworkType::UNKNOWN;

  // Set the flags to pass to GetAdaptersAddresses
  ULONG flags = GAA_FLAG_INCLUDE_PREFIX;

  // default to unspecified address family (both)
  ULONG family = AF_UNSPEC;

  LPVOID lpMsgBuf = NULL;

  PIP_ADAPTER_ADDRESSES pAddresses = NULL;
  ULONG outBufLen = 0;
  ULONG Iterations = 0;

  PIP_ADAPTER_ADDRESSES pCurrAddresses = NULL;

  family = AF_INET;

  // Allocate a 15 KB buffer to start with.
  outBufLen = WORKING_BUFFER_SIZE;

  do {
    pAddresses = (IP_ADAPTER_ADDRESSES *)malloc(outBufLen);
    if (!pAddresses) {
      return (int)networkType;
    }

    dwRetVal = ipHelperLib.GetAdaptersAddresses(family, flags, NULL, pAddresses,
                                                &outBufLen);

    if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
      free(pAddresses);
      pAddresses = NULL;
    } else {
      break;
    }

    Iterations++;

  } while ((dwRetVal == ERROR_BUFFER_OVERFLOW) && (Iterations < MAX_TRIES));

  if (dwRetVal == NO_ERROR) {
    // If successful, output some information from the data we received
    pCurrAddresses = pAddresses;
    while (pCurrAddresses) {
      if (!memcmp(adapterAddress, pCurrAddresses->PhysicalAddress,
                  pCurrAddresses->PhysicalAddressLength)) {
        switch (pCurrAddresses->IfType) {
          case IF_TYPE_ETHERNET_CSMACD:
            networkType = network::NetworkType::LAN;
            break;
          case IF_TYPE_IEEE80211:
            networkType = network::NetworkType::WIFI;
            break;
          default:
            networkType = network::NetworkType::UNKNOWN;
            break;
        }
        goto Done;
      }

      pCurrAddresses = pCurrAddresses->Next;
    }
  } else {
    log(LOG_WARN, "Failed to get network adapter addresses, err=%d", dwRetVal);
    if (dwRetVal == ERROR_NO_DATA)
      log(LOG_WARN, "No addresses were found when getting network adapters");
    else {
      if (FormatMessage(
              FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS,
              NULL, dwRetVal, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
              // Default language
              (LPTSTR)&lpMsgBuf, 0, NULL)) {
        log(LOG_WARN, "failed to get network adapter, err=%s", lpMsgBuf);
        LocalFree(lpMsgBuf);
        if (pAddresses) free(pAddresses);
      }
    }
  }

Done:
  if (pAddresses) {
    free(pAddresses);
  }

  return (int)networkType;
}

static int rssi_to_quality(int rssi) {
  if (rssi <= -100)
    return 0;
  else if (rssi >= -50)
    return 100;
  else
    return 2 * (rssi + 100);
}

static int quality_to_rssi(int quality) {
  if (quality <= 0)
    return -100;
  else if (quality >= 100)
    return -50;
  else
    return -100 + (quality / 2);
}

static network::SignalLevel rssi_to_level(int rssi) {
  /*
  Less than -50dB (-40, -30 and -20) = 100% of signal strength
  From -51 to -55dB= 90%
  From -56 to -62dB=80%
  From -63 to -65dB=75%
  The below is not good enough for Apple devices
  From -66 to 68dB=70%
  From -69 to 74dB= 60%
  From -75 to 79dB= 50%
  From -80 to -83dB=30%
  Windows laptops can work fine on -80dB however with slower speeds


  Anything above -80 is considered good, and anything below -105 is bad.
  iOS:
          5 bars: [-51, -99)
      4 bars: [-99, -101)
          3 bars: [-101, -103)
          2 bars: [-103, -107)
          1 bars: [-107, -113)
  */
  if (rssi <= -50)
    return network::SignalLevel::GREAT;
  else if (rssi <= -55)
    return network::SignalLevel::GREAT;
  else if (rssi <= -65)
    return network::SignalLevel::GOOD;
  else if (rssi <= -79)
    return network::SignalLevel::MODERATE;
  else if (rssi <= -83)
    return network::SignalLevel::POOR;
  else
    return network::SignalLevel::NONE;
}

static network::SignalLevel quality_to_level(int quality) {
  if (quality <= 0) return network::SignalLevel::NONE;
  return (network::SignalLevel)(1 + quality / 25);
}

static bool GetWlanNetowrkInfo(network::network_info_t &networkInfo) {
  if (!wlanHelperLib.initialize()) return 0;

  HANDLE hClient = NULL;
  DWORD dwMaxClient = 2;
  DWORD dwCurVersion = 0;
  DWORD dwResult = 0;
  bool retval = false;

  PWLAN_INTERFACE_INFO_LIST pIfList = NULL;

  dwResult =
      wlanHelperLib.WlanOpenHandle(dwMaxClient, NULL, &dwCurVersion, &hClient);
  if (dwResult != ERROR_SUCCESS) goto Done;

  dwResult = wlanHelperLib.WlanEnumInterfaces(hClient, NULL, &pIfList);
  if (dwResult != ERROR_SUCCESS || !pIfList) goto Done;

  for (int i = 0; !retval && i < (int)pIfList->dwNumberOfItems; i++) {
    PWLAN_INTERFACE_INFO pIfInfo =
        (PWLAN_INTERFACE_INFO)&pIfList->InterfaceInfo[i];

    PWLAN_AVAILABLE_NETWORK_LIST pBssList = NULL;
    dwResult = wlanHelperLib.WlanGetAvailableNetworkList(
        hClient, &pIfInfo->InterfaceGuid, 0, NULL, &pBssList);

    if (dwResult == ERROR_SUCCESS && pBssList) {
      for (DWORD j = 0; !retval && j < pBssList->dwNumberOfItems; j++) {
        PWLAN_AVAILABLE_NETWORK pBssEntry =
            (PWLAN_AVAILABLE_NETWORK)&pBssList->Network[j];

        if (!(pBssEntry->dwFlags & WLAN_AVAILABLE_NETWORK_CONNECTED)) continue;

        PWLAN_BSS_LIST pWlanBssList = NULL;
#if 1
        dwResult = wlanHelperLib.WlanGetNetworkBssList(
            hClient, &pIfInfo->InterfaceGuid, &pBssEntry->dot11Ssid,
            pBssEntry->dot11BssType, pBssEntry->bSecurityEnabled, NULL,
            &pWlanBssList);

        if (dwResult == ERROR_SUCCESS && pWlanBssList &&
            pWlanBssList->dwNumberOfItems > 0) {
          for (DWORD k = 0; !retval && k < pWlanBssList->dwNumberOfItems; k++) {
            PWLAN_BSS_ENTRY pWlanBssEntry =
                (PWLAN_BSS_ENTRY)&pWlanBssList->wlanBssEntries[k];

            networkInfo.asu = pWlanBssEntry->uLinkQuality;
            networkInfo.rssi = pWlanBssEntry->lRssi;
            networkInfo.level = quality_to_level(pWlanBssEntry->uLinkQuality);
            if (pWlanBssEntry->ulChCenterFrequency >= 5000000)
              networkInfo.networkSubtype = network::NetworkSubType::WIFI_5G;
            else if (pWlanBssEntry->ulChCenterFrequency >= 2400000)
              networkInfo.networkSubtype = network::NetworkSubType::WIFI_2P4G;
            retval = true;
          }
        }
#else
        if (false) {
        }
#endif
        else if (ERROR_SUCCESS ==
                 wlanHelperLib.QueryRssi(hClient, &pIfInfo->InterfaceGuid,
                                         &networkInfo.rssi)) {
          networkInfo.asu = pBssEntry->wlanSignalQuality;
          networkInfo.level = quality_to_level(networkInfo.asu);
          retval = true;
        } else {
          networkInfo.asu = pBssEntry->wlanSignalQuality;
          networkInfo.rssi = quality_to_rssi(pBssEntry->wlanSignalQuality);
          networkInfo.level = quality_to_level(pBssEntry->wlanSignalQuality);
          retval = true;
        }

        if (networkInfo.bssid.empty()) {
          PWLAN_CONNECTION_ATTRIBUTES pWlanConnAttr = NULL;
          if (ERROR_SUCCESS ==
              wlanHelperLib.QueryConnectionAttributes(
                  hClient, &pIfInfo->InterfaceGuid, &pWlanConnAttr)) {
            wlanHelperLib.WlanFreeMemory(pWlanConnAttr);
          }
        }

        if (pWlanBssList) wlanHelperLib.WlanFreeMemory(pWlanBssList);
      }
    }
    if (pBssList) wlanHelperLib.WlanFreeMemory(pBssList);
  }

Done:

  if (pIfList) wlanHelperLib.WlanFreeMemory(pIfList);

  if (hClient) wlanHelperLib.WlanCloseHandle(hClient, NULL);

  return retval;
}

}  // namespace detail

namespace network {

std::vector<std::string> local_addresses(NetworkType networkType) {
  std::vector<std::string> ips;
  if (!detail::ipHelperLib.initialize()) return ips;

  PIP_ADAPTER_INFO pAdapterInfo;
  PIP_ADAPTER_INFO pAdapter = NULL;
  DWORD dwRetVal = 0;

  ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
  pAdapterInfo = (IP_ADAPTER_INFO *)malloc(sizeof(IP_ADAPTER_INFO));
  if (pAdapterInfo == NULL) {
    return ips;
  }
  // Make an initial call to GetAdaptersInfo to get
  // the necessary size into the ulOutBufLen variable
  if (detail::ipHelperLib.GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) ==
      ERROR_BUFFER_OVERFLOW) {
    free(pAdapterInfo);
    pAdapterInfo = (IP_ADAPTER_INFO *)malloc(ulOutBufLen);
    if (pAdapterInfo == NULL) {
      return ips;
    }
  }

  if ((dwRetVal = detail::ipHelperLib.GetAdaptersInfo(
           pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
    pAdapter = pAdapterInfo;
    while (pAdapter) {
      PIP_ADDR_STRING pGatewayAddress = &pAdapter->GatewayList;
      if (ip::is_valid(ip::from_string(pGatewayAddress->IpAddress.String))) {
        PIP_ADDR_STRING pIpAddress = &pAdapter->IpAddressList;
        do {
          if (ip::is_valid(ip::from_string(pIpAddress->IpAddress.String))) {
            ips.push_back(pIpAddress->IpAddress.String);
          }
          pIpAddress = pIpAddress->Next;
        } while (pIpAddress != NULL);
      }

      pAdapter = pAdapter->Next;
    }
  } else {
    log(LOG_WARN, "GetAdaptersInfo failed with error: %d\n", dwRetVal);
  }

  if (pAdapterInfo) free(pAdapterInfo);

  return ips;
}

int get_network_type() {
  detail::AdapterIpAddressInfo adapterInfo;
  if (GetAdapterAddressInfo(adapterInfo)) return (int)adapterInfo.networkType;
  return (int)NetworkType::DISCONNECTED;
}

bool get_network_info(network_info_t &net_info) {
  net_info.networkSubtype = NetworkSubType::UNKNOWN;
  net_info.level = SignalLevel::UNKOWN;
  net_info.asu = -1;
  net_info.rssi = -1;
  net_info.localIp4.clear();
  net_info.gatewayIp4.clear();
  net_info.localIp6.clear();
  net_info.gatewayIp6.clear();
  net_info.localIp6List.clear();
  net_info.dnsList.clear();
  if (!net_info.ssid.empty()) net_info.ssid.clear();
  if (!net_info.bssid.empty()) net_info.bssid.clear();

  detail::AdapterIpAddressInfo adapterInfo;
  if (GetAdapterAddressInfo(adapterInfo)) {
    net_info.localIp4 = std::move(adapterInfo.localIp4);
    net_info.gatewayIp4 = std::move(adapterInfo.gatewayIp4);
    net_info.localIp6List = std::move(adapterInfo.localIp6List);
    net_info.dnsList = detail::GetDnsList();
    net_info.localIp6 = ipv6::ip_from_candidates(net_info.localIp6List);
    net_info.gatewayIp6 = std::move(adapterInfo.gatewayIp6);

    net_info.networkType = adapterInfo.networkType;
    if (net_info.networkType == NetworkType::WIFI) {
      if (!detail::GetWlanNetowrkInfo(net_info))
        net_info.networkType = NetworkType::DISCONNECTED;
    }
  }

  return true;
}

class NetworkMonitorWin32Impl {
  using sink_type = NetworkMonitorWin32::sink_type;

 public:
  NetworkMonitorWin32Impl(sink_type &&sink)
      : sink_(std::move(sink)), hShutdownEvent_(NULL) {
    if (!detail::ipHelperLib.initialize()) return;
    hShutdownEvent_ = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    thread_ = agora::commons::make_unique<std::thread>(
        std::bind(&NetworkMonitorWin32Impl::run, this));
  }
  ~NetworkMonitorWin32Impl() {
    if (hShutdownEvent_) ::SetEvent(hShutdownEvent_);
    if (thread_ && thread_->joinable()) thread_->join();
    if (hShutdownEvent_) ::CloseHandle(hShutdownEvent_);
  }

 private:
  void run() {
    DWORD ret;
    HANDLE hand;
    OVERLAPPED overlap;
    overlap.hEvent = ::WSACreateEvent();
    bool alive = true;
    HANDLE hEvents[] = {hShutdownEvent_, overlap.hEvent};
    DWORD timeout = INFINITE;
    while (alive) {
      ret = detail::ipHelperLib.NotifyAddrChange(&hand, &overlap);

      if (ret != NO_ERROR && WSAGetLastError() != WSA_IO_PENDING) break;

      ret = ::WaitForMultipleObjects(2, hEvents, FALSE, timeout);
      switch (ret) {
        case WAIT_OBJECT_0 + 0:
          alive = false;
          break;
        case WAIT_OBJECT_0 + 1:
          if (GetOverlappedResult(hand, &overlap, &ret, TRUE))
            udpateNetworkInfo(timeout);
          break;
        default:
          udpateNetworkInfo(timeout);
          break;
      }
    }
    detail::ipHelperLib.CancelIPChangeNotify(&overlap);
    ::WSACloseEvent(overlap.hEvent);
  }

 private:
  void udpateNetworkInfo(DWORD &timeout) {
    network_info_t ni;
    if (sink_ && get_network_info(ni)) {
      if (ni.networkType == NetworkType::DISCONNECTED ||
          ni.networkType == NetworkType::UNKNOWN || ni.ssid.empty() ||
          ni.bssid.empty())
        timeout = 2000;
      else
        timeout = INFINITE;
      sink_(ni);
    }
  }

 private:
  sink_type sink_;
  std::unique_ptr<std::thread> thread_;
  HANDLE hShutdownEvent_;
};

NetworkMonitorWin32::NetworkMonitorWin32(sink_type &&sink)
    : m_impl(new NetworkMonitorWin32Impl(std::move(sink))) {}

NetworkMonitorWin32::~NetworkMonitorWin32() { delete m_impl; }

namespace iphelper {
bool initialize() { return detail::ipHelperLib.initialize(); }
BOOL IcmpCloseHandle(HANDLE IcmpHandle) {
  return detail::ipHelperLib.IcmpCloseHandle(IcmpHandle);
}
HANDLE IcmpCreateFile() { return detail::ipHelperLib.IcmpCreateFile(); }
DWORD IcmpSendEcho2(HANDLE IcmpHandle, HANDLE Event, PVOID ApcRoutine,
                    PVOID ApcContext, IPAddr DestinationAddress,
                    LPVOID RequestData, WORD RequestSize,
                    PIP_OPTION_INFORMATION RequestOptions, LPVOID ReplyBuffer,
                    DWORD ReplySize, DWORD Timeout) {
  return detail::ipHelperLib.IcmpSendEcho2(
      IcmpHandle, Event, ApcRoutine, ApcContext, DestinationAddress,
      RequestData, RequestSize, RequestOptions, ReplyBuffer, ReplySize,
      Timeout);
}
DWORD IcmpParseReplies(LPVOID ReplyBuffer, DWORD ReplySize) {
  return detail::ipHelperLib.IcmpParseReplies(ReplyBuffer, ReplySize);
}
HANDLE Icmp6CreateFile() { return detail::ipHelperLib.Icmp6CreateFile(); }
DWORD Icmp6SendEcho2(HANDLE IcmpHandle, HANDLE Event, PVOID ApcRoutine,
                     PVOID ApcContext, sockaddr_in6 *SourceAddress,
                     sockaddr_in6 *DestinationAddress, LPVOID RequestData,
                     WORD RequestSize, PIP_OPTION_INFORMATION RequestOptions,
                     LPVOID ReplyBuffer, DWORD ReplySize, DWORD Timeout) {
  return detail::ipHelperLib.Icmp6SendEcho2(
      IcmpHandle, Event, ApcRoutine, ApcContext, SourceAddress,
      DestinationAddress, RequestData, RequestSize, RequestOptions, ReplyBuffer,
      ReplySize, Timeout);
}
DWORD Icmp6ParseReplies(LPVOID ReplyBuffer, DWORD ReplySize) {
  return detail::ipHelperLib.Icmp6ParseReplies(ReplyBuffer, ReplySize);
}
}

}  // namespace network
}
}
