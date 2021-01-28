/*
 *  Copyright 2004 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "rtc_base/ssladapter.h"
#ifndef DISABLE_WEBRTC_SSL
#include "rtc_base/openssladapter.h"
#endif
///////////////////////////////////////////////////////////////////////////////

namespace rtc {

SSLAdapterFactory* SSLAdapterFactory::Create() {
#ifndef DISABLE_WEBRTC_SSL
  return new OpenSSLAdapterFactory();
#else
  RTC_LOG(LS_ERROR) << "rtc_build_ssl is closed.";
  return nullptr;
#endif
}

SSLAdapter* SSLAdapter::Create(AsyncSocket* socket) {
#ifndef DISABLE_WEBRTC_SSL
  return new OpenSSLAdapter(socket);
#else
  RTC_LOG(LS_ERROR) << "rtc_build_ssl is closed.";
  return nullptr;
#endif
}

///////////////////////////////////////////////////////////////////////////////

bool InitializeSSL() {
#ifndef DISABLE_WEBRTC_SSL
  return OpenSSLAdapter::InitializeSSL();
#else
  RTC_LOG(LS_ERROR) << "rtc_build_ssl is closed.";
  return true;
#endif
}

bool CleanupSSL() {
#ifndef DISABLE_WEBRTC_SSL
  return OpenSSLAdapter::CleanupSSL();
#else
  RTC_LOG(LS_ERROR) << "rtc_build_ssl is closed.";
  return true;
#endif
}

///////////////////////////////////////////////////////////////////////////////

}  // namespace rtc
