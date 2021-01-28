//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#include "facilities/transport/tls_manager.h"

#include <cstdint>
#include <functional>
#include <memory>

#ifdef RTC_BUILD_SSL
#include "facilities/transport/tls_handler.h"
#endif

#include "utils/log/log.h"

#if defined(_WIN32)
#include <wincrypt.h>
#elif defined(__APPLE__)
#include <Security/Security.h>
#endif

namespace {
using agora::commons::LOG_WARN;

static const char GoDaddyRootG2[] =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIEADCCAuigAwIBAgIBADANBgkqhkiG9w0BAQUFADBjMQswCQYDVQQGEwJVUzEh\n"
    "MB8GA1UEChMYVGhlIEdvIERhZGR5IEdyb3VwLCBJbmMuMTEwLwYDVQQLEyhHbyBE\n"
    "YWRkeSBDbGFzcyAyIENlcnRpZmljYXRpb24gQXV0aG9yaXR5MB4XDTA0MDYyOTE3\n"
    "MDYyMFoXDTM0MDYyOTE3MDYyMFowYzELMAkGA1UEBhMCVVMxITAfBgNVBAoTGFRo\n"
    "ZSBHbyBEYWRkeSBHcm91cCwgSW5jLjExMC8GA1UECxMoR28gRGFkZHkgQ2xhc3Mg\n"
    "MiBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0eTCCASAwDQYJKoZIhvcNAQEBBQADggEN\n"
    "ADCCAQgCggEBAN6d1+pXGEmhW+vXX0iG6r7d/+TvZxz0ZWizV3GgXne77ZtJ6XCA\n"
    "PVYYYwhv2vLM0D9/AlQiVBDYsoHUwHU9S3/Hd8M+eKsaA7Ugay9qK7HFiH7Eux6w\n"
    "wdhFJ2+qN1j3hybX2C32qRe3H3I2TqYXP2WYktsqbl2i/ojgC95/5Y0V4evLOtXi\n"
    "EqITLdiOr18SPaAIBQi2XKVlOARFmR6jYGB0xUGlcmIbYsUfb18aQr4CUWWoriMY\n"
    "avx4A6lNf4DD+qta/KFApMoZFv6yyO9ecw3ud72a9nmYvLEHZ6IVDd2gWMZEewo+\n"
    "YihfukEHU1jPEX44dMX4/7VpkI+EdOqXG68CAQOjgcAwgb0wHQYDVR0OBBYEFNLE\n"
    "sNKR1EwRcbNhyz2h/t2oatTjMIGNBgNVHSMEgYUwgYKAFNLEsNKR1EwRcbNhyz2h\n"
    "/t2oatTjoWekZTBjMQswCQYDVQQGEwJVUzEhMB8GA1UEChMYVGhlIEdvIERhZGR5\n"
    "IEdyb3VwLCBJbmMuMTEwLwYDVQQLEyhHbyBEYWRkeSBDbGFzcyAyIENlcnRpZmlj\n"
    "YXRpb24gQXV0aG9yaXR5ggEAMAwGA1UdEwQFMAMBAf8wDQYJKoZIhvcNAQEFBQAD\n"
    "ggEBADJL87LKPpH8EsahB4yOd6AzBhRckB4Y9wimPQoZ+YeAEW5p5JYXMP80kWNy\n"
    "OO7MHAGjHZQopDH2esRU1/blMVgDoszOYtuURXO1v0XJJLXVggKtI3lpjbi2Tc7P\n"
    "TMozI+gciKqdi0FuFskg5YmezTvacPd+mSYgFFQlq25zheabIZ0KbIIOqPjCDPoQ\n"
    "HmyW74cNxA9hi63ugyuV+I6ShHI56yDqg+2DzZduCLzrTia2cyvk0/ZM/iZx4mER\n"
    "dEr/VxqHD3VILs9RaRegAhJhldXRQLIQTO7ErBBDpqWeCtWVYpoNz4iCxTIM5Cuf\n"
    "ReYNnyicsbkqWletNw+vHX/bvZ8=\n"
    "-----END CERTIFICATE-----\n";
static const char DigiCertGlobalRootCA[] =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\n"
    "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n"
    "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n"
    "QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\n"
    "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n"
    "b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\n"
    "9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\n"
    "CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\n"
    "nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\n"
    "43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\n"
    "T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\n"
    "gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\n"
    "BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\n"
    "TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\n"
    "DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\n"
    "hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\n"
    "06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\n"
    "PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\n"
    "YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\n"
    "CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\n"
    "-----END CERTIFICATE-----\n";

static bool ImportCaFromBuffer(X509_STORE* verify_store, const char* ca_buffer,
                               std::size_t length) {
#ifdef RTC_BUILD_SSL
  if (!verify_store || !ca_buffer || !length) {
    return false;
  }
  auto bio = BIO_new_mem_buf(static_cast<const void*>(ca_buffer), length);
  auto cert = PEM_read_bio_X509(bio, nullptr, nullptr, nullptr);
  X509_STORE_add_cert(verify_store, cert);
  X509_free(cert);
  BIO_free(bio);
  return true;
#else
  return false;
#endif
}

#if 0 && defined(_WIN32)

static bool ImportCertStoreToX509_STORE(
    LPWSTR store_name, DWORD store_location, X509_STORE *verify_store) {
  struct CertificateItem {
    using X509FreeFunc = std::function<void(X509*)>;
    X509 *cert;
    X509FreeFunc func;
    explicit CertificateItem(X509FreeFunc f)
        : cert(nullptr), func(f) {}
    ~CertificateItem() {
      if (cert) {
        func(cert);
      }
    }
  };
  struct StoreHolder {
    HCERTSTORE store = nullptr;
    ~StoreHolder() {
      if (store) {
        CertCloseStore(store, 0);
      }
    }
  };
  StoreHolder holder;
  holder.store = CertOpenStore(
      CERT_STORE_PROV_SYSTEM_W, 0, (HCRYPTPROV)nullptr,
      store_location | CERT_STORE_READONLY_FLAG, store_name);
  if (!holder.store) {
    return false;
  }
  PCCERT_CONTEXT cert_context = nullptr;
  while ((cert_context = CertEnumCertificatesInStore(
             holder.store, cert_context))) {
    CertificateItem item(X509_free);
    auto cert_bytes = static_cast<const uint8_t*>(cert_context->pbCertEncoded);
    item.cert = d2i_X509(
        nullptr, &cert_bytes, cert_context->cbCertEncoded);
    if (!item.cert) {
      return false;
    }
    if (X509_STORE_add_cert(verify_store, item.cert) != 1) {
      return false;
    }
  }
  return true;
}

#endif

#if 0 && defined(__APPLE__) && !TARGET_OS_IOS
static bool ImportKeychainToX509Store(
    X509_STORE *verify_store) {
  struct CertificateItem {
    using X509FreeFunc = std::function<void(X509*)>;
    CFDataRef data;
    X509 *cert;
    X509FreeFunc func;
    explicit CertificateItem(X509FreeFunc f)
        : data(nullptr), cert(nullptr), func(std::move(f)) {}
    ~CertificateItem() {
      if (data) {
        CFRelease(data);
      }
      if (cert) {
        func(cert);
      }
    }
  };
  struct CertificateArray {
    CFArrayRef array = nullptr;
    ~CertificateArray() {
      if (array) {
        CFRelease(array);
      }
    }
  };
  CertificateArray certificates;
  auto status = SecTrustCopyAnchorCertificates(&certificates.array);
  if (status != 0) {
    auto status_string = SecCopyErrorMessageString(status, nullptr);
    log(LOG_WARN, "Error enumerating certificates: %s",
        CFStringGetCStringPtr(status_string, kCFStringEncodingASCII));
    CFRelease(status_string);
    return false;
  }
  for (CFIndex i = 0; i < CFArrayGetCount(certificates.array); ++i) {
    CertificateItem item(X509_free);
    auto cert = (SecCertificateRef)CFArrayGetValueAtIndex(
        certificates.array, i);
    item.data = SecCertificateCopyData(cert);
    if (!item.data) {
      return false;
    }
    auto data_ptr = CFDataGetBytePtr(item.data);
    item.cert = d2i_X509(
        nullptr, &data_ptr, CFDataGetLength(item.data));
    if (!item.cert) {
      return false;
    }
    if (X509_STORE_add_cert(verify_store, item.cert) != 1) {
      return false;
    }
  }
  return true;
}
#endif

}  // namespace

