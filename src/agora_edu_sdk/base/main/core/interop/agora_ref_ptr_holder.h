//
//  Agora C SDK
//
//  Created by Ender Zheng in 2020.5
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include "base/AgoraRefPtr.h"
#include "base/agora_api.h"

namespace agora {
namespace interop {

template <typename T>
class RefPtrHolder {
 public:
  explicit RefPtrHolder(agora_refptr<T> ref_ptr) : ref_ptr_(ref_ptr) {}

  ~RefPtrHolder() { ref_ptr_ = nullptr; }

  RefPtrHolder(const RefPtrHolder& rhs) { ref_ptr_ = rhs.ref_ptr_; }

  RefPtrHolder(RefPtrHolder&& rhs) {
    ref_ptr_ = rhs.ref_ptr_;
    rhs.ref_ptr_ = nullptr;
  }

  RefPtrHolder& operator=(const RefPtrHolder& rhs) {
    if (this == &rhs) {
      return *this;
    }

    ref_ptr_ = rhs.ref_ptr_;
    return *this;
  }

  RefPtrHolder& operator=(RefPtrHolder&& rhs) {
    if (this == &rhs) {
      return *this;
    }

    ref_ptr_ = rhs.ref_ptr_;
    rhs.ref_ptr_ = nullptr;
    return *this;
  }

  agora_refptr<T>& Get() { return ref_ptr_; }

 private:
  agora_refptr<T> ref_ptr_;
};

}  // namespace interop
}  // namespace agora

/**
 * RefPtrHolder
 */
#define REF_PTR_HOLDER(type) agora::interop::RefPtrHolder<type>
#define REF_PTR_HOLDER_NEW(type, ref_ptr) \
  (ref_ptr ? new agora::interop::RefPtrHolder<type>(ref_ptr) : nullptr)

/**
 * Casts
 */
#define REINTER_CAST(casted_ptr, type, cast_ptr) auto casted_ptr = reinterpret_cast<type*>(cast_ptr)

#define REF_PTR_HOLDER_CAST(ref_ptr_holder, type, agora_handle) \
  REINTER_CAST(ref_ptr_holder, agora::interop::RefPtrHolder<type>, agora_handle)

#define AGORA_SERVICE_CAST(agora_service, agora_svc) \
  REINTER_CAST(agora_service, agora::base::IAgoraService, agora_svc)

#define LOCAL_USER_CAST(local_user_ptr, local_user) \
  REINTER_CAST(local_user_ptr, agora::rtc::ILocalUser, local_user)

/**
 * Create and Destroy Functions
 */
#define DEFINE_CREATE_NORMAL_HANDLE_FUNC(func, struct_t, cpp_struct_t) \
  AGORA_API_C_HDL func(struct_t* c_struct) { return new cpp_struct_t(c_struct); }

#define DEFINE_DESTROY_NORMAL_HANDLE_FUNC(func, type) \
  AGORA_API_C_VOID func(AGORA_HANDLE ptr_to_handle) { \
    if (!ptr_to_handle) {                             \
      return;                                         \
    }                                                 \
                                                      \
    REINTER_CAST(handle, type, ptr_to_handle);        \
    delete handle;                                    \
                                                      \
    ptr_to_handle = nullptr;                          \
  }

