//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once
#include <jni.h>
#include <cassert>
#include <string>

namespace agora { namespace commons { namespace android {

class String
{
	JNIEnv* m_env;
	jstring m_jstr;
	const char* m_cstr;
public:
	String(JNIEnv* env, jstring jstr)
		:m_env(env)
		, m_jstr(jstr)
		, m_cstr(nullptr)
	{
		if (env&&jstr)
		{
			jboolean isCopy;
			m_cstr = env->GetStringUTFChars(jstr, &isCopy);
		}
	}
	~String()
	{
		// release the Java string and UTF-8
		if (m_env && m_jstr)
			m_env->ReleaseStringUTFChars(m_jstr, m_cstr);
	}
	const char* c_str() { return m_cstr; }
	operator bool() const { return m_cstr != nullptr; }
};

	inline std::string to_string(JNIEnv* env, jbyteArray &jarray)
	{
        std::string result;
        if (jarray) {
            jsize alen = env->GetArrayLength(jarray);
            if (alen > 0) {
                jbyte* ba = env->GetByteArrayElements(jarray, JNI_FALSE);
                result.assign((const char*)ba, (size_t)alen);
                env->ReleaseByteArrayElements(jarray, ba, 0);
            }
        }
        return std::move(result);
	}
	inline size_t copy_buffer(JNIEnv* env, jbyteArray &jarray, void* buffer, size_t length)
	{
		jsize alen = env->GetArrayLength(jarray);
		if (alen > 0) {
			jbyte* ba = env->GetByteArrayElements(jarray, JNI_FALSE);
			size_t cblen = alen < length ? alen : length;
			memcpy(buffer, (const char*)ba, cblen);
			env->ReleaseByteArrayElements(jarray, ba, 0);
			return cblen;
		}
		return 0;
	}

class AttachThreadScoped
{
public:
	explicit AttachThreadScoped(JavaVM* jvm)
		: attached_(false), jvm_(jvm), env_(nullptr) {
		jint ret_val = jvm->GetEnv(reinterpret_cast<void**>(&env_),
			JNI_VERSION_1_4);
		if (ret_val == JNI_EDETACHED) {
			// Attach the thread to the Java VM.
			ret_val = jvm_->AttachCurrentThread(&env_, nullptr);
			attached_ = ret_val >= 0;
			assert(attached_);
		}
	}
	~AttachThreadScoped() {
		if (attached_ && (jvm_->DetachCurrentThread() < 0)) {
			assert(false);
		}
	}

	JNIEnv* env() { return env_; }

private:
	bool attached_;
	JavaVM* jvm_;
	JNIEnv* env_;
};

inline bool isSameObject(JNIEnv* env, jobject o1, jobject o2) {
	if (o1 == o2)
		return true;
	else if (!o1 || !o2)
		return false;
	else
		return env->IsSameObject(o1, o2);
}

}}}