namespace agora {
namespace transport {

TlsManager::TlsManager() : ssl_context_(nullptr) {}

TlsManager::~TlsManager() {
#ifdef RTC_BUILD_SSL
  SSL_CTX_free(ssl_context_);
#endif
}

void TlsManager::Initialize() {
#ifdef RTC_BUILD_SSL
  if (Initialized()) {
    return;
  }
  ssl_context_ = SSL_CTX_new(TLSv1_2_method());
  if (!ssl_context_) {
    return;
  }
  LoadCACertsFromSystem(ssl_context_);
  SSL_CTX_set_verify(ssl_context_, SSL_VERIFY_PEER, nullptr);
  SSL_CTX_set_cipher_list(ssl_context_, "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
#endif
}

ITlsHandler* TlsManager::CreateHandler(ITlsHandlerObserver* observer) {
#ifdef RTC_BUILD_SSL
  if (!Initialized()) {
    return nullptr;
  }
  if (!observer) {
    return nullptr;
  }
  auto handler = new TlsHandler(observer, ssl_context_);
  return handler;
#else
  return nullptr;
#endif
}

#ifdef RTC_BUILD_SSL
bool TlsManager::LoadCACertsFromSystem(SSL_CTX* context) {
  if (!context) {
    return false;
  }
#if 1
  // We should load specific root ca instead of using the system root cas.
  auto verify_store = SSL_CTX_get_cert_store(context);
  if (!verify_store) {
    log(LOG_WARN,
        "No X509 store found for SSL context while loading"
        " system certificates");
    return false;
  }
  if (!ImportCaFromBuffer(verify_store, GoDaddyRootG2, sizeof(GoDaddyRootG2))) {
    return false;
  }
  return ImportCaFromBuffer(verify_store, DigiCertGlobalRootCA, sizeof(DigiCertGlobalRootCA));
#else
#if !defined(_WIN32) && !defined(__APPLE__)
  if (SSL_CTX_set_default_verify_paths(context) != 1) {
    log(LOG_WARN, "Load default ca path failed");
    return false;
  }
  return true;
#else
  auto verify_store = SSL_CTX_get_cert_store(context);
  if (!verify_store) {
    log(LOG_WARN,
        "No X509 store found for SSL context while loading"
        " system certificates");
    return false;
  }
#if defined(_WIN32)
  if (!ImportCertStoreToX509_STORE(L"root", CERT_SYSTEM_STORE_CURRENT_USER, verify_store)) {
    return false;
  }
  return ImportCertStoreToX509_STORE(L"CA", CERT_SYSTEM_STORE_CURRENT_USER, verify_store);
#elif defined(__APPLE__) && (TARGET_OS_MAC)
  return ImportKeychainToX509Store(verify_store);
#endif
#endif
#endif
  return false;
}
#endif

}  // namespace transport
}  // namespace agora