#define DEFINE_CREATE_AND_DESTROY_PAIR_FUNCS(struct_t, cpp_struct_t)                  \
  DEFINE_CREATE_NORMAL_HANDLE_FUNC(agora_##struct_t##_create, struct_t, cpp_struct_t) \
  DEFINE_DESTROY_NORMAL_HANDLE_FUNC(agora_##struct_t##_destroy, cpp_struct_t)

#define DEFINE_DESTROY_REF_PTR_HOLDER_HANDLE_FUNC(func, type) \
  DEFINE_DESTROY_NORMAL_HANDLE_FUNC(func, agora::interop::RefPtrHolder<type>)

/**
 * RefPtrHolder related functions
 */
#define DEFINE_REF_PTR_HOLDER_FUNC_VOID(c_func, handle_t, cpp_func) \
  AGORA_API_C_VOID c_func(AGORA_HANDLE handle) {                    \
    if (!handle) {                                                  \
      return;                                                       \
    }                                                               \
                                                                    \
    REF_PTR_HOLDER_CAST(handle_holder, handle_t, handle);           \
                                                                    \
    handle_holder->Get()->cpp_func();                               \
  }

#define DEFINE_REF_PTR_HOLDER_FUNC_INT(c_func, handle_t, cpp_func) \
  AGORA_API_C_INT c_func(AGORA_HANDLE handle) {                    \
    if (!handle) {                                                 \
      return -1;                                                   \
    }                                                              \
                                                                   \
    REF_PTR_HOLDER_CAST(handle_holder, handle_t, handle);          \
                                                                   \
    return handle_holder->Get()->cpp_func();                       \
  }

#define DEFINE_REF_PTR_HOLDER_FUNC_VOID_ARG_1(c_func, handle_t, cpp_func, arg_t_1) \
  AGORA_API_C_VOID c_func(AGORA_HANDLE handle, arg_t_1 arg_1) {                    \
    if (!handle) {                                                                 \
      return;                                                                      \
    }                                                                              \
                                                                                   \
    REF_PTR_HOLDER_CAST(handle_holder, handle_t, handle);                          \
                                                                                   \
    handle_holder->Get()->cpp_func(arg_1);                                         \
  }

#define DEFINE_REF_PTR_HOLDER_FUNC_INT_ARG_1(c_func, handle_t, cpp_func, arg_t_1) \
  AGORA_API_C_INT c_func(AGORA_HANDLE handle, arg_t_1 arg_1) {                    \
    if (!handle) {                                                                \
      return -1;                                                                  \
    }                                                                             \
                                                                                  \
    REF_PTR_HOLDER_CAST(handle_holder, handle_t, handle);                         \
                                                                                  \
    return handle_holder->Get()->cpp_func(arg_1);                                 \
  }

#define DEFINE_REF_PTR_HOLDER_FUNC_INT_ARG_1_CAST(c_func, handle_t, cpp_func, arg_t_1, casted_t_1) \
  AGORA_API_C_INT c_func(AGORA_HANDLE handle, arg_t_1 arg_1) {                                     \
    if (!handle) {                                                                                 \
      return -1;                                                                                   \
    }                                                                                              \
                                                                                                   \
    REF_PTR_HOLDER_CAST(handle_holder, handle_t, handle);                                          \
                                                                                                   \
    return handle_holder->Get()->cpp_func(static_cast<casted_t_1>(arg_1));                         \
  }

#define DEFINE_REF_PTR_HOLDER_FUNC_INT_ARG_2(c_func, handle_t, cpp_func, arg_t_1, arg_t_2) \
  AGORA_API_C_INT c_func(AGORA_HANDLE handle, arg_t_1 arg_1, arg_t_2 arg_2) {              \
    if (!handle) {                                                                         \
      return -1;                                                                           \
    }                                                                                      \
                                                                                           \
    REF_PTR_HOLDER_CAST(handle_holder, handle_t, handle);                                  \
                                                                                           \
    return handle_holder->Get()->cpp_func(arg_1, arg_2);                                   \
  }

#define DEFINE_REF_PTR_HOLDER_FUNC_CREATE(c_func, handle_t, ret_t, cpp_func) \
  AGORA_API_C_HDL c_func(AGORA_HANDLE handle) {                              \
    if (!handle) {                                                           \
      return nullptr;                                                        \
    }                                                                        \
                                                                             \
    REF_PTR_HOLDER_CAST(handle_holder, handle_t, handle);                    \
                                                                             \
    return REF_PTR_HOLDER_NEW(ret_t, handle_holder->Get()->cpp_func());      \
  }
