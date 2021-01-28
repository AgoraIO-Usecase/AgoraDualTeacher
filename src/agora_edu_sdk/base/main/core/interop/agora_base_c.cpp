//
//  Agora C SDK
//
//  Created by Tommy Miao in 2020.5
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#include <stdlib.h>
#include <string.h>

#include "agora_receiver_c.h"
#include "agora_ref_ptr_holder.h"
#include "base/AgoraBase.h"
#include "base/agora_base.h"

AGORA_API_C_HDL agora_alloc(size_t size) {
  void* buf = malloc(size);
  memset(buf, 0, size);
  return buf;
}

AGORA_API_C_VOID agora_free(AGORA_HANDLE* buf) {
  if (!buf || !*buf) {
    return;
  }

  free(*buf);
  *buf = nullptr;
}
