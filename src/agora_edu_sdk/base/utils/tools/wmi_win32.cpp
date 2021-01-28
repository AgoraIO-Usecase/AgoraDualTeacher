#include <comdef.h>
#include <wbemidl.h>
#include <string>
#include <thread>
#include <functional>

namespace agora {
namespace commons {
static std::string ConvertWCSToMBS(const wchar_t* pstr, long wslen) {
  int len = ::WideCharToMultiByte(CP_ACP, 0, pstr, wslen, NULL, 0, NULL, NULL);

  std::string dblstr(len, '\0');
  len = ::WideCharToMultiByte(CP_ACP, 0 /* no flags */, pstr,
                              wslen /* not necessary NULL-terminated */, &dblstr[0], len, NULL,
                              NULL /* no default char */);

  return dblstr;
}

static std::string ConvertBSTRToMBS(BSTR bstr) {
  int wslen = ::SysStringLen(bstr);
  return ConvertWCSToMBS((wchar_t*)bstr, wslen);
}

static std::string getPropertyValue(IWbemClassObject* pclsObj, const wchar_t* prop) {
  std::string value;
  VARIANT vtProp;
  HRESULT hr = pclsObj->Get(prop, 0, &vtProp, 0, 0);
  if (SUCCEEDED(hr)) {
    value = ConvertBSTRToMBS(vtProp.bstrVal);
    VariantClear(&vtProp);
  }
  return value;
}

#define COM_INITIALIZATION 1
std::string do_device_id_win32(bool infoOnly) {
  std::string deviceId;
  HRESULT hres;

  // Step 1: --------------------------------------------------
  // Initialize COM. ------------------------------------------
#if COM_INITIALIZATION
  hres = CoInitializeEx(0, COINIT_MULTITHREADED);
  if (FAILED(hres)) {
    //		cout << "Failed to initialize COM library. Error code = 0x" << hex << hres << endl;
    return deviceId;  // Program has failed.
  }
  // Step 2: --------------------------------------------------
  // Set general COM security levels --------------------------
#if 0
	hres = CoInitializeSecurity(
		NULL,
		-1,                          // COM authentication
		NULL,                        // Authentication services
		NULL,                        // Reserved
		RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
		RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
		NULL,                        // Authentication info
		EOAC_NONE,                   // Additional capabilities 
		NULL                         // Reserved
		);


	if (FAILED(hres))
	{
		//		cout << "Failed to initialize security. Error code = 0x" << hex << hres << endl;
		CoUninitialize();
		return deviceId;                    // Program has failed.
	}
#endif
#endif

  // Step 3: ---------------------------------------------------
  // Obtain the initial locator to WMI -------------------------

  IWbemLocator* pLoc = NULL;

  hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator,
                          (LPVOID*)&pLoc);

  if (FAILED(hres)) {
    //		cout << "Failed to create IWbemLocator object." << " Err code = 0x" << hex << hres
    //<< endl;
#if COM_INITIALIZATION
    CoUninitialize();
#endif
    return deviceId;  // Program has failed.
  }

  // Step 4: -----------------------------------------------------
  // Connect to WMI through the IWbemLocator::ConnectServer method

  IWbemServices* pSvc = NULL;

  // Connect to the root\cimv2 namespace with
  // the current user and obtain pointer pSvc
  // to make IWbemServices calls.
  hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"),  // Object path of WMI namespace
                             NULL,                     // User name. NULL = current user
                             NULL,                     // User password. NULL = current
                             0,                        // Locale. NULL indicates current
                             NULL,                     // Security flags.
                             0,                        // Authority (for example, Kerberos)
                             0,                        // Context object
                             &pSvc                     // pointer to IWbemServices proxy
  );

  if (FAILED(hres)) {
    //		cout << "Could not connect. Error code = 0x" << hex << hres << endl;
    pLoc->Release();
#if COM_INITIALIZATION
    CoUninitialize();
#endif
    return deviceId;  // Program has failed.
  }

  //	cout << "Connected to ROOT\\CIMV2 WMI namespace" << endl;

  // Step 5: --------------------------------------------------
  // Set security levels on the proxy -------------------------

  hres = CoSetProxyBlanket(pSvc,                         // Indicates the proxy to set
                           RPC_C_AUTHN_WINNT,            // RPC_C_AUTHN_xxx
                           RPC_C_AUTHZ_NONE,             // RPC_C_AUTHZ_xxx
                           NULL,                         // Server principal name
                           RPC_C_AUTHN_LEVEL_CALL,       // RPC_C_AUTHN_LEVEL_xxx
                           RPC_C_IMP_LEVEL_IMPERSONATE,  // RPC_C_IMP_LEVEL_xxx
                           NULL,                         // client identity
                           EOAC_NONE                     // proxy capabilities
  );

  if (FAILED(hres)) {
    //		cout << "Could not set proxy blanket. Error code = 0x" << hex << hres << endl;
    pSvc->Release();
    pLoc->Release();
#if COM_INITIALIZATION
    CoUninitialize();
#endif
    return deviceId;  // Program has failed.
  }

  // Step 6: --------------------------------------------------
  // Use the IWbemServices pointer to make requests of WMI ----

  // For example, get the name of the operating system
  IEnumWbemClassObject* pEnumerator = NULL;
  hres = pSvc->ExecQuery(bstr_t("WQL"),
                         bstr_t("SELECT Manufacturer, Model, SystemType FROM Win32_ComputerSystem"),
                         WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);

  if (FAILED(hres)) {
    //		cout << "Query for operating system name failed." << " Error code = 0x" << hex << hres
    //<< endl;
    pSvc->Release();
    pLoc->Release();
#if COM_INITIALIZATION
    CoUninitialize();
#endif
    return deviceId;  // Program has failed.
  }

  // Step 7: -------------------------------------------------
  // Get the data from the query in step 6 -------------------

  IWbemClassObject* pclsObj = NULL;
  ULONG uReturn = 0;

  if (pEnumerator) {
    HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

    if (SUCCEEDED(hr) && 0 != uReturn) {
      deviceId = getPropertyValue(pclsObj, L"Manufacturer");
      deviceId += '/';
      deviceId += getPropertyValue(pclsObj, L"Model");
      if (!infoOnly) {
        deviceId += '/';
        deviceId += getPropertyValue(pclsObj, L"SystemType");
      }

      pclsObj->Release();
    }
  }

  // Cleanup
  // ========

  pSvc->Release();
  pLoc->Release();
  pEnumerator->Release();
#if COM_INITIALIZATION
  CoUninitialize();
#endif

  return deviceId;
}

namespace {
static std::string dev_id("");
}

static void device_id_win32_thread(std::string*, bool infoOnly) {
  dev_id = do_device_id_win32(infoOnly);
}

std::string device_id_win32(bool infoOnly) {
  if (dev_id == "") {
    std::thread thrd(std::bind(&device_id_win32_thread, nullptr, infoOnly));
    thrd.join();
  }
  return dev_id;
}
}  // namespace commons
}  // namespace agora
